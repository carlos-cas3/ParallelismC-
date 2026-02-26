#pragma once
#include <mpi.h>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "zmq/receiver.hpp"
#include "zmq/sender.hpp"
#include "mpi/task.hpp"
#include "core/response_builder.hpp"

class MasterNode {
public:
    MasterNode(int worldSize);
    ~MasterNode();
    void run();
    void shutdown();

private:
    struct SharedState {
        std::mutex mutex;
        std::condition_variable cv;
        std::queue<FaceTask> task_queue;
        std::atomic<bool> running{true};
    };

    int worldSize;
    SharedState state;
    std::atomic<int> globalTaskCounter{0};

    void registerReceiverThread();
    void recognizeReceiverThread();
    void dispatcherLoop();
};
