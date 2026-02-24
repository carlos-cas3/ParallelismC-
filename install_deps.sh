#!/bin/bash
# install_deps.sh - Instalar dependencias del sistema
# Ejecutar en cada máquina (master y workers)

set -e

echo "=========================================="
echo "  Instalando dependencias del sistema"
echo "=========================================="

sudo apt-get update

sudo apt-get install -y \
    build-essential \
    cmake \
    libopencv-dev \
    libzmq3-dev \
    libssl-dev \
    openmpi-bin \
    libopenmpi-dev

echo ""
echo "✓ Dependencias instaladas correctamente"
echo ""
echo "Siguiente paso:"
echo "  - Master: ./sync_worker.sh <worker_hostname>"
echo "  - Workers: Se configurarán automáticamente desde el master"
