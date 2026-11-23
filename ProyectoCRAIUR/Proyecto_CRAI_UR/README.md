# Sistema de Gestión de Recursos y Préstamos — CRAI Universidad del Rosario

Aplicación offline en **C++17** con API REST local (`cpp-httplib`) e interfaz web (`Pico.css`), implementando desde cero las 10 estructuras de datos del curso (sin contenedores STL) con estricto cumplimiento de la **Regla de los 3** y **cero fugas de memoria** bajo `AddressSanitizer`.

**Autores:**
- Oscar David Alvarado Navarrete (DA)
- Nicolás Marín Quintero (NM)

**Docente / Product Owner:** Tatiana Cabrera Vargas  
**Institución:** Universidad del Rosario — Facultad de Matemáticas e Ingeniería (2026)

---
M)

## Requisitos en Arch Linux
```bash
sudo pacman -S --needed base-devel openssl nlohmann-json curl
curl -fsSL https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o include/httplib.h
curl -fsSL https://cdn.jsdelivr.net/npm/@picocss/pico@2/css/pico.min.css -o web/pico.min.css
```

## Compilación y Pruebas (AddressSanitizer)
```bash
make test SANITIZE=1 && ./bin/tests_crai
make clean && make && make run
```

## Credenciales Iniciales y Desbloqueo Local
- **Documento Admin:** `ADMIN01`
- **Contraseña inicial:** `CraiUR2026!`
- **Cooldown anti fuerza bruta:** 60 segundos tras 3 intentos fallidos consecutivos.
- **Desbloqueo local de emergencia (terminal):**
  ```bash
  ./bin/crai_server --unlock ADMIN01
  ```

# Sistema de Gestión de Recursos y Préstamos — CRAI Universidad del Rosario

Aplicación **100% offline** en **C++17** para la gestión integral de préstamos, inventario, usuarios, sanciones, categorías jerárquicas, disponibilidad semanal y red de sedes del CRAI de la Universidad del Rosario.

Implementa desde cero las **10 estructuras de datos de la asignatura sin contenedores STL** (`Nodo`, `VectorDinamico`, `ListaSimple`, `ListaDoble`, `Pila`, `Cola`, `TablaHash`, `ArbolAVL`, `ArbolCategorias` LCRS, `MatrizDisponibilidad` 3D y `GrafoSedes` BFS/DFS), cumpliendo los **29 Requerimientos Funcionales (RQF)**, persistencia atómica en **7 archivos CSV** (catálogo base de **501 recursos**), seguridad con **OpenSSL** (`PBKDF2-HMAC-SHA256`), interfaz web local (`cpp-httplib`, cero CDNs) y **0 fugas de memoria** verificadas con `AddressSanitizer` / `UBSan` y GitHub Actions.

## Estructura del Proyecto

```text
Proyecto_CRAI_UR/
├── dependencias/          # 7 archivos CSV locales (501 recursos, usuarios, préstamos, espera, historial, turnos, categorías)
├── docs/                  # Diagrama de clases UML (.png, .mmd, .md)
├── include/
│   ├── Dominio/           # Recurso.h (Libro, Computador, Sala), Usuario.h (Administrador)
│   ├── Estructuras/       # 10 estructuras propias en C++17 (0% STL, Regla de los 3)
│   ├── Seguridad/         # CryptoHash.h (OpenSSL PBKDF2), Validador.h
│   └── Servicios/         # AdminCSV.h (escritura atómica), GestorCRAI.h (fachada 29 RQF)
├── src/main.cpp           # Servidor REST local (cpp-httplib en 127.0.0.1:8080) + CLI --unlock
├── tests/tests_main.cpp   # Suite automatizada de pruebas unitarias (AddressSanitizer)
├── web/                   # Interfaz gráfica offline (index.html, app.js, styles.css)
└── Makefile               # Compilación C++17 (-O2 y modo SANITIZE=1)