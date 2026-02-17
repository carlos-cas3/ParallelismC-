#include <iostream>
#include <string>
#include "zmq/receiver.hpp"
#include "arcface/arcface.hpp"
#include "supabase/client.hpp"

int main() {
    std::cout << "===========================================\n";
    std::cout << "  Sistema de Reconocimiento Facial (C++)  \n";
    std::cout << "===========================================\n\n";

    std::cout << "[INIT] Cargando modelo ArcFace...\n";
    ArcFace arcface("public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx");
    
    std::cout << "[INIT] Conectando a Supabase...\n";
    SupabaseClient supabase(
        "https://tlcxmvqbhyctreqzljpl.supabase.co",
        "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InRsY3htdnFiaHljdHJlcXpsanBsIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTc3MDkxNzkxMSwiZXhwIjoyMDg2NDkzOTExfQ.eD03EAfZQrynLHdydlRmvQBrO7MjDqxfgI9-_u_cC6Y"
    );
    
    std::cout << "[INIT] Iniciando receptor ZMQ...\n";
    ZMQReceiver receiver("tcp://*:5555");
    
    std::cout << "\n[READY] Sistema listo. Esperando caras...\n";
    std::cout << "===========================================\n\n";

    while (true) {
        cv::Mat face;
        int faceId;
        std::string mode;
        std::string personName;

        if (!receiver.receive(face, faceId, mode, personName)) {
            std::cerr << "[ERROR] Failed to receive face\n";
            continue;
        }

        std::cout << "\n[" << mode << "] Processing FaceID=" << faceId << ", Name=" << personName << "\n";

        std::vector<float> embedding = arcface.getEmbedding(face);

        if (mode == "register") {
            if (personName.empty()) {
                std::cout << "[REGISTER] Nombre vacío, omitiendo...\n";
                continue;
            }

            std::string personId = supabase.getOrCreatePerson(personName);
            if (personId.empty()) {
                std::cerr << "[REGISTER] Error al obtener/crear persona\n";
                continue;
            }

            bool success = supabase.saveEmbedding(personId, embedding);
            if (success) {
                std::cout << "[REGISTER] ✓ Guardado: " << personName << "\n";
            } else {
                std::cerr << "[REGISTER] ✗ Error al guardar embedding\n";
            }

        } else if (mode == "recognize") {
            float score;
            std::string name = supabase.findBestMatch(embedding, score, 0.60f);

            std::cout << "[RECOGNIZE] Resultado: " << name 
                      << " (score=" << score << ")\n";

            if (name == "DESCONOCIDO") {
                std::cout << "[RECOGNIZE] ⚠ Persona no reconocida\n";
            } else {
                std::cout << "[RECOGNIZE] ✓ Identificado: " << name << "\n";
            }
        } else {
            std::cerr << "[ERROR] Modo desconocido: " << mode << "\n";
        }

        std::cout << "===========================================\n";
    }

    return 0;
}
