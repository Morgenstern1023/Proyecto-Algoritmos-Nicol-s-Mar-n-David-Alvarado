/**
 * @file AdminCSV.h
 * @brief Persistencia offline 100% propia en archivos CSV con escritura atómica y tolerancia a corrupción (RQF 018, 024, 025).
 * @author Oscar David Alvarado Navarrete (DA)
 */

#ifndef ADMIN_CSV_H
#define ADMIN_CSV_H

#include "../Dominio/Recurso.h"
#include "../Dominio/Usuario.h"
#include "../Estructuras/ArbolCategorias.h"
#include "../Estructuras/MatrizDisponibilidad.h"
#include "../Estructuras/VectorDinamico.h"
#include "../Seguridad/Validador.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

struct RegistroRecursoCSV {
    std::string categoria;
    std::string codigo;
    std::string nombre;
    std::string estado;
    bool disponible;
    int sedeId;
    int anioAdquisicion;
    int vecesPrestado;
    std::string subcategoria;
    std::string detalleExtra;

    RegistroRecursoCSV()
        : categoria(""), codigo(""), nombre(""), estado("disponible"),
          disponible(true), sedeId(0), anioAdquisicion(2024),
          vecesPrestado(0), subcategoria("General"), detalleExtra("") {}
};

struct RegistroUsuarioCSV {
    std::string documento;
    std::string nombre;
    std::string apellido;
    bool sancionado;
    bool esAdmin;
    std::string credencialHash;
    int intentosFallidos;
    long long bloqueadoHasta;

    RegistroUsuarioCSV()
        : documento(""), nombre(""), apellido(""), sancionado(false),
          esAdmin(false), credencialHash(""), intentosFallidos(0), bloqueadoHasta(0) {}
};

struct RegistroPrestamoCSV {
    int idPrestamo;
    std::string codigoRecurso;
    std::string documentoUsuario;
    int sedeId;
    std::string fechaPrestamo;
    std::string fechaDevolucion;
    bool activo;

    RegistroPrestamoCSV()
        : idPrestamo(0), codigoRecurso(""), documentoUsuario(""),
          sedeId(0), fechaPrestamo(""), fechaDevolucion(""), activo(true) {}
};

struct RegistroEsperaCSV {
    std::string codigoRecurso;
    std::string documentoUsuario;

    RegistroEsperaCSV(const std::string& cod = "", const std::string& doc = "")
        : codigoRecurso(cod), documentoUsuario(doc) {}
};

struct RegistroHistorialCSV {
    std::string fechaHora;
    std::string tipoEvento;
    std::string codigoRecurso;
    std::string documentoUsuario;
    std::string detalle;

    RegistroHistorialCSV()
        : fechaHora(""), tipoEvento(""), codigoRecurso(""), documentoUsuario(""), detalle("") {}
};

struct RegistroTurnoCSV {
    std::string codigoRecurso;
    int sedeId;
    int dia;
    bool ocupado;

    RegistroTurnoCSV()
        : codigoRecurso(""), sedeId(0), dia(0), ocupado(false) {}
};

struct RegistroCategoriaCSV {
    std::string nombreCategoria;
    std::string nombrePadre;

    RegistroCategoriaCSV(const std::string& cat = "", const std::string& padre = "")
        : nombreCategoria(cat), nombrePadre(padre) {}
};

class AdminCSV {
private:
    static int extraerNumeroCodigo(const std::string& codigo) {
        int acum = 0;
        for (size_t i = 0; i < codigo.size(); ++i) {
            if (codigo[i] >= '0' && codigo[i] <= '9') {
                acum = acum * 10 + (codigo[i] - '0');
            }
        }
        return acum;
    }

    static bool reemplazarArchivoAtomico(const std::string& rutaTmp, const std::string& rutaFinal) {
#ifdef _WIN32
        std::remove(rutaFinal.c_str());
#endif
        if (std::rename(rutaTmp.c_str(), rutaFinal.c_str()) != 0) {
            std::cerr << "[ERROR] [AdminCSV] Fallo en reemplazo atómico hacia: " << rutaFinal << std::endl;
            std::remove(rutaTmp.c_str());
            return false;
        }
        return true;
    }

public:
    static bool cargarInventario(
        const std::string& ruta,
        VectorDinamico<RegistroRecursoCSV>& salida,
        int numSedesValidas,
        int& filasOmitidas
    ) {
        filasOmitidas = 0;
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) {
            std::cerr << "[ERROR] [AdminCSV] No se pudo abrir inventario en: " << ruta << std::endl;
            return false;
        }
        std::string linea;
        if (!std::getline(archivo, linea)) {
            std::cout << "[WARN] [AdminCSV] Archivo inventario vacío: " << ruta << std::endl;
            return false;
        }

