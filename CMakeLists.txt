cmake_minimum_required(VERSION 3.10)
project(FaceRecognition)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(cppzmq REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(MPI REQUIRED)

include_directories(
    ${OpenCV_INCLUDE_DIRS} 
    ${MPI_CXX_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
)

set(CORE_SRC
    core/face_processor.cpp
)

set(SUPABASE_SRC
    supabase/http_client.cpp
    supabase/json_utils.cpp
    supabase/person_repo.cpp
    supabase/embedding_repo.cpp
    supabase/face_quality.cpp
)

set(COMMON_SRC
    zmq/receiver.cpp zmq/sender.cpp
    arcface/arcface.cpp
    ${SUPABASE_SRC}
    ${CORE_SRC}
)

# MPI distribuido (master + workers)
add_executable(face_mpi main_mpi.cpp mpi/master.cpp mpi/worker.cpp ${COMMON_SRC})
target_link_libraries(face_mpi ${OpenCV_LIBS} cppzmq OpenSSL::SSL OpenSSL::Crypto Threads::Threads ${MPI_CXX_LIBRARIES})

# Standalone sin MPI
add_executable(face_receiver main.cpp ${COMMON_SRC})
target_link_libraries(face_receiver ${OpenCV_LIBS} cppzmq OpenSSL::SSL OpenSSL::Crypto Threads::Threads)

set(MODEL ${CMAKE_SOURCE_DIR}/public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx)
if(NOT EXISTS ${MODEL})
    file(DOWNLOAD https://media.githubusercontent.com/media/onnx/models/bb0d4cf3d4e2a5f7376c13a08d337e86296edbe8/vision/body_analysis/arcface/model/arcfaceresnet100-8.onnx ${MODEL} SHOW_PROGRESS)
endif()
