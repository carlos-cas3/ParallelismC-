# Instalación y Ejecución

Este proyecto requiere instalar dependencias, compilar y configurar variables de entorno antes de ejecutarse.

## Pasos

### 1. Instalar dependencias

```bash
./install_deps.sh
```

### 2. Compilar el proyecto

```bash
./build.sh
```

### 3. Crear el archivo `.env`

Crear el archivo en la raíz del proyecto:

```bash
touch .env
```

Luego editarlo y agregar las variables necesarias.

### 4. Ejecutar la aplicación

```bash
./build/face_receiver
```

---

## Notas

- Asegúrate de tener permisos de ejecución en los scripts:

```bash
chmod +x install_deps.sh
chmod +x build.sh
```
