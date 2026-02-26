#!/bin/bash

# =======================
# build.sh - Face Recognition MPI
# =======================

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Directorio de build
BUILD_DIR="build_mpi"

# Función de ayuda
ayuda() {
    echo "Uso: ./build.sh [opciones]"
    echo ""
    echo "Opciones:"
    echo "  -h, --help     Mostrar esta ayuda"
    echo "  -m, --mpi      Compilar versión MPI (default)"
    echo "  -s, --standalone Compilar versión standalone"
    echo "  -a, --all      Compilar ambas versiones"
    echo "  -c, --clean    Limpiar directorio de build"
    echo ""
    echo "Ejemplos:"
    echo "  ./build.sh              # Compilar MPI"
    echo "  ./build.sh --standalone # Compilar standalone"
    echo "  ./build.sh --all        # Compilar ambas"
    echo "  ./build.sh --clean      # Limpiar"
}

# Verificar dependencias
verificar_dependencias() {
    echo -e "${YELLOW}Verificando dependencias...${NC}"
    
    # Verificar cmake
    if ! command -v cmake &> /dev/null; then
        echo -e "${RED}ERROR: cmake no está instalado${NC}"
        exit 1
    fi
    
    # Verificar compilador
    if ! command -v g++ &> /dev/null; then
        echo -e "${RED}ERROR: g++ no está instalado${NC}"
        exit 1
    fi
    
    # Verificar librerías del sistema
    missing=""
    
    # OpenCV
    pkg-config --exists opencv4 2>/dev/null || pkg-config --exists opencv 2>/dev/null || missing="$missing opencv"
    
    # ZeroMQ
    pkg-config --exists libzmq 2>/dev/null || missing="$missing libzmq"
    
    # OpenSSL
    pkg-config --exists openssl 2>/dev/null || missing="$missing openssl"
    
    # MPI
    command -v mpic++ &> /dev/null || missing="$missing openmpi"
    
    if [ -n "$missing" ]; then
        echo -e "${RED}ERROR: Faltan dependencias:$missing${NC}"
        echo "Instalar con:"
        echo "  sudo apt-get install libopencv-dev libzmq3-dev libssl-dev openmpi-bin libopenmpi-dev"
        exit 1
    fi
    
    echo -e "${GREEN}✓ Todas las dependencias instaladas${NC}"
}

# Compilar
compilar() {
    echo -e "${YELLOW}Compilando en $BUILD_DIR/...${NC}"
    
    # Crear directorio si no existe
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configurar con CMake
    cmake ..
    
    # Compilar
    make -j$(nproc)
    
    # Volver al directorio raíz
    cd ..
    
    echo -e "${GREEN}✓ Compilación completada${NC}"
}

# Limpiar
limpiar() {
    echo -e "${YELLOW}Limpiando $BUILD_DIR/...${NC}"
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}✓ Limpieza completada${NC}"
}

# Verificar modelo
verificar_modelo() {
    if [ ! -f "public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx" ]; then
        echo -e "${YELLOW}Descargando modelo ArcFace...${NC}"
        mkdir -p public/face-recognition-resnet100-arcface-onnx
        wget -O public/face-recognition-resnet100-arcface-onnx/arcfaceresnet100-8.onnx \
            "https://media.githubusercontent.com/media/onnx/models/bb0d4cf3d4e2a5f7376c13a08d337e86296edbe8/vision/body_analysis/arcface/model/arcfaceresnet100-8.onnx"
        echo -e "${GREEN}✓ Modelo descargado${NC}"
    else
        echo -e "${GREEN}✓ Modelo ya existe${NC}"
    fi
}

# Verificar .env
verificar_env() {
    if [ ! -f ".env" ]; then
        echo -e "${YELLOW}ADVERTENCIA: No se encontró .env${NC}"
        echo "Creando plantilla..."
        cat > .env << 'EOF'
SUPABASE_URL=tu_url_supabase
SUPABASE_KEY=tu_api_key_supabase
EOF
        echo -e "${RED}ERROR: Edita .env con tus credenciales antes de continuar${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Archivo .env encontrado${NC}"
}

# =======================
# MAIN
# =======================

# Verificar que estamos en el directorio correcto
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}ERROR: Ejecuta desde el directorio raíz del proyecto${NC}"
    exit 1
fi

# Verificar modelo
verificar_modelo

# Verificar .env
verificar_env

# Procesar argumentos
case "${1:-}" in
    -h|--help)
        ayuda
        exit 0
        ;;
    -c|--clean)
        limpiar
        exit 0
        ;;
    -m|--mpi|"")
        verificar_dependencias
        compilar
        echo ""
        echo "Ejecutar con: mpirun --hostfile hosts.txt -np 4 $BUILD_DIR/face_mpi"
        ;;
    -s|--standalone)
        verificar_dependencias
        compilar
        echo ""
        echo "Ejecutar con: $BUILD_DIR/face_receiver"
        ;;
    -a|--all)
        verificar_dependencias
        compilar
        echo ""
        echo "Versiones compiladas:"
        echo "  - MPI:       mpirun --hostfile hosts.txt -np 4 $BUILD_DIR/face_mpi"
        echo "  - Standalone: $BUILD_DIR/face_receiver"
        ;;
    *)
        echo -e "${RED}Opción desconocida: $1${NC}"
        ayuda
        exit 1
        ;;
esac
