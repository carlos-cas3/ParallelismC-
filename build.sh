#!/bin/bash

# Crear directorio de compilación
mkdir -p build
cd build

# Compilar
cmake ..
make -j$(nproc)

# Volver al directorio raíz
cd ..

echo "✓ Compilación completa. Ejecutar con: ./build/face_receiver"
