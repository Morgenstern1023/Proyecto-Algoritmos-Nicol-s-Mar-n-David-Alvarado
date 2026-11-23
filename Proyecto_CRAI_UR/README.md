# Sistema de Gestión de Recursos y Préstamos — CRAI Universidad del Rosario

Aplicación offline en **C++17** con API REST local (`cpp-httplib`) e interfaz web (`Pico.css`), implementando desde cero las 10 estructuras de datos del curso (sin contenedores STL) con estricto cumplimiento de la **Regla de los 3** y **cero fugas de memoria** bajo `AddressSanitizer`.

**Autores:**
- Oscar David Alvarado Navarrete (DA)
- Nicolás Marín Quintero (NM)

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
