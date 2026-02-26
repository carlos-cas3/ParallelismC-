#pragma once
#include <mpi.h>
#include <string>
#include "core/face_processor.hpp"

class WorkerNode {
public:
    WorkerNode(int rank, const std::string& modelPath, 
               const std::string& supabaseUrl, const std::string& apiKey);
    void run();

private:
    int rank;
    std::string modelPath;
    std::string supabaseUrl;
    std::string apiKey;
};
