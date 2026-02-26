#!/bin/bash
# sync_worker.sh - Sincronizar proyecto a un worker
# Uso: ./sync_worker.sh <hostname>
# Ejemplo: ./sync_worker.sh client1

WORKER=$1
USER=$(whoami)
PROJECT_DIR=$(basename "$PWD")

if [ -z "$WORKER" ]; then
    echo "Uso: ./sync_worker.sh <hostname>"
    echo "Ejemplo: ./sync_worker.sh client1"
    echo ""
    echo "Workers configurados actualmente:"
    if [ -f "hosts.txt" ]; then
        grep -v "^#" hosts.txt | grep -v "^$" | awk -F' slots=' '{print "  - " $1 " (slots=" $2 ")"}'
    fi
    exit 1
fi

echo "=========================================="
echo "  Sincronizando proyecto: $PROJECT_DIR"
echo "  Con worker: $WORKER"
echo "=========================================="

# Verificar que el ejecutable existe
if [ ! -f "build_mpi/face_mpi" ]; then
    echo "ERROR: build_mpi/face_mpi no existe"
    echo "Ejecuta primero: ./build.sh --mpi"
    exit 1
fi

# Verificar que el modelo existe
if [ ! -f "public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx" ]; then
    echo "ERROR: Modelo ONNX no encontrado"
    exit 1
fi

# Verificar que .env existe
if [ ! -f ".env" ]; then
    echo "ERROR: .env no encontrado"
    exit 1
fi

# 1. Copiar install_deps.sh al worker
echo ""
echo "[1/5] Copiando script de instalación..."
scp install_deps.sh $USER@$WORKER:~/$PROJECT_DIR/

# 2. Ejecutar install_deps.sh en el worker
echo "[2/5] Instalando dependencias en $WORKER..."
ssh -o StrictHostKeyChecking=no $USER@$WORKER "cd ~/$PROJECT_DIR && chmod +x install_deps.sh && sudo ./install_deps.sh"

# 3. Crear directorio remoto
echo "[3/5] Creando directorio en worker..."
ssh $USER@$WORKER "mkdir -p ~/$PROJECT_DIR/{build_mpi,public}"

# 4. Copiar ejecutable compilado
echo "[4/5] Copiando ejecutable..."
scp build_mpi/face_mpi $USER@$WORKER:~/$PROJECT_DIR/build_mpi/

# 5. Copiar modelo ONNX
echo "[5/5] Copiando modelo ONNX y credenciales..."
scp -r public/face-recognition-resnet100-arcface-onnx \
    $USER@$WORKER:~/$PROJECT_DIR/public/
scp .env $USER@$WORKER:~/$PROJECT_DIR/

echo ""
echo "=========================================="
echo "  ✓ Worker $WORKER configurado"
echo "=========================================="
echo ""
echo "Para ejecutar desde el master:"
echo "  mpirun --hostfile hosts.txt -np 4 build_mpi/face_mpi"
