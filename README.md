# Instalación y Ejecución

Este proyecto requiere instalar dependencias, compilar y configurar variables de entorno antes de ejecutarse.

## Pasos

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
./build.sh
```

### 4. Crear el archivo `.env`

Crear el archivo en la raíz del proyecto:

```bash
touch .env
```

Luego editarlo y agregar las variables necesarias.

### 5. Ejecutar la aplicación

```bash
./build/face_receiver
```

---

## Notas

- Si no se puede ejecutar los scripts actualizar los permisos:

```bash
chmod +x install_deps.sh
chmod +x build.sh
```
