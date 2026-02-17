#include <iostream>
#include <string>
#include "zmq/receiver.hpp"
#include "zmq/sender.hpp"
#include "arcface/arcface.hpp"
#include "supabase/env_loader.hpp"
#include "supabase/http_client.hpp"
#include "supabase/person_repo.hpp"
#include "supabase/embedding_repo.hpp"

int main() {
    std::cout << "===========================================\n";
    std::cout << "  Sistema de Reconocimiento Facial (C++)  \n";
    std::cout << "===========================================\n\n";

    // Cargar credenciales
    EnvLoader::load(".env");
    const std::string url     = EnvLoader::get("SUPABASE_URL");
    const std::string api_key = EnvLoader::get("SUPABASE_KEY");

    if (url.empty() || api_key.empty()) {
        std::cerr << "[INIT] ERROR: Faltan SUPABASE_URL o SUPABASE_KEY en .env\n";
        return 1;
    }

    std::cout << "[INIT] Cargando modelo ArcFace...\n";
    ArcFace arcface("public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx");

    std::cout << "[INIT] Conectando a Supabase...\n";
    HttpClient    http(url);
    PersonRepo    persons(http,    api_key);
    EmbeddingRepo embeddings(http, api_key);

    std::cout << "[INIT] Iniciando receptor ZMQ...\n";
    ZMQReceiver receiver("tcp://*:5555");
    ZMQSender sender("tcp://*:5556");

    std::cout << "\n[READY] Sistema listo. Esperando caras...\n";
    std::cout << "===========================================\n\n";

    cv::Mat     face;
    int         faceId;
    std::string mode, personName;

    while (true) {
        if (!receiver.receive(face, faceId, mode, personName)) {
            std::cerr << "[ERROR] Failed to receive face\n";
            continue;
        }

        std::cout << "\n[" << mode << "] FaceID=" << faceId
                  << " Name='" << personName << "'\n";

        auto embedding = arcface.getEmbedding(face);

        if (mode == "register") {
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

        } else if (mode == "recognize") {
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

        } else {
            std::cerr << "[ERROR] Modo desconocido: " << mode << "\n";
        }

        std::cout << "===========================================\n";
    }

    return 0;
}