        int numLinea = 1;
        while (std::getline(archivo, linea)) {
            numLinea++;
            if (Validador::normalizarTexto(linea).empty()) continue;
            try {
                VectorDinamico<std::string> cols = Validador::dividirCSV(linea);
                RegistroRecursoCSV reg;

                if (cols.getTamano() == 4) {
                    reg.categoria = Validador::aMinusculas(cols[0]);
                    reg.codigo = Validador::sanitizarCampoCSV(cols[1]);
                    reg.estado = Validador::aMinusculas(cols[2]);
                    if (Validador::esNuloOProhibido(reg.categoria) ||
                        Validador::esNuloOProhibido(reg.codigo) ||
                        !Validador::parsearBooleanoSeguro(cols[3], reg.disponible)) {
                        throw std::invalid_argument("Campos obligatorios inválidos (formato 4 cols)");
                    }

                    int num = extraerNumeroCodigo(reg.codigo);
                    reg.sedeId = (numSedesValidas > 0) ? (num % numSedesValidas) : 0;
                    reg.anioAdquisicion = 2018 + (num % 9);
                    reg.vecesPrestado = (num * 7) % 45;

                    if (reg.categoria == "libro") {
                        reg.nombre = "Libro Academico " + reg.codigo;
                        reg.subcategoria = (num % 2 == 0) ? "Ingenieria" : "Ciencias Basicas";
                        reg.detalleExtra = "Editorial UR";
                    } else if (reg.categoria == "computador") {
                        reg.nombre = "Estacion Portatil " + reg.codigo;
                        reg.subcategoria = "Equipos de Computo";
                        reg.detalleExtra = (num % 2 == 0) ? "Arch Linux" : "Windows 11";
                    } else if (reg.categoria == "sala") {
                        reg.nombre = "Sala de Estudio " + reg.codigo;
                        reg.subcategoria = "Espacios Colaborativos";
                        reg.detalleExtra = std::to_string(4 + (num % 7));
                    } else {
                        throw std::invalid_argument("Categoría desconocida: " + reg.categoria);
                    }
                    salida.push_back(reg);
                } else if (cols.getTamano() == 10) {
                    reg.categoria = Validador::aMinusculas(cols[0]);
                    reg.codigo = Validador::sanitizarCampoCSV(cols[1]);
                    reg.nombre = Validador::sanitizarCampoCSV(cols[2]);
                    reg.estado = Validador::aMinusculas(cols[3]);

                    if (Validador::esNuloOProhibido(reg.categoria) ||
                        Validador::esNuloOProhibido(reg.codigo) ||
                        Validador::esNuloOProhibido(reg.nombre) ||
                        (reg.categoria != "libro" && reg.categoria != "computador" && reg.categoria != "sala")) {
                        throw std::invalid_argument("Identificador o categoría corrupta");
                    }
                    if (!Validador::parsearBooleanoSeguro(cols[4], reg.disponible)) {
                        throw std::invalid_argument("Booleano disponible corrupto");
                    }
                    int maxSede = (numSedesValidas > 0) ? (numSedesValidas - 1) : 0;
                    if (!Validador::parsearEnteroSeguro(cols[5], reg.sedeId, 0, maxSede)) {
                        throw std::invalid_argument("sedeId fuera de rango");
                    }
                    if (!Validador::parsearEnteroSeguro(cols[6], reg.anioAdquisicion, 1950, 2100)) {
                        throw std::invalid_argument("anioAdquisicion inválido");
                    }
                    if (!Validador::parsearEnteroSeguro(cols[7], reg.vecesPrestado, 0, 1000000)) {
                        throw std::invalid_argument("vecesPrestado inválido");
                    }
                    reg.subcategoria = Validador::sanitizarOpcional(cols[8], "General");
                    reg.detalleExtra = Validador::sanitizarOpcional(cols[9], "Estandar");
                    salida.push_back(reg);
                } else {
                    throw std::invalid_argument("Número de columnas inesperado: " + std::to_string(cols.getTamano()));
                }
            } catch (const std::exception& e) {
                filasOmitidas++;
                std::cout << "[WARN] [AdminCSV] Línea " << numLinea
                          << " corrupta en inventario.csv omitida: " << e.what() << std::endl;
            }
        }
        return true;
    }

    static bool guardarInventario(const std::string& ruta, const VectorDinamico<Recurso*>& recursos) {
        std::string rutaTmp = ruta + ".tmp";
        std::ofstream archivo(rutaTmp, std::ios::out | std::ios::trunc);
        if (!archivo.is_open()) {
            std::cerr << "[ERROR] [AdminCSV] No se pudo crear temporal: " << rutaTmp << std::endl;
            return false;
        }
        archivo << "categoria,codigo,nombre,estado,disponible,sede_id,anio_adquisicion,veces_prestado,subcategoria,detalle_extra\n";
        for (int i = 0; i < recursos.getTamano(); ++i) {
            const Recurso* r = recursos[i];
            if (r == nullptr) continue;
            archivo << Validador::sanitizarCampoCSV(r->getCategoria()) << ','
                    << Validador::sanitizarCampoCSV(r->getCodigo()) << ','
                    << Validador::sanitizarCampoCSV(r->getNombre()) << ','
                    << Validador::sanitizarCampoCSV(r->getEstado()) << ','
                    << (r->isDisponible() ? "1" : "0") << ','
                    << r->getSedeId() << ','
                    << r->getAnioAdquisicion() << ','
                    << r->getVecesPrestado() << ','
                    << Validador::sanitizarCampoCSV(r->getSubcategoria()) << ','
                    << Validador::sanitizarCampoCSV(r->getDetalleExtra()) << '\n';
        }
        archivo.flush();
        bool ok = archivo.good();
        archivo.close();
        if (!ok) {
            std::cerr << "[ERROR] [AdminCSV] Fallo de escritura en: " << rutaTmp << std::endl;
            std::remove(rutaTmp.c_str());
            return false;
        }
        return reemplazarArchivoAtomico(rutaTmp, ruta);
    }

    static bool cargarUsuarios(
        const std::string& ruta,
        VectorDinamico<RegistroUsuarioCSV>& salida,
        int& filasOmitidas
    ) {
        filasOmitidas = 0;
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) {
            std::cout << "[INFO] [AdminCSV] usuarios.csv no encontrado; se inicializará nuevo." << std::endl;
            return false;
        }
        std::string linea;
        if (!std::getline(archivo, linea)) return false;

        int numLinea = 1;
        while (std::getline(archivo, linea)) {
            numLinea++;
            if (Validador::normalizarTexto(linea).empty()) continue;
            try {
                VectorDinamico<std::string> cols = Validador::dividirCSV(linea);
                if (cols.getTamano() != 8) {
                    throw std::invalid_argument("Se esperaban 8 columnas");
                }
                RegistroUsuarioCSV reg;
                reg.documento = Validador::sanitizarCampoCSV(cols[0]);
                reg.nombre = Validador::sanitizarCampoCSV(cols[1]);
                reg.apellido = Validador::sanitizarCampoCSV(cols[2]);
                if (Validador::esNuloOProhibido(reg.documento) ||
                    Validador::esNuloOProhibido(reg.nombre) ||
                    Validador::esNuloOProhibido(reg.apellido)) {
                    throw std::invalid_argument("Datos personales nulos o prohibidos");
                }
                if (!Validador::parsearBooleanoSeguro(cols[3], reg.sancionado) ||
                    !Validador::parsearBooleanoSeguro(cols[4], reg.esAdmin) ||
                    !Validador::parsearEnteroSeguro(cols[6], reg.intentosFallidos, 0, 100) ||
                    !Validador::parsearLongLongSeguro(cols[7], reg.bloqueadoHasta, 0LL, 4102444800LL)) {
                    throw std::invalid_argument("Campos booleanos/numéricos corruptos");
                }
                reg.credencialHash = Validador::normalizarTexto(cols[5]);
                if (reg.esAdmin && reg.credencialHash.find('$') == std::string::npos) {
                    throw std::invalid_argument("Hash de administrador corrupto");
                }
                salida.push_back(reg);
            } catch (const std::exception& e) {
                filasOmitidas++;
                std::cout << "[WARN] [AdminCSV] Línea " << numLinea
                          << " corrupta en usuarios.csv omitida: " << e.what() << std::endl;
            }
        }
        return true;
    }

    static bool guardarUsuarios(const std::string& ruta, const VectorDinamico<Usuario*>& usuarios) {
        std::string rutaTmp = ruta + ".tmp";
        std::ofstream archivo(rutaTmp, std::ios::out | std::ios::trunc);
        if (!archivo.is_open()) {
            std::cerr << "[ERROR] [AdminCSV] No se pudo crear temporal: " << rutaTmp << std::endl;
            return false;
        }
        archivo << "documento,nombre,apellido,sancionado,es_admin,credencial_hash,intentos_fallidos,bloqueado_hasta\n";
        for (int i = 0; i < usuarios.getTamano(); ++i) {
            const Usuario* u = usuarios[i];
            if (u == nullptr) continue;
            const Administrador* admin = dynamic_cast<const Administrador*>(u);
            archivo << Validador::sanitizarCampoCSV(u->getDocumento()) << ','
                    << Validador::sanitizarCampoCSV(u->getNombre()) << ','
                    << Validador::sanitizarCampoCSV(u->getApellido()) << ','
                    << (u->isSancionado() ? "1" : "0") << ','
                    << (admin != nullptr ? "1" : "0") << ','
                    << (admin != nullptr ? admin->getCredencialHash() : "") << ','
                    << (admin != nullptr ? admin->getIntentosFallidos() : 0) << ','
                    << (admin != nullptr ? admin->getBloqueadoHasta() : 0LL) << '\n';
        }
        archivo.flush();
        bool ok = archivo.good();
        archivo.close();
        if (!ok) {
            std::cerr << "[ERROR] [AdminCSV] Fallo de escritura en: " << rutaTmp << std::endl;
            std::remove(rutaTmp.c_str());
            return false;
        }
        return reemplazarArchivoAtomico(rutaTmp, ruta);
    }

    static bool cargarPrestamos(
        const std::string& ruta,
        VectorDinamico<RegistroPrestamoCSV>& salida,
        int numSedesValidas,
        int& filasOmitidas
    ) {
        filasOmitidas = 0;
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) return false;
        std::string linea;
        if (!std::getline(archivo, linea)) return false;

        int numLinea = 1;
        while (std::getline(archivo, linea)) {
            numLinea++;
            if (Validador::normalizarTexto(linea).empty()) continue;
            try {
                VectorDinamico<std::string> cols = Validador::dividirCSV(linea);
                if (cols.getTamano() != 7) throw std::invalid_argument("Se esperaban 7 columnas");
                RegistroPrestamoCSV reg;
                int maxSede = (numSedesValidas > 0) ? (numSedesValidas - 1) : 0;
                if (!Validador::parsearEnteroSeguro(cols[0], reg.idPrestamo, 1, 10000000) ||
                    Validador::esNuloOProhibido(cols[1]) ||
                    Validador::esNuloOProhibido(cols[2]) ||
                    !Validador::parsearEnteroSeguro(cols[3], reg.sedeId, 0, maxSede) ||
                    !Validador::parsearBooleanoSeguro(cols[6], reg.activo)) {
                    throw std::invalid_argument("Campos corruptos en prestamos.csv");
                }
                reg.codigoRecurso = Validador::sanitizarCampoCSV(cols[1]);
                reg.documentoUsuario = Validador::sanitizarCampoCSV(cols[2]);
                reg.fechaPrestamo = Validador::sanitizarOpcional(cols[4], "Sin fecha");
                reg.fechaDevolucion = Validador::sanitizarCampoCSV(cols[5]);
                salida.push_back(reg);
            } catch (const std::exception& e) {
                filasOmitidas++;
                std::cout << "[WARN] [AdminCSV] Línea " << numLinea
                          << " corrupta en prestamos.csv omitida: " << e.what() << std::endl;
            }
        }
        return true;
    }

    static bool guardarPrestamos(const std::string& ruta, const VectorDinamico<RegistroPrestamoCSV>& prestamos) {
        std::string rutaTmp = ruta + ".tmp";
        std::ofstream archivo(rutaTmp, std::ios::out | std::ios::trunc);
        if (!archivo.is_open()) {
            std::cerr << "[ERROR] [AdminCSV] No se pudo crear temporal: " << rutaTmp << std::endl;
            return false;
        }
        archivo << "id_prestamo,codigo_recurso,documento_usuario,sede_id,fecha_prestamo,fecha_devolucion,activo\n";
        for (int i = 0; i < prestamos.getTamano(); ++i) {
            const RegistroPrestamoCSV& p = prestamos[i];
            archivo << p.idPrestamo << ','
                    << Validador::sanitizarCampoCSV(p.codigoRecurso) << ','
                    << Validador::sanitizarCampoCSV(p.documentoUsuario) << ','
                    << p.sedeId << ','
                    << Validador::sanitizarCampoCSV(p.fechaPrestamo) << ','
                    << Validador::sanitizarCampoCSV(p.fechaDevolucion) << ','
                    << (p.activo ? "1" : "0") << '\n';
        }
        archivo.flush();
        bool ok = archivo.good();
        archivo.close();
        if (!ok) {
            std::cerr << "[ERROR] [AdminCSV] Fallo de escritura en: " << rutaTmp << std::endl;
            std::remove(rutaTmp.c_str());
            return false;
        }
        return reemplazarArchivoAtomico(rutaTmp, ruta);
    }

    static bool cargarEspera(
        const std::string& ruta,
        VectorDinamico<RegistroEsperaCSV>& salida,
        int& filasOmitidas
    ) {
        filasOmitidas = 0;
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) return false;
        std::string linea;
        if (!std::getline(archivo, linea)) return false;

        int numLinea = 1;
        while (std::getline(archivo, linea)) {
            numLinea++;
            if (Validador::normalizarTexto(linea).empty()) continue;
            try {
                VectorDinamico<std::string> cols = Validador::dividirCSV(linea);
                if (cols.getTamano() != 2 ||
                    Validador::esNuloOProhibido(cols[0]) ||
                    Validador::esNuloOProhibido(cols[1])) {
                    throw std::invalid_argument("Par (recurso, usuario) inválido");
                }
                salida.push_back(RegistroEsperaCSV(
                    Validador::sanitizarCampoCSV(cols[0]),
                    Validador::sanitizarCampoCSV(cols[1])
                ));
            } catch (const std::exception& e) {
                filasOmitidas++;
                std::cout << "[WARN] [AdminCSV] Línea " << numLinea
                          << " corrupta en espera.csv omitida: " << e.what() << std::endl;
            }
        }
        return true;
    }

    // =========================================================================
    // HISTORIAL CSV (RQF 029 - Historial completo entre reinicios)
    // =========================================================================

    static bool cargarHistorial(
        const std::string& ruta,
        VectorDinamico<RegistroHistorialCSV>& salida,
        int& filasOmitidas
    ) {
        filasOmitidas = 0;
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) return false;
        std::string linea;
        if (!std::getline(archivo, linea)) return false;

        int numLinea = 1;
        while (std::getline(archivo, linea)) {
            numLinea++;
            if (Validador::normalizarTexto(linea).empty()) continue;
            try {
                VectorDinamico<std::string> cols = Validador::dividirCSV(linea);
                if (cols.getTamano() != 5) {
                    throw std::invalid_argument("Se esperaban 5 columnas");
                }
                RegistroHistorialCSV reg;
                reg.fechaHora = Validador::sanitizarCampoCSV(cols[0]);
                reg.tipoEvento = Validador::sanitizarCampoCSV(cols[1]);
                reg.codigoRecurso = Validador::sanitizarCampoCSV(cols[2]);
                reg.documentoUsuario = Validador::sanitizarCampoCSV(cols[3]);
                reg.detalle = Validador::sanitizarCampoCSV(cols[4]);

                if (Validador::esNuloOProhibido(reg.fechaHora) ||
                    Validador::esNuloOProhibido(reg.tipoEvento)) {
                    throw std::invalid_argument("Campos obligatorios vacíos");
                }
                salida.push_back(reg);
            } catch (const std::exception& e) {
                filasOmitidas++;
                std::cout << "[WARN] [AdminCSV] Línea " << numLinea
                          << " corrupta en historial.csv omitida: " << e.what() << std::endl;
            }
        }
        return true;
    }

    static bool guardarHistorial(const std::string& ruta, const VectorDinamico<RegistroHistorial>& historial) {
        std::string rutaTmp = ruta + ".tmp";
        std::ofstream archivo(rutaTmp, std::ios::out | std::ios::trunc);
        if (!archivo.is_open()) {
            std::cerr << "[ERROR] [AdminCSV] No se pudo crear temporal: " << rutaTmp << std::endl;
            return false;
        }
        archivo << "fecha_hora,tipo_evento,codigo_recurso,documento_usuario,detalle\n";
        for (int i = 0; i < historial.getTamano(); ++i) {
            const RegistroHistorial& h = historial[i];
            archivo << Validador::sanitizarCampoCSV(h.fechaHora) << ','
                    << Validador::sanitizarCampoCSV(h.tipoEvento) << ','
                    << Validador::sanitizarCampoCSV(h.codigoRecurso) << ','
                    << Validador::sanitizarCampoCSV(h.documentoUsuario) << ','
                    << Validador::sanitizarCampoCSV(h.detalle) << '\n';
        }
        archivo.flush();
        bool ok = archivo.good();
        archivo.close();
        if (!ok) {
            std::cerr << "[ERROR] [AdminCSV] Fallo de escritura en: " << rutaTmp << std::endl;
            std::remove(rutaTmp.c_str());
            return false;
        }
        return reemplazarArchivoAtomico(rutaTmp, ruta);
    }

    // =========================================================================
    // TURNOS CSV (RQF 029 - Matriz de disponibilidad semanal)
    // =========================================================================

    static bool cargarTurnos(
        const std::string& ruta,
        VectorDinamico<RegistroTurnoCSV>& salida,
        int numSedesValidas,
        int& filasOmitidas
    ) {
        filasOmitidas = 0;
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) return false;
        std::string linea;
        if (!std::getline(archivo, linea)) return false;

        int numLinea = 1;
        while (std::getline(archivo, linea)) {
            numLinea++;
            if (Validador::normalizarTexto(linea).empty()) continue;
            try {
                VectorDinamico<std::string> cols = Validador::dividirCSV(linea);
                if (cols.getTamano() != 4) {
                    throw std::invalid_argument("Se esperaban 4 columnas");
                }
                RegistroTurnoCSV reg;
                reg.codigoRecurso = Validador::sanitizarCampoCSV(cols[0]);
                int maxSede = (numSedesValidas > 0) ? (numSedesValidas - 1) : 0;
                if (!Validador::parsearEnteroSeguro(cols[1], reg.sedeId, 0, maxSede) ||
                    !Validador::parsearEnteroSeguro(cols[2], reg.dia, 0, MatrizDisponibilidad::DIAS_SEMANA - 1) ||
                    !Validador::parsearBooleanoSeguro(cols[3], reg.ocupado)) {
                    throw std::invalid_argument("Campos numéricos/booleanos corruptos");
                }
                if (Validador::esNuloOProhibido(reg.codigoRecurso)) {
                    throw std::invalid_argument("Código de recurso vacío");
                }
                salida.push_back(reg);
            } catch (const std::exception& e) {
                filasOmitidas++;
                std::cout << "[WARN] [AdminCSV] Línea " << numLinea
                          << " corrupta en turnos.csv omitida: " << e.what() << std::endl;
            }
        }
        return true;
    }

    static bool guardarTurnos(const std::string& ruta, const VectorDinamico<Recurso*>& recursos, const MatrizDisponibilidad& matriz) {
        std::string rutaTmp = ruta + ".tmp";
        std::ofstream archivo(rutaTmp, std::ios::out | std::ios::trunc);
        if (!archivo.is_open()) {
            std::cerr << "[ERROR] [AdminCSV] No se pudo crear temporal: " << rutaTmp << std::endl;
            return false;
        }
        archivo << "codigo_recurso,sede_id,dia,ocupado\n";
        for (int i = 0; i < recursos.getTamano(); ++i) {
            const Recurso* r = recursos[i];
            if (r == nullptr) continue;
            std::string codigo = r->getCodigo();
            if (!matriz.existeRecurso(codigo)) continue;
            int recIdx = matriz.buscarIndiceRecurso(codigo);
            if (recIdx == -1) continue;
            for (int s = 0; s < matriz.getTotalSedes(); ++s) {
                for (int d = 0; d < MatrizDisponibilidad::DIAS_SEMANA; ++d) {
                    if (!matriz.estaDisponible(codigo, s, d)) {
                        archivo << Validador::sanitizarCampoCSV(codigo) << ','
                                << s << ','
                                << d << ",1\n";
                    }
                }
            }
        }
        archivo.flush();
        bool ok = archivo.good();
        archivo.close();
        if (!ok) {
            std::cerr << "[ERROR] [AdminCSV] Fallo de escritura en: " << rutaTmp << std::endl;
            std::remove(rutaTmp.c_str());
            return false;
        }
        return reemplazarArchivoAtomico(rutaTmp, ruta);
    }

    // =========================================================================
    // CATEGORÍAS CSV (RQF 012 - Subcategorías personalizadas)
    // =========================================================================

    static bool cargarCategorias(
        const std::string& ruta,
        VectorDinamico<RegistroCategoriaCSV>& salida,
        int& filasOmitidas
    ) {
        filasOmitidas = 0;
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) return false;
        std::string linea;
        if (!std::getline(archivo, linea)) return false;

        int numLinea = 1;
        while (std::getline(archivo, linea)) {
            numLinea++;
            if (Validador::normalizarTexto(linea).empty()) continue;
            try {
                VectorDinamico<std::string> cols = Validador::dividirCSV(linea);
                if (cols.getTamano() != 2) {
                    throw std::invalid_argument("Se esperaban 2 columnas");
                }
                RegistroCategoriaCSV reg;
                reg.nombreCategoria = Validador::sanitizarCampoCSV(cols[0]);
                reg.nombrePadre = Validador::sanitizarCampoCSV(cols[1]);
                if (Validador::esNuloOProhibido(reg.nombreCategoria) ||
                    Validador::esNuloOProhibido(reg.nombrePadre)) {
                    throw std::invalid_argument("Nombres de categoría vacíos");
                }
                salida.push_back(reg);
            } catch (const std::exception& e) {
                filasOmitidas++;
                std::cout << "[WARN] [AdminCSV] Línea " << numLinea
                          << " corrupta en categorias.csv omitida: " << e.what() << std::endl;
            }
        }
        return true;
    }

    static bool guardarCategorias(const std::string& ruta, const ArbolCategorias<Recurso*>& arbol) {
        std::string rutaTmp = ruta + ".tmp";
        std::ofstream archivo(rutaTmp, std::ios::out | std::ios::trunc);
        if (!archivo.is_open()) {
            std::cerr << "[ERROR] [AdminCSV] No se pudo crear temporal: " << rutaTmp << std::endl;
            return false;
        }
        archivo << "nombre_categoria,nombre_padre\n";
        arbol.recolectarParesPadreHijo([&](const std::string& nombreCategoria, const std::string& nombrePadre) {
            archivo << Validador::sanitizarCampoCSV(nombreCategoria) << ','
                    << Validador::sanitizarCampoCSV(nombrePadre) << '\n';
        });
        archivo.flush();
        bool ok = archivo.good();
        archivo.close();
        if (!ok) {
            std::cerr << "[ERROR] [AdminCSV] Fallo de escritura en: " << rutaTmp << std::endl;
            std::remove(rutaTmp.c_str());
            return false;
        }
        return reemplazarArchivoAtomico(rutaTmp, ruta);
    }

    static bool guardarEspera(const std::string& ruta, const VectorDinamico<Recurso*>& recursos) {
        std::string rutaTmp = ruta + ".tmp";
        std::ofstream archivo(rutaTmp, std::ios::out | std::ios::trunc);
        if (!archivo.is_open()) {
            std::cerr << "[ERROR] [AdminCSV] No se pudo crear temporal: " << rutaTmp << std::endl;
            return false;
        }
        archivo << "codigo_recurso,documento_usuario\n";
        for (int i = 0; i < recursos.getTamano(); ++i) {
            const Recurso* r = recursos[i];
            if (r == nullptr || !r->tieneColaEspera()) continue;
            VectorDinamico<std::string> cola = r->obtenerColaEsperaOrdenada();
            for (int j = 0; j < cola.getTamano(); ++j) {
                archivo << Validador::sanitizarCampoCSV(r->getCodigo()) << ','
                        << Validador::sanitizarCampoCSV(cola[j]) << '\n';
            }
        }
        archivo.flush();
        bool ok = archivo.good();
        archivo.close();
        if (!ok) {
            std::cerr << "[ERROR] [AdminCSV] Fallo de escritura en: " << rutaTmp << std::endl;
            std::remove(rutaTmp.c_str());
            return false;
        }
        return reemplazarArchivoAtomico(rutaTmp, ruta);
    }
};

#endif // ADMIN_CSV_H
