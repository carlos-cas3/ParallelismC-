# Face Recognition MPI

Sistema de reconocimiento facial distribuido usando MPI, ZeroMQ y Supabase.

## Requisitos

- Ubuntu 20.04+ o similar
- Acceso a internet (para modelo y Supabase)

## Instalación y Ejecución

### 1. Clonar el repositorio

```bash
git clone https://github.com/carlos-cas3/ParallelismC-.git
cd ParallelismC-
```

### 2. Instalar dependencias

```bash
./install_deps.sh
```

### 3. Compilar el proyecto

```bash
./build.sh --mpi
```

### 4. Configurar credenciales

Editar el archivo `.env` con tus credenciales de Supabase:

```bash
SUPABASE_URL=tu_url_supabase
SUPABASE_KEY=tu_api_key_supabase
```

### 5. Configurar workers (opcional)

Para agregar workers, editar `hosts.txt` y luego:

```bash
./sync_worker.sh client1
```

### 6. Ejecutar

```bash
# Modo local (1 master + workers locales)
mpirun --oversubscribe -np 4 build_mpi/face_mpi

# Con workers remotos (editar hosts.txt primero)
mpirun --hostfile hosts.txt -np 4 build_mpi/face_mpi
```

---

## Estructura del Proyecto

```
├── build_mpi/         # Ejecutable compilado
├── mpi/               # Código Master y Worker
├── core/              # Procesamiento de caras
├── supabase/          # Conexión a Supabase
├── zmq/               # Comunicación ZMQ
├── public/            # Modelo ONNX
├── hosts.txt          # Configuración de workers
├── build.sh           # Compilar
├── install_deps.sh    # Instalar dependencias
└── sync_worker.sh    # Sincronizar workers
```

---

## Scripts Disponibles

| Script | Descripción |
|--------|-------------|
| `./build.sh` | Compilar versión MPI |
| `./build.sh --standalone` | Compilar sin MPI |
| `./build.sh --clean` | Limpiar build |
| `./install_deps.sh` | Instalar dependencias |
| `./sync_worker.sh <host>` | Sincronizar con worker |

---

## Permisos

Si los scripts no se ejecutan, actualizar permisos:

```bash
chmod +x install_deps.sh build.sh sync_worker.sh
```
