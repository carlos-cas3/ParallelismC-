#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include "zmq/receiver.hpp"
#include "zmq/sender.hpp"
#include "arcface/arcface.hpp"
#include "supabase/env_loader.hpp"
#include "supabase/http_client.hpp"
#include "supabase/person_repo.hpp"
#include "supabase/embedding_repo.hpp"

std::string g_url;
std::string g_api_key;
std::atomic<bool> g_running(true);

void registerThread() {
    std::cout << "[REGISTER] Cargando modelo ArcFace...\n";
    ArcFace arcface("public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx");

    std::cout << "[REGISTER] Conectando a Supabase...\n";
    HttpClient http(g_url);
    PersonRepo persons(http, g_api_key);
    EmbeddingRepo embeddings(http, g_api_key);

    std::cout << "[REGISTER] Iniciando sockets ZMQ (5555 PULL, 5556 PUSH)...\n";
    ZMQReceiver receiver("tcp://*:5555");
    ZMQSender sender("tcp://*:5556");

    std::cout << "[REGISTER] Listo. Esperando caras...\n";

    cv::Mat face;
    int faceId;
    std::string mode, personName;

    while (g_running) {
        if (!receiver.receive(face, faceId, mode, personName)) {
            continue;
        }

        std::cout << "\n[REGISTER] FaceID=" << faceId
                  << " Name='" << personName << "'\n";

        auto embedding = arcface.getEmbedding(face);

        if (personName.empty()) {
            std::cerr << "[REGISTER] Nombre vacío, omitiendo...\n";
            continue;
        }

        std::string person_id = persons.getOrCreate(personName);
        if (person_id.empty()) {
            std::cerr << "[REGISTER] Error al obtener/crear persona\n";
            continue;
        }

        bool ok = embeddings.save(person_id, embedding);
        std::cout << (ok ? "[REGISTER] ✓ Guardado: " : "[REGISTER] ✗ Error: ")
                  << personName << "\n";

        if (ok) {
            std::string json = "{\"face_id\":" + std::to_string(faceId) +
                               ",\"person_id\":\"" + person_id + "\"" +
                               ",\"person_name\":\"" + personName + "\"" +
                               ",\"status\":\"registered\"}";
            sender.send(json);
            std::cout << "[REGISTER] Sent: " << json << "\n";
        }

        std::cout << "===========================================\n";
    }
}

void recognizeThread() {
    std::cout << "[RECOGNIZE] Cargando modelo ArcFace...\n";
    ArcFace arcface("public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx");

    std::cout << "[RECOGNIZE] Conectando a Supabase...\n";
    HttpClient http(g_url);
    EmbeddingRepo embeddings(http, g_api_key);

    std::cout << "[RECOGNIZE] Iniciando sockets ZMQ (5557 PULL, 5558 PUSH)...\n";
    ZMQReceiver receiver("tcp://*:5557");
    ZMQSender sender("tcp://*:5558");

    std::cout << "[RECOGNIZE] Listo. Esperando caras...\n";

    cv::Mat face;
    int faceId;
    std::string mode, personName;

    while (g_running) {
        if (!receiver.receive(face, faceId, mode, personName)) {
            continue;
        }

        std::cout << "\n[RECOGNIZE] FaceID=" << faceId << "\n";

        auto embedding = arcface.getEmbedding(face);
        MatchResult match = embeddings.findBestMatch(embedding, 0.60f);

        std::string json;
        if (match.score >= 0.60f) {
            json = "{\"face_id\":" + std::to_string(faceId) +
                   ",\"person_id\":\"" + match.person_id + "\"" +
                   ",\"person_name\":\"" + match.name + "\"" +
                   ",\"confidence\":" + std::to_string(match.score) + "}";
            std::cout << "[RECOGNIZE] ✓ Identificado: " << match.name
                      << " (score=" << match.score << ")\n";
        } else {
            json = "{\"face_id\":" + std::to_string(faceId) +
                   ",\"person_id\":\"\"" +
                   ",\"person_name\":\"Desconocido\"" +
                   ",\"confidence\":0.0}";
            std::cout << "[RECOGNIZE] ⚠ Persona no reconocida\n";
        }

        if (sender.send(json)) {
            std::cout << "[RECOGNIZE] Sent: " << json << "\n";
        } else {
            std::cerr << "[RECOGNIZE] Error enviando respuesta\n";
        }

        std::cout << "===========================================\n";
    }
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "  Sistema de Reconocimiento Facial (C++)  \n";
    std::cout << "  Dos hilos: Register | Recognize         \n";
    std::cout << "===========================================\n\n";

    EnvLoader::load(".env");
    g_url = EnvLoader::get("SUPABASE_URL");
    g_api_key = EnvLoader::get("SUPABASE_KEY");

    if (g_url.empty() || g_api_key.empty()) {
        std::cerr << "[INIT] ERROR: Faltan SUPABASE_URL o SUPABASE_KEY en .env\n";
        return 1;
    }

    std::cout << "[INIT] Iniciando hilos...\n\n";

    std::thread t_register(registerThread);
    std::thread t_recognize(recognizeThread);

    t_register.join();
    t_recognize.join();

    return 0;
}
