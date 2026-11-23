# Sistema de Gestión de Recursos y Préstamos — CRAI Universidad del Rosario

Aplicación 100% offline en **C++17** con API REST local (`cpp-httplib`), interfaz web administrativa (`web/styles.css` + JS vanilla, cero CDNs) y persistencia atómica en 7 archivos CSV. Implementa desde cero las **10 estructuras de datos de la asignatura sin contenedores STL**, cumpliendo la **Regla de los 3** y **cero fugas de memoria** verificadas bajo `AddressSanitizer` y `UBSan`.

**Autores:**
- **Oscar David Alvarado Navarrete (DA)**
- **Nicolás Marín Quintero (NM)**

**Docente / Product Owner:** Tatiana Cabrera Vargas  
**Institución:** Universidad del Rosario — Algoritmos y Estructuras de Datos (Línea 1)

---

## Requisitos Previos e Instalación por Sistema Operativo

### 1. Linux (Arch Linux / Manjaro)
```bash
sudo pacman -S --needed base-devel openssl nlohmann-json curl
```

### 2. Linux (Ubuntu / Debian / WSL2)
```bash
sudo apt-get update
sudo apt-get install -y build-essential libssl-dev nlohmann-json3-dev curl
```

### 3. macOS (Homebrew)
```bash
brew install openssl@3 nlohmann-json curl
```

### 4. Windows
* **Opción A — WSL2 (Ubuntu en Windows, recomendada para `Makefile` y `AddressSanitizer`):** instala WSL2 (`wsl --install`), abre Ubuntu y sigue los pasos de **Ubuntu / Debian**. Luego abre `http://127.0.0.1:8080` en tu navegador de Windows.
* **Opción B — Windows Nativo (MSYS2 MINGW64):**
  ```bash
  pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-openssl mingw-w64-x86_64-nlohmann-json curl
  mkdir -p bin
  g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude src/main.cpp -o bin/crai_server.exe -pthread -lcrypto -lws2_32 -lcrypt32
  ./bin/crai_server.exe
  ```

---

## Descarga de Dependencia Header-Only (`cpp-httplib`)

Desde la raíz de `Proyecto_CRAI_UR/`:
```bash
curl -fsSL https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o include/httplib.h
```

---

## Compilación, Pruebas (AddressSanitizer) y Ejecución

```bash
# 1. Compilar y correr suite de pruebas con AddressSanitizer + UBSan (0 fugas)
make clean && make test SANITIZE=1
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 ./bin/tests_crai

# 2. Compilar servidor optimizado y ejecutar
make clean && make && make run
```

Al iniciar, el servidor abre automáticamente `http://127.0.0.1:8080`.

---

## Credenciales y Desbloqueo de Emergencia (CLI)

- **Documento Administrador:** `ADMIN01`
- **Contraseña inicial:** `CraiUR2026!`
- **Cooldown anti fuerza bruta:** 60 segundos tras 3 intentos fallidos (`HTTP 423`).
- **Desbloqueo local por terminal:**
  ```bash
  ./bin/crai_server --unlock ADMIN01
  ```
