#!/bin/bash
# sync_worker.sh - Sincronizar proyecto a un worker
# Uso: ./sync_worker.sh <hostname>
# Ejemplo: ./sync_worker.sh client1

WORKER=$1
USER=$(whoami)
PROJECT_DIR="zmq_test"

if [ -z "$WORKER" ]; then
    echo "Uso: ./sync_worker.sh <hostname>"
    echo "Ejemplo: ./sync_worker.sh client1"
    echo ""
    echo "Workers configurados actualmente:"
    if [ -f "hosts.txt" ]; then
        grep -v "^#" hosts.txt | grep -v "^$" | awk '{print "  - " $1 " (slots=" $2 ")"}'
    fi
    exit 1
fi

echo "=========================================="
echo "  Sincronizando con worker: $WORKER"
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

# 1. Instalar dependencias en el worker
echo ""
echo "[1/5] Instalando dependencias en $WORKER..."
ssh -o StrictHostKeyChecking=no $USER@$WORKER "sudo apt-get update && sudo apt-get install -y \
    build-essential cmake libopencv-dev libzmq3-dev libssl-dev \
    openmpi-bin libopenmpi-dev" 2>/dev/null || {
    echo "WARNING: No se pudieron instalar dependencias automáticamente"
    echo "         Instalar manualmente en $WORKER: ./install_deps.sh"
}

# 2. Crear directorio remoto
echo "[2/5] Creando directorio en worker..."
ssh $USER@$WORKER "mkdir -p ~/$PROJECT_DIR/{build_mpi,public}"

# 3. Copiar ejecutable compilado
echo "[3/5] Copiando ejecutable..."
scp build_mpi/face_mpi $USER@$WORKER:~/$PROJECT_DIR/build_mpi/

# 4. Copiar modelo ONNX
echo "[4/5] Copiando modelo ONNX..."
scp -r public/face-recognition-resnet100-arcface-onnx \
    $USER@$WORKER:~/$PROJECT_DIR/public/

# 5. Copiar credenciales
echo "[5/5] Copiando credenciales..."
scp .env $USER@$WORKER:~/$PROJECT_DIR/

echo ""
echo "=========================================="
echo "  ✓ Worker $WORKER configurado"
echo "=========================================="
echo ""
echo "Para ejecutar desde el master:"
echo "  mpirun --hostfile hosts.txt -np 4 build_mpi/face_mpi"
