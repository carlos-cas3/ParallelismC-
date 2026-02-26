#include "master.hpp"
#include <iostream>
#include <chrono>
#include <csignal>

#define TAG_TASK 1
#define TAG_RESULT 2

// Timeout para polling (ms)
constexpr int ZMQ_POLL_TIMEOUT = 100;
constexpr int MPI_POLL_TIMEOUT = 10;
constexpr int IDLE_SLEEP_MS = 5;
constexpr int MAX_PENDING_TASKS = 100;

MasterNode::MasterNode(int ws) : worldSize(ws) {}

MasterNode::~MasterNode() {
    shutdown();
}

void MasterNode::shutdown() {
    state.running = false;
}

void signalHandler(int signum) {
    std::cout << "\n[MASTER] Señal recibida, cerrando...\n";
    exit(0);
}

void MasterNode::registerReceiverThread() {
    std::cout << "[REGISTER_THREAD] Iniciando en puerto 5555...\n";
    ZMQReceiver receiver("tcp://*:5555");
    
    while (state.running) {
        cv::Mat face;
        int faceId;
        std::string mode, personName;
        
        // receive() ahora tiene timeout interno, no bloquea infinitamente
        if (receiver.receive(face, faceId, mode, personName)) {
            FaceTask task;
            task.task_id = globalTaskCounter++;
            task.face_id = faceId;
            task.type = TaskType::REGISTER;
            task.person_name = personName;
            
            std::vector<uint8_t> imgData;
            cv::imencode(".jpg", face, imgData);
            task.image_data = imgData;
            
            {
                std::lock_guard<std::mutex> lock(state.mutex);
                if (state.task_queue.size() < MAX_PENDING_TASKS) {
                    state.task_queue.push(task);
                    std::cout << "[REGISTER_THREAD] Task " << task.task_id 
                              << " queued (faceId=" << faceId << ", queue size=" << state.task_queue.size() << ")\n";
                } else {
                    std::cerr << "[REGISTER_THREAD] Cola llena, descartando tarea " << task.task_id << "\n";
                }
            }
            state.cv.notify_one();
        }
        
        // OPTIMIZACION: Pequeño sleep para evitar busy-wait 100% CPU
        // cuando no hay mensajes
        std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_SLEEP_MS));
    }
    
    std::cout << "[REGISTER_THREAD] Terminando...\n";
}

void MasterNode::recognizeReceiverThread() {
    std::cout << "[RECOGNIZE_THREAD] Iniciando en puerto 5557...\n";
    ZMQReceiver receiver("tcp://*:5557");
    
    while (state.running) {
        cv::Mat face;
        int faceId;
        std::string mode, personName;
        
        if (receiver.receive(face, faceId, mode, personName)) {
            FaceTask task;
            task.task_id = globalTaskCounter++;
            task.face_id = faceId;
            task.type = TaskType::RECOGNIZE;
            task.person_name = "";
            
            std::vector<uint8_t> imgData;
            cv::imencode(".jpg", face, imgData);
            task.image_data = imgData;
            
            {
                std::lock_guard<std::mutex> lock(state.mutex);
                if (state.task_queue.size() < MAX_PENDING_TASKS) {
                    state.task_queue.push(task);
                    std::cout << "[RECOGNIZE_THREAD] Task " << task.task_id 
                              << " queued (faceId=" << faceId << ", queue size=" << state.task_queue.size() << ")\n";
                } else {
                    std::cerr << "[RECOGNIZE_THREAD] Cola llena, descartando tarea " << task.task_id << "\n";
                }
            }
            state.cv.notify_one();
        }
        
        // OPTIMIZACION: Pequeño sleep para evitar busy-wait
        std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_SLEEP_MS));
    }
    
    std::cout << "[RECOGNIZE_THREAD] Terminando...\n";
}

