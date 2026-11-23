# Sistema de Gestión de Recursos y Préstamos — CRAI Universidad del Rosario

Aplicación 100% offline en C++17 con API REST local (cpp-httplib), interfaz web administrativa (web/styles.css + JavaScript vanilla, sin CDNs) y persistencia atómica en 7 archivos CSV. Implementa desde cero las 10 estructuras de datos de la asignatura sin contenedores STL, cumpliendo los 29 Requerimientos Funcionales (RQF), la Regla de los 3 y cero fugas de memoria verificadas bajo AddressSanitizer y UBSan.

Autores:
- Oscar David Alvarado Navarrete (DA)
- Nicolás Marín Quintero (NM)

Docente / Product Owner: Tatiana Cabrera Vargas
Institución: Universidad del Rosario — Facultad de Matemáticas e Ingeniería (2026)

================================================================================
1. INICIO RÁPIDO EN LINUX (COMANDO ÚNICO COMENTADO)
================================================================================

# 1. Instalar dependencias del sistema (ejecuta solo la línea de tu distribución si aún no las tienes):
# Ubuntu / Debian: sudo apt-get update && sudo apt-get install -y build-essential libssl-dev nlohmann-json3-dev curl
# Arch Linux:      sudo pacman -S --needed base-devel openssl nlohmann-json curl
# Fedora / RHEL:   sudo dnf install -y gcc-c++ make openssl-devel json-devel curl

# 2. Entrar al directorio del proyecto donde vive el Makefile, descargar httplib.h, correr tests con ASan y arrancar el servidor:
cd Proyecto_CRAI_UR && \
[ -f include/httplib.h ] || curl -fsSL https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o include/httplib.h && \
make clean && make test SANITIZE=1 && \
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 ./bin/tests_crai && \
make clean && make && make run

# Nota: En ejecuciones posteriores dentro de Proyecto_CRAI_UR/, basta con ejecutar: make clean && make && make run

================================================================================
2. PASO A PASO POR SISTEMA OPERATIVO
================================================================================

--- LINUX (Ubuntu / Debian / Arch / Fedora) ---
Paso 1: Instalar g++, OpenSSL, nlohmann-json y curl según tu distro:
  - Ubuntu/Debian: sudo apt-get update && sudo apt-get install -y build-essential libssl-dev nlohmann-json3-dev curl
  - Arch Linux:    sudo pacman -S --needed base-devel openssl nlohmann-json curl
  - Fedora:        sudo dnf install -y gcc-c++ make openssl-devel json-devel curl
Paso 2: Entrar a la carpeta del proyecto y descargar cpp-httplib:
  cd Proyecto_CRAI_UR
  curl -fsSL https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o include/httplib.h
Paso 3: Ejecutar pruebas unitarias con detector de fugas (AddressSanitizer):
  make clean && make test SANITIZE=1
  ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 ./bin/tests_crai
Paso 4: Compilar servidor optimizado (-O2) y ejecutar:
  make clean && make && make run

--- macOS (Homebrew) ---
Paso 1: Instalar herramientas de Apple y dependencias con Homebrew:
  xcode-select --install
  brew install openssl@3 nlohmann-json curl
Paso 2: Entrar a la carpeta del proyecto y descargar cpp-httplib:
  cd Proyecto_CRAI_UR
  curl -fsSL https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o include/httplib.h
Paso 3: Configurar rutas de Homebrew y compilar pruebas y servidor:
  BREW_PREFIX="$(brew --prefix)"
  export CXXFLAGS="-I${BREW_PREFIX}/include -I${BREW_PREFIX}/opt/openssl@3/include"
  export LDFLAGS="-L${BREW_PREFIX}/opt/openssl@3/lib"
  mkdir -p bin
  g++ -std=c++17 -fsanitize=address,undefined -g -O1 -Wall -Wextra -Wpedantic -Iinclude $CXXFLAGS tests/tests_main.cpp -o bin/tests_crai $LDFLAGS -pthread -lcrypto && ./bin/tests_crai
  g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude $CXXFLAGS src/main.cpp -o bin/crai_server $LDFLAGS -pthread -lcrypto && ./bin/crai_server

--- WINDOWS (Opción A: WSL2 Ubuntu - Recomendada) ---
Paso 1: En PowerShell (Administrador) ejecuta: wsl --install -d Ubuntu
Paso 2: Abre Ubuntu e instala las dependencias:
  sudo apt-get update && sudo apt-get install -y build-essential libssl-dev nlohmann-json3-dev curl
Paso 3: Entra a Proyecto_CRAI_UR, descarga httplib.h, compila y ejecuta:
  cd Proyecto_CRAI_UR
  curl -fsSL https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o include/httplib.h
  make clean && make test SANITIZE=1 && ./bin/tests_crai
  make clean && make && make run
Paso 4: Abre http://127.0.0.1:8080 en Chrome o Edge de Windows.

--- WINDOWS (Opción B: Nativo con MSYS2 MINGW64) ---
Paso 1: Instala MSYS2 (https://www.msys2.org/) y abre la terminal MSYS2 MINGW64.
Paso 2: Instala compilador y librerías:
  pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-openssl mingw-w64-x86_64-nlohmann-json curl
Paso 3: Entra a Proyecto_CRAI_UR, descarga httplib.h, compila con sockets Windows y ejecuta:
  cd Proyecto_CRAI_UR
  curl -fsSL https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o include/httplib.h
  mkdir -p bin
  g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude src/main.cpp -o bin/crai_server.exe -pthread -lcrypto -lws2_32 -lcrypt32
  ./bin/crai_server.exe

================================================================================
3. CREDENCIALES Y ACCESO LOCAL (100% OFFLINE)
================================================================================
- URL Local:                http://127.0.0.1:8080
- Documento Administrador:  ADMIN01
- Contraseña Inicial:       CraiUR2026!
- Cooldown fuerza bruta:    60 segundos tras 3 intentos fallidos (HTTP 423)
- Desbloqueo por terminal:  ./bin/crai_server --unlock ADMIN01

================================================================================
4. ESTRUCTURA DEL PROYECTO
================================================================================
Proyecto_CRAI_UR/
├── dependencias/          # 7 CSV locales (inventario.csv con 501 reg., usuarios, préstamos, espera, historial, turnos, categorías)
├── docs/                  # Diagrama de clases UML (.png, .mmd, .md)
├── include/
│   ├── Dominio/           # Recurso.h (Libro, Computador, Sala), Usuario.h (Administrador)
│   ├── Estructuras/       # 10 estructuras propias en C++17 (0% STL, Regla de los 3)
│   ├── Seguridad/         # CryptoHash.h (OpenSSL PBKDF2-HMAC-SHA256), Validador.h
│   └── Servicios/         # AdminCSV.h (escritura atómica .tmp -> rename), GestorCRAI.h (fachada 29 RQF)
├── src/main.cpp           # Servidor REST local (cpp-httplib en 127.0.0.1:8080) + CLI --unlock
├── tests/tests_main.cpp   # Suite automatizada de pruebas unitarias con AddressSanitizer
├── web/                   # Interfaz gráfica offline (index.html, app.js, styles.css)
└── Makefile               # Compilación C++17 (-O2 y modo SANITIZE=1)
