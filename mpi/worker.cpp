#include "worker.hpp"
#include <iostream>

#define TAG_TASK 1
#define TAG_RESULT 2

WorkerNode::WorkerNode(int r, const std::string& model, 
                       const std::string& url, const std::string& key)
    : rank(r), modelPath(model), supabaseUrl(url), apiKey(key) {}

void WorkerNode::run() {
    std::cout << "[WORKER " << rank << "] Cargando modelo ArcFace...\n";
    FaceProcessor processor(modelPath, supabaseUrl, apiKey);
    
    std::cout << "[WORKER " << rank << "] Listo para recibir tareas\n";
    
    while (true) {
        int taskSize;
        MPI_Status status;
        
        MPI_Recv(&taskSize, 1, MPI_INT, 0, TAG_TASK, MPI_COMM_WORLD, &status);
        
        std::vector<uint8_t> taskData(taskSize);
        MPI_Recv(taskData.data(), taskSize, MPI_BYTE, 0, TAG_TASK, MPI_COMM_WORLD, &status);
        
        FaceTask task = deserializeTask(taskData);
        
        std::cout << "[WORKER " << rank << "] Procesando task " << task.task_id << " faceId=" << task.face_id << "\n";
        
        FaceResult result;
        result.task_id = task.task_id;
        result.face_id = task.face_id;
        result.success = false;
        
        cv::Mat face = cv::imdecode(task.image_data, cv::IMREAD_COLOR);
        
        if (task.type == TaskType::REGISTER) {
            result = processor.processRegister(face, task.face_id, task.person_name);
        } else {
            result = processor.processRecognize(face, task.face_id);
        }
        
        std::cout << "[WORKER " << rank << "] Resultado: success=" << result.success 
                  << " name=" << result.person_name << " confidence=" << result.confidence << "\n";
        
        std::vector<uint8_t> serialized = serializeResult(result);
        int resultSize = serialized.size();
        
        std::cout << "[WORKER " << rank << "] Enviando resultado size=" << resultSize << " -> master\n";
        
        MPI_Send(&resultSize, 1, MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
        MPI_Send(serialized.data(), resultSize, MPI_BYTE, 0, TAG_RESULT, MPI_COMM_WORLD);
        
        std::cout << "[WORKER " << rank << "] Resultado enviado exitosamente\n";
    }
}