void MasterNode::run() {
    std::cout << "===========================================\n";
    std::cout << "  MASTER NODE - Face Recognition MPI      \n";
    std::cout << "  Workers disponibles: " << (worldSize - 1) << "\n";
    std::cout << "  Hilos receptores: 2 (REGISTER + RECOGNIZE)\n";
    std::cout << "  Timeouts: ZMQ=" << ZMQ_POLL_TIMEOUT << "ms, MPI=" << MPI_POLL_TIMEOUT << "ms\n";
    std::cout << "===========================================\n\n";
    
    ZMQSender registerSender("tcp://*:5556");
    ZMQSender recognizeSender("tcp://*:5558");
    
    std::unordered_map<int, TaskType> pendingTasks;
    std::queue<int> availableWorkers;
    
    for (int i = 1; i < worldSize; i++) {
        availableWorkers.push(i);
    }
    
    std::thread tRegister(&MasterNode::registerReceiverThread, this);
    std::thread tRecognize(&MasterNode::recognizeReceiverThread, this);
    
    tRegister.detach();
    tRecognize.detach();
    
    std::cout << "[MASTER] Dispatcher listo. Esperando tareas...\n";
    
    // Contador para debug de actividad
    int idleCount = 0;
    
    while (state.running) {
        FaceTask task;
        bool hasTask = false;
        
        {
            std::unique_lock<std::mutex> lock(state.mutex);
            state.cv.wait_for(lock, std::chrono::milliseconds(MPI_POLL_TIMEOUT), [this] {
                return !state.task_queue.empty();
            });
            
            if (!state.task_queue.empty()) {
                task = state.task_queue.front();
                state.task_queue.pop();
                hasTask = true;
            }
        }
        
        if (hasTask && !availableWorkers.empty()) {
            int workerRank = availableWorkers.front();
            availableWorkers.pop();
            
            std::vector<uint8_t> serialized = serializeTask(task);
            int size = serialized.size();
            
            MPI_Send(&size, 1, MPI_INT, workerRank, TAG_TASK, MPI_COMM_WORLD);
            MPI_Send(serialized.data(), size, MPI_BYTE, workerRank, TAG_TASK, MPI_COMM_WORLD);
            
            pendingTasks[task.face_id] = task.type;
            std::string typeStr = (task.type == TaskType::REGISTER) ? "REGISTER" : "RECOGNIZE";
            std::cout << "[MASTER] " << typeStr << " task face_id=" << task.face_id 
                      << " -> worker " << workerRank << "\n";
            
            idleCount = 0;
        }
        
        // OPTIMIZACION: MPI_Iprobe no bloqueante con flag
        int flag = 0;
        MPI_Status probe_status;
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &flag, &probe_status);
        
        if (flag) {
            int source = probe_status.MPI_SOURCE;
            std::cout << "[MASTER] Recibiendo resultado de worker " << source << "\n";
            
            try {
                int resultSize;
                MPI_Recv(&resultSize, 1, MPI_INT, source, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                if (resultSize <= 0 || resultSize > 100000) {
                    std::cerr << "[MASTER] Tamaño de resultado inválido: " << resultSize << "\n";
                    availableWorkers.push(source);
                    idleCount = 0;
                    continue;
                }
                
                std::vector<uint8_t> resultData(resultSize);
                MPI_Recv(resultData.data(), resultSize, MPI_BYTE, source, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                std::cout << "[MASTER] Resultado recibido: size=" << resultSize << "\n";
                
                FaceResult result = deserializeResult(resultData);
                std::cout << "[MASTER] Resultado deserializado: task_id=" << result.task_id 
                          << " face_id=" << result.face_id << " success=" << result.success << "\n";
                
                availableWorkers.push(source);
            
            auto it = pendingTasks.find(result.face_id);
            if (it != pendingTasks.end()) {
                TaskType type = it->second;
                pendingTasks.erase(it);
                
                std::string json;
                if (type == TaskType::REGISTER) {
                    if (result.success) {
                        json = ResponseBuilder::buildRegisterSuccess(result.face_id, result.person_id, result.person_name);
                    } else {
                        json = ResponseBuilder::buildRegisterError(result.face_id);
                    }
                    registerSender.send(json);
                } else {
                    if (result.success) {
                        json = ResponseBuilder::buildRecognizeSuccess(result.face_id, result.person_id, result.person_name, result.confidence);
                    } else {
                        json = ResponseBuilder::buildRecognizeUnknown(result.face_id);
                    }
                    bool sent = recognizeSender.send(json);
                    if (sent) {
                        std::cout << "[MASTER] Enviado a Python (puerto 5558): " << json << "\n";
                    } else {
                        std::cerr << "[MASTER] ERROR: No se pudo enviar respuesta\n";
                    }
                }
                std::cout << "[MASTER] Response: " << json << "\n";
            } else {
                std::cerr << "[MASTER] ERROR: task_id " << result.task_id << " no encontrado. IDs pendientes: [";
                for (const auto& p : pendingTasks) {
                    std::cerr << p.first << ",";
                }
                std::cerr << "]\n";
            }
            } catch (const std::exception& e) {
                std::cerr << "[MASTER] ERROR procesando resultado: " << e.what() << "\n";
                availableWorkers.push(source);
            }
            
            idleCount = 0;
        } else {
            // OPTIMIZACION: Solo hacer debug cada 100 iteraciones vacías
            idleCount++;
            if (idleCount % 100 == 0) {
                std::cout << "[MASTER] Esperando... (tasks pendientes: " 
                          << pendingTasks.size() << ", workers libres: " 
                          << availableWorkers.size() << ")\n";
            }
        }
        
        // OPTIMIZACION: Sleep pequeño cuando no hay actividad
        // Esto reduce el uso de CPU drásticamente
        std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_SLEEP_MS));
    }
    
    std::cout << "[MASTER] Terminando dispatcher...\n";
}
