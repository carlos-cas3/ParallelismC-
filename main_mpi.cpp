#include <iostream>
#include <mpi.h>
#include "supabase/env_loader.hpp"
#include "mpi/master.hpp"
#include "mpi/worker.hpp"

std::string g_url;
std::string g_api_key;
const std::string MODEL_PATH = "public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx";

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int worldSize, worldRank;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    
    EnvLoader::load(".env");
    g_url = EnvLoader::get("SUPABASE_URL");
    g_api_key = EnvLoader::get("SUPABASE_KEY");
    
    if (g_url.empty() || g_api_key.empty()) {
        std::cerr << "[" << (worldRank == 0 ? "MASTER" : "WORKER") << "] ERROR: Faltan credenciales\n";
        MPI_Finalize();
        return 1;
    }
    
    if (worldSize < 2) {
        if (worldRank == 0) {
            std::cerr << "[MASTER] ERROR: Se necesita al menos 1 worker\n";
            std::cerr << "Uso: mpirun -np 2 ./build_mpi/face_mpi\n";
        }
        MPI_Finalize();
        return 1;
    }
    
    if (worldRank == 0) {
        MasterNode master(worldSize);
        master.run();
    } else {
        WorkerNode worker(worldRank, MODEL_PATH, g_url, g_api_key);
        worker.run();
    }
    
    MPI_Finalize();
    return 0;
}
