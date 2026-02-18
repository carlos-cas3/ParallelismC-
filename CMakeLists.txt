cmake_minimum_required(VERSION 3.10)
project(FaceRecognition)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package(cppzmq REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Fuentes compartidas entre ambos ejecutables
set(SUPABASE_SOURCES
    supabase/http_client.cpp
    supabase/json_utils.cpp
    supabase/person_repo.cpp
    supabase/embedding_repo.cpp
)

# --- Ejecutable principal ---
add_executable(face_receiver
    main.cpp
    zmq/receiver.cpp
    zmq/sender.cpp
    arcface/arcface.cpp
    ${SUPABASE_SOURCES}
)
target_link_libraries(face_receiver
    ${OpenCV_LIBS}
    cppzmq
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# --- Ejecutable de prueba (sin Supabase) ---
add_executable(test_receiver
    main.cpp
    zmq/receiver.cpp
    zmq/sender.cpp
    arcface/arcface.cpp
    ${SUPABASE_SOURCES}
)
target_link_libraries(test_receiver
    ${OpenCV_LIBS}
    cppzmq
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

target_compile_options(face_receiver PRIVATE -Wall -Wextra)
target_compile_options(test_receiver PRIVATE -Wall -Wextra)

set(MODEL_DIR
    ${CMAKE_SOURCE_DIR}/public/face-recognition-resnet100-arcface-onnx
)

set(MODEL_FILE
    ${MODEL_DIR}/arcfaceresnet100-8.onnx
)

file(MAKE_DIRECTORY ${MODEL_DIR})

if(NOT EXISTS ${MODEL_FILE})
    message(STATUS "Descargando modelo ArcFace ONNX en public/...")

    file(DOWNLOAD
        https://media.githubusercontent.com/media/onnx/models/bb0d4cf3d4e2a5f7376c13a08d337e86296edbe8/vision/body_analysis/arcface/model/arcfaceresnet100-8.onnx
        ${MODEL_FILE}
        SHOW_PROGRESS
    )
endif()
