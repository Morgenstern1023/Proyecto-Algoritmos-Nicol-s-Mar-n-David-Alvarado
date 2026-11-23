/**
 * @file GestorCRAI.h
 * @brief Fachada central del sistema CRAI (28/29 RQF, 10 estructuras propias, cooldown + CLI unlock, logs completos).
 * @author Oscar David Alvarado Navarrete (DA) & Nicolás Marín Quintero (NM)
 */

#ifndef GESTOR_CRAI_H
#define GESTOR_CRAI_H

#include "../Dominio/Recurso.h"
#include "../Dominio/Usuario.h"
#include "../Estructuras/ArbolAVL.h"
#include "../Estructuras/ArbolCategorias.h"
#include "../Estructuras/Cola.h"
#include "../Estructuras/GrafoSedes.h"
#include "../Estructuras/ListaDoble.h"
#include "../Estructuras/ListaSimple.h"
#include "../Estructuras/MatrizDisponibilidad.h"
#include "../Estructuras/Pila.h"
#include "../Estructuras/TablaHash.h"
#include "../Estructuras/VectorDinamico.h"
#include "../Seguridad/CryptoHash.h"
#include "../Seguridad/Validador.h"
#include "AdminCSV.h"
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

struct PrestamoActivo {
    int idPrestamo;
    std::string codigoRecurso;
    std::string documentoUsuario;
    int sedeId;
    std::string fechaPrestamo;

    PrestamoActivo(
        int id = 0, const std::string& cod = "", const std::string& doc = "",
        int sede = 0, const std::string& fecha = ""
    ) : idPrestamo(id), codigoRecurso(cod), documentoUsuario(doc),
        sedeId(sede), fechaPrestamo(fecha) {}

    bool operator==(const PrestamoActivo& otro) const {
        return idPrestamo == otro.idPrestamo;
    }

    std::string aJSON() const {
        std::ostringstream oss;
        oss << "{\"idPrestamo\":" << idPrestamo
            << ",\"codigoRecurso\":\"" << Validador::escaparJSON(codigoRecurso)
            << "\",\"documentoUsuario\":\"" << Validador::escaparJSON(documentoUsuario)
            << "\",\"sedeId\":" << sedeId
            << ",\"fechaPrestamo\":\"" << Validador::escaparJSON(fechaPrestamo) << "\"}";
        return oss.str();
    }
};

struct ResultadoOperacion {
    bool exito;
    int codigoHttp;
    std::string mensaje;
    std::string dataJSON;

    ResultadoOperacion(bool ok = false, int http = 400, const std::string& msg = "", const std::string& json = "{}")
        : exito(ok), codigoHttp(http), mensaje(msg), dataJSON(json) {}

    std::string aJSON() const {
        std::ostringstream oss;
        oss << "{\"exito\":" << (exito ? "true" : "false")
            << ",\"mensaje\":\"" << Validador::escaparJSON(mensaje)
            << "\",\"data\":" << (dataJSON.empty() ? "{}" : dataJSON) << "}";
        return oss.str();
    }
};

class GestorCRAI {
private:
    static const int MAX_PRESTAMOS_SIMULTANEOS = 3;

    struct SesionAdmin {
        std::string documentoAdmin;
        std::string nombreCompleto;
        std::string fechaInicio;

        SesionAdmin(const std::string& doc = "", const std::string& nom = "", const std::string& fecha = "")
            : documentoAdmin(doc), nombreCompleto(nom), fechaInicio(fecha) {}
    };

    enum TipoUndo {
        UNDO_NINGUNO = 0,
        UNDO_PRESTAMO,
        UNDO_DEVOLUCION,
        UNDO_TURNO_MATRIZ,
        UNDO_ALTA_RECURSO,
        UNDO_BAJA_RECURSO,
        UNDO_ACTUALIZAR_RECURSO,
        UNDO_ALTA_USUARIO,
        UNDO_BAJA_USUARIO,
        UNDO_ACTUALIZAR_USUARIO
    };

    struct OperacionUndo {
        TipoUndo tipo;
        std::string descripcion;
        int idPrestamo;
        std::string codigoRecurso;
        std::string documentoUsuario;
        std::string usuarioDesencoladoEspera;
        int sedeId;
        std::string fechaOriginal;
        RegistroRecursoCSV estadoPrevioRecurso;
        RegistroUsuarioCSV estadoPrevioUsuario;
        MatrizDisponibilidad::SnapshotEstado snapshotMatriz;

        OperacionUndo()
            : tipo(UNDO_NINGUNO), descripcion(""), idPrestamo(0),
              codigoRecurso(""), documentoUsuario(""), usuarioDesencoladoEspera(""),
              sedeId(0), fechaOriginal(""), estadoPrevioRecurso(),
              estadoPrevioUsuario(), snapshotMatriz() {}
    };

    std::string rutaInventarioCSV;
    std::string rutaUsuariosCSV;
    std::string rutaPrestamosCSV;
    std::string rutaEsperaCSV;
    std::string rutaHistorialCSV;
    std::string rutaTurnosCSV;
    std::string rutaCategoriasCSV;
    long long cooldownSegundos;

    VectorDinamico<Recurso*> catalogoRecursos;
    VectorDinamico<Usuario*> padronUsuarios;
    TablaHash<Recurso*> hashRecursos;
    TablaHash<Usuario*> hashUsuarios;
    TablaHash<SesionAdmin> sesionesActivas;
    ArbolAVL<std::string, Recurso*> avlRecursosPorNombre;
    ArbolCategorias<Recurso*> arbolCategorias;
    ListaSimple<PrestamoActivo> prestamosActivos;
    VectorDinamico<RegistroPrestamoCSV> registroCompletoPrestamos;
    ListaDoble<RegistroHistorial> historialGeneral;
    Pila<OperacionUndo> pilaUndo;
    MatrizDisponibilidad matrizTurnos;
    GrafoSedes grafoSedes;
    int siguienteIdPrestamo;

    static ResultadoOperacion logYRetornar(const std::string& tag, const ResultadoOperacion& r) {
        if (tag == "ERROR") {
            std::cerr << "[ERROR] [GestorCRAI] (" << r.codigoHttp << ") " << r.mensaje << std::endl;
        } else {
            std::cout << "[" << tag << "] [GestorCRAI] (" << r.codigoHttp << ") " << r.mensaje << std::endl;
        }
        return r;
    }

    static std::string claveAVL(const std::string& nombre, const std::string& codigo) {
        return Validador::aMinusculas(nombre) + "|" + codigo;
    }

    static Recurso* construirRecursoDesdeDTO(const RegistroRecursoCSV& dto) {
        std::string cat = Validador::aMinusculas(dto.categoria);
        if (cat == "libro") {
            return new Libro(dto.codigo, dto.nombre, dto.subcategoria, dto.estado,
                             dto.disponible, dto.sedeId, dto.anioAdquisicion,
                             dto.vecesPrestado, dto.detalleExtra);
        }
        if (cat == "computador") {
            return new Computador(dto.codigo, dto.nombre, dto.subcategoria, dto.estado,
                                  dto.disponible, dto.sedeId, dto.anioAdquisicion,
                                  dto.vecesPrestado, dto.detalleExtra);
        }
        if (cat == "sala") {
            int cap = 6;
            Validador::parsearEnteroSeguro(dto.detalleExtra, cap, 1, 200);
            return new Sala(dto.codigo, dto.nombre, dto.subcategoria, dto.estado,
                            dto.disponible, dto.sedeId, dto.anioAdquisicion,
                            dto.vecesPrestado, cap);
        }
        return nullptr;
    }

    static RegistroRecursoCSV exportarRecursoADTO(const Recurso* r) {
        RegistroRecursoCSV dto;
        if (r == nullptr) return dto;
        dto.categoria = r->getCategoria();
        dto.codigo = r->getCodigo();
        dto.nombre = r->getNombre();
        dto.estado = r->getEstado();
        dto.disponible = r->isDisponible();
        dto.sedeId = r->getSedeId();
        dto.anioAdquisicion = r->getAnioAdquisicion();
        dto.vecesPrestado = r->getVecesPrestado();
        dto.subcategoria = r->getSubcategoria();
        dto.detalleExtra = r->getDetalleExtra();
        return dto;
    }

    static RegistroUsuarioCSV exportarUsuarioADTO(const Usuario* u) {
        RegistroUsuarioCSV dto;
        if (u == nullptr) return dto;
        dto.documento = u->getDocumento();
        dto.nombre = u->getNombre();
        dto.apellido = u->getApellido();
        dto.sancionado = u->isSancionado();
        const Administrador* admin = dynamic_cast<const Administrador*>(u);
        if (admin != nullptr) {
            dto.esAdmin = true;
            dto.credencialHash = admin->getCredencialHash();
            dto.intentosFallidos = admin->getIntentosFallidos();
            dto.bloqueadoHasta = admin->getBloqueadoHasta();
        }
        return dto;
    }

    void inicializarJerarquiaCategoriasBase() {
        arbolCategorias.insertarSubcategoria("CRAI", "Libros");
        arbolCategorias.insertarSubcategoria("CRAI", "Tecnologia");
        arbolCategorias.insertarSubcategoria("CRAI", "Espacios");
        arbolCategorias.insertarSubcategoria("Libros", "Ingenieria");
        arbolCategorias.insertarSubcategoria("Libros", "Ciencias Basicas");
        arbolCategorias.insertarSubcategoria("Libros", "General");
        arbolCategorias.insertarSubcategoria("Tecnologia", "Equipos de Computo");
        arbolCategorias.insertarSubcategoria("Espacios", "Espacios Colaborativos");
    }

    void vincularRecursoEnEstructuras(Recurso* rec) {
        if (rec == nullptr) return;
        catalogoRecursos.push_back(rec);
        hashRecursos.insertar(rec->getCodigo(), rec);
        avlRecursosPorNombre.insertar(claveAVL(rec->getNombre(), rec->getCodigo()), rec);

        std::string padreBase = "Libros";
        if (rec->getCategoria() == "computador") padreBase = "Tecnologia";
        else if (rec->getCategoria() == "sala") padreBase = "Espacios";

        if (!arbolCategorias.existeCategoria(rec->getSubcategoria())) {
            arbolCategorias.insertarSubcategoria(padreBase, rec->getSubcategoria());
        }
        arbolCategorias.asociarElemento(rec->getSubcategoria(), rec);

        matrizTurnos.inicializarRecurso(rec->getCodigo());
        if (!rec->isDisponible()) {
            matrizTurnos.establecerDisponibilidadSede(rec->getCodigo(), rec->getSedeId(), false);
        }
    }

    bool removerRecursoDeEstructuras(const std::string& codigo, bool destruirObjeto) {
        Recurso* rec = nullptr;
        if (!hashRecursos.buscar(codigo, rec) || rec == nullptr) return false;
        hashRecursos.eliminar(codigo);
        avlRecursosPorNombre.eliminar(claveAVL(rec->getNombre(), rec->getCodigo()));
        arbolCategorias.desasociarElemento(rec->getSubcategoria(), rec);
        for (int i = 0; i < catalogoRecursos.getTamano(); ++i) {
            if (catalogoRecursos[i] == rec) {
                catalogoRecursos.eliminarEn(i);
                break;
            }
        }
        if (destruirObjeto) delete rec;
        return true;
    }

    int contarPrestamosActivosUsuario(const std::string& documento) const {
        int cuenta = 0;
        const NodoSimple<PrestamoActivo>* actual = prestamosActivos.getCabeza();
        while (actual != nullptr) {
            if (actual->dato.documentoUsuario == documento) cuenta++;
            actual = actual->siguiente;
        }
        return cuenta;
    }

    static void bucketSortPorMasPrestados(VectorDinamico<Recurso*>& vec) {
        int n = vec.getTamano();
        if (n <= 1) return;
        int maxPrestamos = 0;
        for (int i = 0; i < n; ++i) {
            if (vec[i]->getVecesPrestado() > maxPrestamos) maxPrestamos = vec[i]->getVecesPrestado();
        }
        if (maxPrestamos > 10000) {
            vec.mergeSort([](const Recurso* a, const Recurso* b) {
                return a->getVecesPrestado() > b->getVecesPrestado();
            });
            return;
        }
        int numCubetas = maxPrestamos + 1;
        ListaSimple<Recurso*>* cubetas = new ListaSimple<Recurso*>[numCubetas];
        try {
            for (int i = 0; i < n; ++i) cubetas[vec[i]->getVecesPrestado()].insertarFinal(vec[i]);
            int idx = 0;
            for (int b = maxPrestamos; b >= 0; --b) {
                const NodoSimple<Recurso*>* nodo = cubetas[b].getCabeza();
                while (nodo != nullptr) {
                    vec[idx++] = nodo->dato;
                    nodo = nodo->siguiente;
                }
            }
        } catch (...) {
            delete[] cubetas;
            throw;
        }
        delete[] cubetas;
    }

    void insertarRegistroHistorialEnMemoria(const RegistroHistorial& reg) {
        historialGeneral.insertarFinal(reg);
        Recurso* r = nullptr;
        if (!reg.codigoRecurso.empty() && hashRecursos.buscar(reg.codigoRecurso, r) && r != nullptr) {
            r->agregarAlHistorial(reg);
        }
        Usuario* u = nullptr;
        if (!reg.documentoUsuario.empty() && hashUsuarios.buscar(reg.documentoUsuario, u) && u != nullptr) {
            u->agregarAlHistorial(reg);
        }
    }

    void registrarEventoHistorial(
        const std::string& tipo, const std::string& codRec,
        const std::string& docUsu, const std::string& detalle
    ) {
        RegistroHistorial reg(Validador::obtenerTimestampActual(), tipo, codRec, docUsu, detalle);
        insertarRegistroHistorialEnMemoria(reg);
        AdminCSV::guardarHistorial(rutaHistorialCSV, historialGeneral.obtenerAdelante());
    }

    void persistirTodo() {
        AdminCSV::guardarInventario(rutaInventarioCSV, catalogoRecursos);
        AdminCSV::guardarUsuarios(rutaUsuariosCSV, padronUsuarios);
        AdminCSV::guardarPrestamos(rutaPrestamosCSV, registroCompletoPrestamos);
        AdminCSV::guardarEspera(rutaEsperaCSV, catalogoRecursos);
        AdminCSV::guardarHistorial(rutaHistorialCSV, historialGeneral.obtenerAdelante());
        AdminCSV::guardarTurnos(rutaTurnosCSV, catalogoRecursos, matrizTurnos);
        AdminCSV::guardarCategorias(rutaCategoriasCSV, arbolCategorias);
    }

    void asegurarAdminPorDefecto() {
        for (int i = 0; i < padronUsuarios.getTamano(); ++i) {
            if (padronUsuarios[i] != nullptr && padronUsuarios[i]->esAdmin()) return;
        }
        std::string hashDefecto = CryptoHash::crearCredencial("CraiUR2026!");
        Administrador* adminDef = new Administrador("ADMIN01", "Recepcion", "CRAI", hashDefecto);
        padronUsuarios.push_back(adminDef);
        hashUsuarios.insertar(adminDef->getDocumento(), adminDef);
        AdminCSV::guardarUsuarios(rutaUsuariosCSV, padronUsuarios);
        std::cout << "[INFO] [GestorCRAI] Cuenta administrativa por defecto creada (doc: ADMIN01)." << std::endl;
    }

public:
    explicit GestorCRAI(
        const std::string& rutaInv = "dependencias/inventario.csv",
        const std::string& rutaUsu = "dependencias/usuarios.csv",
        const std::string& rutaPre = "dependencias/prestamos.csv",
        const std::string& rutaEsp = "dependencias/espera.csv",
        const std::string& rutaHis = "dependencias/historial.csv",
        const std::string& rutaTur = "dependencias/turnos.csv",
        const std::string& rutaCat = "dependencias/categorias.csv",
        long long duracionCooldownSeg = 60
    ) : rutaInventarioCSV(rutaInv), rutaUsuariosCSV(rutaUsu),
        rutaPrestamosCSV(rutaPre), rutaEsperaCSV(rutaEsp),
        rutaHistorialCSV(rutaHis), rutaTurnosCSV(rutaTur), rutaCategoriasCSV(rutaCat),
        cooldownSegundos(duracionCooldownSeg > 0 ? duracionCooldownSeg : 60),
        catalogoRecursos(64), padronUsuarios(64),
        hashRecursos(1009), hashUsuarios(1009), sesionesActivas(101),
        avlRecursosPorNombre(), arbolCategorias("CRAI"),
        prestamosActivos(), registroCompletoPrestamos(64),
        historialGeneral(), pilaUndo(),
        matrizTurnos(64, 6), grafoSedes(8), siguienteIdPrestamo(1) {}

    // Constructor legacy (5 args): deriva rutas auxiliares junto a rutaInv para aislar pruebas
    explicit GestorCRAI(
        const std::string& rutaInv,
        const std::string& rutaUsu,
        const std::string& rutaPre,
        const std::string& rutaEsp,
        long long duracionCooldownSeg
    ) : GestorCRAI(rutaInv, rutaUsu, rutaPre, rutaEsp,
                    rutaInv + ".historial.csv", rutaInv + ".turnos.csv",
                    rutaInv + ".categorias.csv", duracionCooldownSeg) {}

    GestorCRAI(const GestorCRAI&) = delete;
    GestorCRAI& operator=(const GestorCRAI&) = delete;

    ~GestorCRAI() {
        for (int i = 0; i < catalogoRecursos.getTamano(); ++i) {
            delete catalogoRecursos[i];
            catalogoRecursos[i] = nullptr;
        }
        for (int i = 0; i < padronUsuarios.getTamano(); ++i) {
            delete padronUsuarios[i];
            padronUsuarios[i] = nullptr;
        }
    }

    bool inicializar() {
        std::cout << "[INFO] [GestorCRAI] Inicializando motor CRAI offline..." << std::endl;
        grafoSedes.precargarSedesCRAI(matrizTurnos);
        inicializarJerarquiaCategoriasBase();

        // 1. Cargar inventario (inventario.csv)
        VectorDinamico<RegistroRecursoCSV> dtosInv(512);
        int omitInv = 0;
        if (AdminCSV::cargarInventario(rutaInventarioCSV, dtosInv, grafoSedes.getNumSedes(), omitInv)) {
            int cargados = 0;
            for (int i = 0; i < dtosInv.getTamano(); ++i) {
                if (hashRecursos.existe(dtosInv[i].codigo)) {
                    omitInv++;
                    continue;
                }
                Recurso* rec = construirRecursoDesdeDTO(dtosInv[i]);
                if (rec != nullptr) {
                    vincularRecursoEnEstructuras(rec);
                    cargados++;
                }
            }
            std::cout << "[INFO] [GestorCRAI] Inventario cargado: " << cargados
                      << " recursos (" << omitInv << " omitidos). Colisiones Hash: "
                      << hashRecursos.getTotalColisiones() << ", Altura AVL: "
                      << avlRecursosPorNombre.getAltura() << std::endl;
        }

        // 2. Cargar usuarios (usuarios.csv) y asegurar ADMIN01
        VectorDinamico<RegistroUsuarioCSV> dtosUsu(128);
        int omitUsu = 0;
        if (AdminCSV::cargarUsuarios(rutaUsuariosCSV, dtosUsu, omitUsu)) {
            for (int i = 0; i < dtosUsu.getTamano(); ++i) {
                const RegistroUsuarioCSV& u = dtosUsu[i];
                if (hashUsuarios.existe(u.documento)) continue;
                Usuario* obj = u.esAdmin
                    ? static_cast<Usuario*>(new Administrador(u.documento, u.nombre, u.apellido,
                                                              u.credencialHash, u.intentosFallidos, u.bloqueadoHasta))
                    : new Usuario(u.documento, u.nombre, u.apellido, u.sancionado);
                padronUsuarios.push_back(obj);
                hashUsuarios.insertar(obj->getDocumento(), obj);
            }
            std::cout << "[INFO] [GestorCRAI] Usuarios cargados: " << padronUsuarios.getTamano()
                      << " (" << omitUsu << " omitidos)." << std::endl;
        }
        asegurarAdminPorDefecto();

        // 3. Cargar préstamos (prestamos.csv) ANTES de auditar estados huérfanos
        VectorDinamico<RegistroPrestamoCSV> dtosPre(64);
        int omitPre = 0;
        bool huboPrestamosCSV = AdminCSV::cargarPrestamos(rutaPrestamosCSV, dtosPre, grafoSedes.getNumSedes(), omitPre);
        if (huboPrestamosCSV) {
            for (int i = 0; i < dtosPre.getTamano(); ++i) {
                const RegistroPrestamoCSV& p = dtosPre[i];
                Recurso* r = nullptr;
                if (!hashRecursos.buscar(p.codigoRecurso, r) || r == nullptr || !hashUsuarios.existe(p.documentoUsuario)) {
                    std::cout << "[WARN] [GestorCRAI] Préstamo huérfano omitido ID " << p.idPrestamo << std::endl;
                    omitPre++;
                    continue;
                }
                registroCompletoPrestamos.push_back(p);
                if (p.idPrestamo >= siguienteIdPrestamo) siguienteIdPrestamo = p.idPrestamo + 1;
                if (p.activo) {
                    prestamosActivos.insertarFinal(PrestamoActivo(
                        p.idPrestamo, p.codigoRecurso, p.documentoUsuario, p.sedeId, p.fechaPrestamo
                    ));
                    // Garantizar invariante: todo préstamo activo bloquea su recurso y la matriz
                    r->setEstado("prestado");
                    r->setDisponible(false);
                    matrizTurnos.establecerDisponibilidadSede(r->getCodigo(), r->getSedeId(), false);
                }
            }
            std::cout << "[INFO] [GestorCRAI] Préstamos cargados: " << registroCompletoPrestamos.getTamano()
                      << " (" << prestamosActivos.getTamano() << " activos, " << omitPre << " omitidos)." << std::endl;
        }

        // 4. Verificación de integridad referencial al arranque (después de cargar prestamos.csv)
        int huerfanosCorregidos = 0;
        for (int i = 0; i < catalogoRecursos.getTamano(); ++i) {
            Recurso* rec = catalogoRecursos[i];
            if (rec != nullptr && rec->getEstado() == "prestado") {
                bool tienePrestamoActivo = false;
                const NodoSimple<PrestamoActivo>* actual = prestamosActivos.getCabeza();
                while (actual != nullptr) {
                    if (actual->dato.codigoRecurso == rec->getCodigo()) {
                        tienePrestamoActivo = true;
                        break;
                    }
                    actual = actual->siguiente;
                }
                if (!tienePrestamoActivo) {
                    std::cout << "[WARN] [Integridad] Recurso " << rec->getCodigo()
                              << " en estado 'prestado' sin préstamo activo; corregido a 'disponible'." << std::endl;
                    rec->setEstado("disponible");
                    rec->setDisponible(true);
                    matrizTurnos.establecerDisponibilidadSede(rec->getCodigo(), rec->getSedeId(), true);
                    huerfanosCorregidos++;
                }
            }
        }
        if (huerfanosCorregidos > 0) {
            std::cout << "[WARN] [Integridad] Total recursos huérfanos saneados: " << huerfanosCorregidos << std::endl;
        }
        if (catalogoRecursos.getTamano() > 0) {
            AdminCSV::guardarInventario(rutaInventarioCSV, catalogoRecursos);
        }

        // 5. Cargar lista de espera FIFO (espera.csv)
        VectorDinamico<RegistroEsperaCSV> dtosEsp(32);
        int omitEsp = 0;
        if (AdminCSV::cargarEspera(rutaEsperaCSV, dtosEsp, omitEsp)) {
            for (int i = 0; i < dtosEsp.getTamano(); ++i) {
                Recurso* r = nullptr;
                if (hashRecursos.buscar(dtosEsp[i].codigoRecurso, r) && r != nullptr &&
                    hashUsuarios.existe(dtosEsp[i].documentoUsuario)) {
                    r->encolarUsuarioEspera(dtosEsp[i].documentoUsuario);
                } else {
                    std::cout << "[WARN] [GestorCRAI] Registro de espera huérfano omitido." << std::endl;
                }
            }
        }

        // 6. Cargar historial completo (historial.csv) en orden cronológico y vincular a Recurso/Usuario
        VectorDinamico<RegistroHistorialCSV> dtosHis(256);
        int omitHis = 0;
        bool huboHistorialCSV = AdminCSV::cargarHistorial(rutaHistorialCSV, dtosHis, omitHis);
        if (huboHistorialCSV && dtosHis.getTamano() > 0) {
            for (int i = 0; i < dtosHis.getTamano(); ++i) {
                const RegistroHistorialCSV& h = dtosHis[i];
                RegistroHistorial reg(h.fechaHora, h.tipoEvento, h.codigoRecurso, h.documentoUsuario, h.detalle);
                insertarRegistroHistorialEnMemoria(reg);
            }
            std::cout << "[INFO] [GestorCRAI] Historial cargado: " << dtosHis.getTamano()
                      << " eventos (" << omitHis << " omitidos)." << std::endl;
        } else if (huboPrestamosCSV) {
            // Respaldo: reconstruir eventos desde prestamos.csv conservando sus fechas originales
            for (int i = 0; i < registroCompletoPrestamos.getTamano(); ++i) {
                const RegistroPrestamoCSV& p = registroCompletoPrestamos[i];
                insertarRegistroHistorialEnMemoria(RegistroHistorial(
                    p.fechaPrestamo, "PRESTAMO", p.codigoRecurso, p.documentoUsuario,
                    "Préstamo registrado (ID " + std::to_string(p.idPrestamo) + ")."
                ));
                if (!p.activo) {
                    std::string fDev = Validador::esNuloOProhibido(p.fechaDevolucion) ? p.fechaPrestamo : p.fechaDevolucion;
                    insertarRegistroHistorialEnMemoria(RegistroHistorial(
                        fDev, "DEVOLUCION", p.codigoRecurso, p.documentoUsuario,
                        "Devolución registrada (ID " + std::to_string(p.idPrestamo) + ")."
                    ));
                }
            }
        }

        // 7. Cargar turnos ocupados en matriz semanal (turnos.csv)
        VectorDinamico<RegistroTurnoCSV> dtosTur(256);
        int omitTur = 0;
        if (AdminCSV::cargarTurnos(rutaTurnosCSV, dtosTur, grafoSedes.getNumSedes(), omitTur)) {
            for (int i = 0; i < dtosTur.getTamano(); ++i) {
                const RegistroTurnoCSV& t = dtosTur[i];
                if (matrizTurnos.existeRecurso(t.codigoRecurso)) {
                    if (t.ocupado) matrizTurnos.reservarTurno(t.codigoRecurso, t.sedeId, t.dia);
                    else matrizTurnos.liberarTurno(t.codigoRecurso, t.sedeId, t.dia);
                }
            }
            std::cout << "[INFO] [GestorCRAI] Turnos cargados: " << dtosTur.getTamano()
                      << " registros (" << omitTur << " omitidos)." << std::endl;
        }

        // 8. Cargar subcategorías personalizadas (categorias.csv)
        VectorDinamico<RegistroCategoriaCSV> dtosCat(64);
        int omitCat = 0;
        if (AdminCSV::cargarCategorias(rutaCategoriasCSV, dtosCat, omitCat)) {
            for (int i = 0; i < dtosCat.getTamano(); ++i) {
                const RegistroCategoriaCSV& c = dtosCat[i];
                if (!arbolCategorias.existeCategoria(c.nombreCategoria)) {
                    arbolCategorias.insertarSubcategoria(c.nombrePadre, c.nombreCategoria);
                }
            }
            std::cout << "[INFO] [GestorCRAI] Categorías cargadas: " << dtosCat.getTamano()
                      << " relaciones (" << omitCat << " omitidos)." << std::endl;
        }

        // 9. Garantizar cabeceras persistidas en disco y registrar arranque
        if (!huboPrestamosCSV) AdminCSV::guardarPrestamos(rutaPrestamosCSV, registroCompletoPrestamos);
        AdminCSV::guardarEspera(rutaEsperaCSV, catalogoRecursos);
        AdminCSV::guardarCategorias(rutaCategoriasCSV, arbolCategorias);
        registrarEventoHistorial("SISTEMA", "", "", "Sistema CRAI inicializado correctamente.");
        return true;
    }

    // =========================================================================
    // AUTENTICACIÓN ADMINISTRATIVA CON COOLDOWN + DESBLOQUEO LOCAL POR TERMINAL
    // =========================================================================

    ResultadoOperacion iniciarSesionAdmin(const std::string& documento, const std::string& clavePlana) {
        std::string docLimpio = Validador::sanitizarCampoCSV(documento);
        if (Validador::esNuloOProhibido(docLimpio) || clavePlana.empty()) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Credenciales vacías o inválidas."));
        }

        Usuario* u = nullptr;
        if (!hashUsuarios.buscar(docLimpio, u) || u == nullptr || !u->esAdmin()) {
            return logYRetornar("SECURITY", ResultadoOperacion(false, 401, "Credenciales inválidas."));
        }

        Administrador* admin = dynamic_cast<Administrador*>(u);
        if (admin == nullptr) {
            return logYRetornar("ERROR", ResultadoOperacion(false, 500, "Error interno de rol administrativo."));
        }

        std::time_t ahora = std::time(nullptr);
        if (admin->estaEnCooldown(ahora)) {
            long long segs = admin->segundosRestantesCooldown(ahora);
            std::ostringstream data;
            data << "{\"segundosRestantes\":" << segs << "}";
            return logYRetornar("SECURITY", ResultadoOperacion(
                false, 423,
                "Cuenta en cooldown tras 3 intentos fallidos. Espera " + std::to_string(segs) + " segundos.",
                data.str()
            ));
        }

        if (!CryptoHash::verificarCredencial(clavePlana, admin->getCredencialHash())) {
            bool activoCooldown = admin->registrarIntentoFallido(ahora, cooldownSegundos);
            AdminCSV::guardarUsuarios(rutaUsuariosCSV, padronUsuarios);

            if (activoCooldown) {
                registrarEventoHistorial("SEGURIDAD", "", docLimpio,
                    "Cooldown activado (" + std::to_string(cooldownSegundos) + "s) por 3 fallos.");
                std::ostringstream data;
                data << "{\"segundosRestantes\":" << cooldownSegundos << "}";
                return logYRetornar("SECURITY", ResultadoOperacion(
                    false, 423,
                    "Demasiados intentos fallidos. Cuenta en cooldown por " +
                    std::to_string(cooldownSegundos) + " segundos.",
                    data.str()
                ));
            }
            int restantes = 3 - admin->getIntentosFallidos();
            return logYRetornar("SECURITY", ResultadoOperacion(
                false, 401,
                "Contraseña incorrecta. Intentos restantes antes de cooldown: " + std::to_string(restantes)
            ));
        }

        admin->reiniciarIntentos();
        AdminCSV::guardarUsuarios(rutaUsuariosCSV, padronUsuarios);

        std::string token = CryptoHash::generarAleatorioHex(32);
        std::string nombreCompleto = admin->getNombre() + " " + admin->getApellido();
        sesionesActivas.insertar(token, SesionAdmin(docLimpio, nombreCompleto, Validador::obtenerTimestampActual()));
        registrarEventoHistorial("LOGIN", "", docLimpio, "Inicio de sesión administrativo exitoso.");

        std::ostringstream oss;
        oss << "{\"token\":\"" << token << "\",\"admin\":" << admin->aJSON() << "}";
        return logYRetornar("INFO", ResultadoOperacion(true, 200, "Inicio de sesión exitoso.", oss.str()));
    }

    bool validarTokenSesion(const std::string& token) const {
        return !token.empty() && sesionesActivas.existe(token);
    }

    Administrador* obtenerAdminDeSesion(const std::string& token) {
        if (token.empty()) return nullptr;
        SesionAdmin sesion;
        if (!sesionesActivas.buscar(token, sesion)) return nullptr;
        Usuario* u = nullptr;
        if (!hashUsuarios.buscar(sesion.documentoAdmin, u) || u == nullptr || !u->esAdmin()) return nullptr;
        return dynamic_cast<Administrador*>(u);
    }

    ResultadoOperacion cerrarSesionAdmin(const std::string& token) {
        if (sesionesActivas.eliminar(token)) {
            return logYRetornar("INFO", ResultadoOperacion(true, 200, "Sesión cerrada correctamente."));
        }
        return logYRetornar("WARN", ResultadoOperacion(false, 401, "Sesión no encontrada."));
    }

    /**
     * @brief Opción 3: Desbloqueo de emergencia exclusivo desde la terminal local (`--unlock <DOC>`).
     * @note Nunca se expone como endpoint HTTP; requiere acceso físico/SSH al host.
     */
    ResultadoOperacion desbloquearAdminPorTerminal(const std::string& docAdmin) {
        std::string doc = Validador::sanitizarCampoCSV(docAdmin);
        if (Validador::esNuloOProhibido(doc)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Documento inválido para --unlock."));
        }
        Usuario* u = nullptr;
        if (!hashUsuarios.buscar(doc, u) || u == nullptr || !u->esAdmin()) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Administrador no encontrado: " + doc));
        }
        Administrador* admin = dynamic_cast<Administrador*>(u);
        admin->reiniciarIntentos();
        AdminCSV::guardarUsuarios(rutaUsuariosCSV, padronUsuarios);
        registrarEventoHistorial("SEGURIDAD", "", doc, "Cooldown removido localmente vía terminal (--unlock).");
        return logYRetornar("SECURITY", ResultadoOperacion(true, 200, "Cuenta desbloqueada localmente: " + doc, admin->aJSON()));
    }

    // =========================================================================
    // CRUD RECURSOS (RQF 001-003, 010, 019, 022)
    // =========================================================================

    ResultadoOperacion consultarRecursoPorCodigo(const std::string& codigo) const {
        std::string cod = Validador::sanitizarCampoCSV(codigo);
        if (Validador::esNuloOProhibido(cod)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Código de recurso inválido."));
        }
        Recurso* rec = nullptr;
        if (!hashRecursos.buscar(cod, rec) || rec == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado: " + cod));
        }
        return ResultadoOperacion(true, 200, "Recurso localizado en O(1).", rec->aJSON());
    }

    ResultadoOperacion registrarRecurso(const RegistroRecursoCSV& entrada, const std::string& tokenAdmin = "") {
        RegistroRecursoCSV dto = entrada;
        dto.categoria = Validador::aMinusculas(Validador::sanitizarCampoCSV(dto.categoria));
        dto.codigo = Validador::sanitizarCampoCSV(dto.codigo);
        dto.nombre = Validador::sanitizarCampoCSV(dto.nombre);
        dto.estado = Validador::aMinusculas(Validador::sanitizarOpcional(dto.estado, "disponible"));
        dto.subcategoria = Validador::sanitizarOpcional(dto.subcategoria, "General");
        dto.detalleExtra = Validador::sanitizarOpcional(dto.detalleExtra, "Estandar");

        if (Validador::esNuloOProhibido(dto.codigo) || Validador::esNuloOProhibido(dto.nombre) ||
            (dto.categoria != "libro" && dto.categoria != "computador" && dto.categoria != "sala")) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Datos obligatorios del recurso inválidos o nulos."));
        }
        if (dto.sedeId < 0 || dto.sedeId >= grafoSedes.getNumSedes()) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Sede asignada fuera de rango."));
        }
        if (hashRecursos.existe(dto.codigo)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "Ya existe un recurso con el código " + dto.codigo));
        }

        dto.disponible = (dto.estado == "disponible");
        Recurso* nuevo = construirRecursoDesdeDTO(dto);
        if (nuevo == nullptr) {
            return logYRetornar("ERROR", ResultadoOperacion(false, 500, "No se pudo instanciar el recurso."));
        }

        vincularRecursoEnEstructuras(nuevo);

        OperacionUndo cmd;
        cmd.tipo = UNDO_ALTA_RECURSO;
        cmd.codigoRecurso = dto.codigo;
        cmd.descripcion = "Alta de recurso " + dto.codigo;
        pilaUndo.push(cmd);

        registrarEventoHistorial("ALTA_RECURSO", dto.codigo, "", "Recurso registrado: " + dto.nombre);
        AdminCSV::guardarInventario(rutaInventarioCSV, catalogoRecursos);
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 201, "Recurso registrado correctamente.", nuevo->aJSON()));
    }

    ResultadoOperacion actualizarRecurso(
        const std::string& codigo, const std::string& nuevoNombre, const std::string& nuevoEstado,
        int nuevaSedeId, const std::string& nuevaSubcategoria, const std::string& nuevoDetalleExtra,
        const std::string& tokenAdmin = ""
    ) {
        std::string cod = Validador::sanitizarCampoCSV(codigo);
        Recurso* rec = nullptr;
        if (Validador::esNuloOProhibido(cod) || !hashRecursos.buscar(cod, rec) || rec == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado para actualizar: " + cod));
        }

        // P1 Fix #1: Validaciones de blindaje en actualizarRecurso()
        // 1. Bloquear cambio manual a estado "prestado" y proteger recursos actualmente prestados
        if (!Validador::esNuloOProhibido(nuevoEstado)) {
            std::string est = Validador::aMinusculas(Validador::sanitizarCampoCSV(nuevoEstado));
            if (est == "prestado") {
                return logYRetornar("WARN", ResultadoOperacion(false, 403,
                    "No se permite cambiar estado a 'prestado' manualmente. Use el endpoint de préstamos."));
            }
            if (rec->getEstado() == "prestado" && est != "prestado") {
                return logYRetornar("WARN", ResultadoOperacion(false, 409,
                    "No se puede alterar manualmente el estado de un recurso prestado. Registre su devolución."));
            }
        }

        // 2. Si se intenta cambiar sedeId, validar que no haya préstamo activo
        if (nuevaSedeId >= 0 && nuevaSedeId < grafoSedes.getNumSedes() && nuevaSedeId != rec->getSedeId()) {
            if (!rec->isDisponible() || rec->getEstado() == "prestado") {
                return logYRetornar("WARN", ResultadoOperacion(false, 409,
                    "No se puede cambiar la sede de un recurso prestado o no disponible."));
            }
            // Verificar préstamo activo en prestamosActivos (estar en la lista = activo)
            NodoSimple<PrestamoActivo>* actual = prestamosActivos.getCabeza();
            while (actual != nullptr) {
                if (actual->dato.codigoRecurso == cod) {
                    return logYRetornar("WARN", ResultadoOperacion(false, 409,
                        "No se puede cambiar la sede: el recurso tiene un préstamo activo."));
                }
                actual = actual->siguiente;
            }
        }

        OperacionUndo cmd;
        cmd.tipo = UNDO_ACTUALIZAR_RECURSO;
        cmd.codigoRecurso = cod;
        cmd.estadoPrevioRecurso = exportarRecursoADTO(rec);
        cmd.descripcion = "Actualización de recurso " + cod;

        if (!Validador::esNuloOProhibido(nuevoNombre)) {
            avlRecursosPorNombre.eliminar(claveAVL(rec->getNombre(), rec->getCodigo()));
            rec->setNombre(Validador::sanitizarCampoCSV(nuevoNombre));
            avlRecursosPorNombre.insertar(claveAVL(rec->getNombre(), rec->getCodigo()), rec);
        }
        if (!Validador::esNuloOProhibido(nuevoEstado)) {
            std::string est = Validador::aMinusculas(Validador::sanitizarCampoCSV(nuevoEstado));
            if (est == "disponible" || est == "mantenimiento") {  // "prestado" ya bloqueado arriba
                rec->setEstado(est);
                rec->setDisponible(est == "disponible");
                matrizTurnos.establecerDisponibilidadSede(cod, rec->getSedeId(), rec->isDisponible());
            }
        }
        int sedeAnterior = rec->getSedeId();
        if (nuevaSedeId >= 0 && nuevaSedeId < grafoSedes.getNumSedes()) {
            rec->setSedeId(nuevaSedeId);
            // 3. Sincronizar MatrizDisponibilidad si cambió sedeId
            if (nuevaSedeId != sedeAnterior) {
                // Liberar disponibilidad en sede anterior
                matrizTurnos.establecerDisponibilidadSede(cod, sedeAnterior, false);
                // Establecer en nueva sede según disponibilidad actual
                matrizTurnos.establecerDisponibilidadSede(cod, nuevaSedeId, rec->isDisponible());
            }
        }
        if (!Validador::esNuloOProhibido(nuevaSubcategoria)) {
            std::string subLimpia = Validador::sanitizarCampoCSV(nuevaSubcategoria);
            arbolCategorias.desasociarElemento(rec->getSubcategoria(), rec);
            // 4. Colgar subcategoría nueva bajo rama base correcta según categoría del recurso
            std::string categoriaBase = "";
            if (rec->getCategoria() == "libro") categoriaBase = "Libros";
            else if (rec->getCategoria() == "computador") categoriaBase = "Tecnologia";
            else if (rec->getCategoria() == "sala") categoriaBase = "Espacios";
            else categoriaBase = "CRAI";  // fallback
            if (!arbolCategorias.existeCategoria(subLimpia)) {
                arbolCategorias.insertarSubcategoria(categoriaBase, subLimpia);
            }
            rec->setSubcategoria(subLimpia);
            arbolCategorias.asociarElemento(subLimpia, rec);
        }
        if (!Validador::esNuloOProhibido(nuevoDetalleExtra)) {
            rec->setDetalleExtra(nuevoDetalleExtra);
        }

        pilaUndo.push(cmd);
        registrarEventoHistorial("ACTUALIZAR_RECURSO", cod, "", "Metadatos/estado actualizados.");
        AdminCSV::guardarInventario(rutaInventarioCSV, catalogoRecursos);
        AdminCSV::guardarTurnos(rutaTurnosCSV, catalogoRecursos, matrizTurnos);
        AdminCSV::guardarCategorias(rutaCategoriasCSV, arbolCategorias);
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 200, "Recurso actualizado correctamente.", rec->aJSON()));
    }

    ResultadoOperacion darDeBajaRecurso(const std::string& codigo, const std::string& tokenAdmin = "") {
        std::string cod = Validador::sanitizarCampoCSV(codigo);
        Recurso* rec = nullptr;
        if (Validador::esNuloOProhibido(cod) || !hashRecursos.buscar(cod, rec) || rec == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado para baja: " + cod));
        }
        if (!rec->isDisponible() && rec->getEstado() == "prestado") {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "No se puede dar de baja un recurso prestado actualmente."));
        }
        if (rec->tieneColaEspera()) {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "No se puede dar de baja: hay usuarios en lista de espera."));
        }

        OperacionUndo cmd;
        cmd.tipo = UNDO_BAJA_RECURSO;
        cmd.codigoRecurso = cod;
        cmd.estadoPrevioRecurso = exportarRecursoADTO(rec);
        cmd.descripcion = "Baja de recurso " + cod;

        // P1 Fix #2: Limpiar fila en MatrizDisponibilidad antes de remover de estructuras
        matrizTurnos.eliminarRecurso(cod);
        removerRecursoDeEstructuras(cod, true);
        pilaUndo.push(cmd);
        registrarEventoHistorial("BAJA_RECURSO", cod, "", "Recurso dado de baja del inventario.");
        AdminCSV::guardarInventario(rutaInventarioCSV, catalogoRecursos);
        AdminCSV::guardarTurnos(rutaTurnosCSV, catalogoRecursos, matrizTurnos);
        AdminCSV::guardarCategorias(rutaCategoriasCSV, arbolCategorias);
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 200, "Recurso dado de baja correctamente."));
    }

    // =========================================================================
    // CRUD USUARIOS (RQF 023, 026-028)
    // =========================================================================

    ResultadoOperacion consultarUsuarioPorDocumento(const std::string& documento) const {
        std::string doc = Validador::sanitizarCampoCSV(documento);
        if (Validador::esNuloOProhibido(doc)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Documento inválido."));
        }
        Usuario* u = nullptr;
        if (!hashUsuarios.buscar(doc, u) || u == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Usuario no encontrado: " + doc));
        }
        return ResultadoOperacion(true, 200, "Usuario localizado en O(1).", u->aJSON());
    }

    ResultadoOperacion listarTodosUsuarios() const {
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < padronUsuarios.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << padronUsuarios[i]->aJSON();
        }
        oss << "]";
        return ResultadoOperacion(true, 200, "Listado de usuarios.", oss.str());
    }

    ResultadoOperacion registrarUsuario(
        const std::string& documento, const std::string& nombre, const std::string& apellido,
        bool esAdmin = false, const std::string& claveAdmin = "", const std::string& tokenAdmin = ""
    ) {
        std::string doc = Validador::sanitizarCampoCSV(documento);
        std::string nom = Validador::sanitizarCampoCSV(nombre);
        std::string ape = Validador::sanitizarCampoCSV(apellido);

        if (Validador::esNuloOProhibido(doc) || Validador::esNuloOProhibido(nom) ||
            Validador::esNuloOProhibido(ape)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Documento, nombre y apellido son obligatorios."));
        }
        if (hashUsuarios.existe(doc)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "Ya existe un usuario con documento " + doc));
        }

        Usuario* nuevo = nullptr;
        if (esAdmin) {
            if (claveAdmin.size() < 6) {
                return logYRetornar("WARN", ResultadoOperacion(false, 400, "La clave administrativa debe tener al menos 6 caracteres."));
            }
            nuevo = new Administrador(doc, nom, ape, CryptoHash::crearCredencial(claveAdmin));
        } else {
            nuevo = new Usuario(doc, nom, ape, false);
        }

        padronUsuarios.push_back(nuevo);
        hashUsuarios.insertar(doc, nuevo);

        OperacionUndo cmd;
        cmd.tipo = UNDO_ALTA_USUARIO;
        cmd.documentoUsuario = doc;
        cmd.descripcion = "Registro de usuario " + doc;
        pilaUndo.push(cmd);

        registrarEventoHistorial("ALTA_USUARIO", "", doc, "Usuario registrado: " + nom + " " + ape);
        AdminCSV::guardarUsuarios(rutaUsuariosCSV, padronUsuarios);
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 201, "Usuario registrado correctamente.", nuevo->aJSON()));
    }

    ResultadoOperacion actualizarUsuario(
        const std::string& documento, const std::string& nuevoNombre, const std::string& nuevoApellido,
        bool cambiarSancion, bool nuevaSancion, const std::string& tokenAdmin = ""
    ) {
        std::string doc = Validador::sanitizarCampoCSV(documento);
        Usuario* u = nullptr;
        if (Validador::esNuloOProhibido(doc) || !hashUsuarios.buscar(doc, u) || u == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Usuario no encontrado para actualizar: " + doc));
        }

        OperacionUndo cmd;
        cmd.tipo = UNDO_ACTUALIZAR_USUARIO;
        cmd.documentoUsuario = doc;
        cmd.estadoPrevioUsuario = exportarUsuarioADTO(u);
        cmd.descripcion = "Actualización de usuario " + doc;

        if (!Validador::esNuloOProhibido(nuevoNombre)) u->setNombre(Validador::sanitizarCampoCSV(nuevoNombre));
        if (!Validador::esNuloOProhibido(nuevoApellido)) u->setApellido(Validador::sanitizarCampoCSV(nuevoApellido));
        if (cambiarSancion) u->setSancionado(nuevaSancion);

        pilaUndo.push(cmd);
        registrarEventoHistorial("ACTUALIZAR_USUARIO", "", doc, "Datos/sanción actualizados.");
        AdminCSV::guardarUsuarios(rutaUsuariosCSV, padronUsuarios);
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 200, "Usuario actualizado correctamente.", u->aJSON()));
    }

    ResultadoOperacion darDeBajaUsuario(const std::string& documento, const std::string& tokenAdmin = "") {
        std::string doc = Validador::sanitizarCampoCSV(documento);
        Usuario* u = nullptr;
        if (Validador::esNuloOProhibido(doc) || !hashUsuarios.buscar(doc, u) || u == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Usuario no encontrado para baja: " + doc));
        }
        if (u->esAdmin()) {
            return logYRetornar("SECURITY", ResultadoOperacion(false, 403, "No se permite eliminar cuentas administrativas."));
        }
        if (contarPrestamosActivosUsuario(doc) > 0) {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "No se puede dar de baja: el usuario tiene préstamos activos."));
        }
        if (u->isSancionado()) {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "No se puede dar de baja: el usuario tiene una sanción vigente."));
        }

        OperacionUndo cmd;
        cmd.tipo = UNDO_BAJA_USUARIO;
        cmd.documentoUsuario = doc;
        cmd.estadoPrevioUsuario = exportarUsuarioADTO(u);
        cmd.descripcion = "Baja de usuario " + doc;

        hashUsuarios.eliminar(doc);
        for (int i = 0; i < padronUsuarios.getTamano(); ++i) {
            if (padronUsuarios[i] == u) {
                padronUsuarios.eliminarEn(i);
                break;
            }
        }
        delete u;

        pilaUndo.push(cmd);
        registrarEventoHistorial("BAJA_USUARIO", "", doc, "Usuario dado de baja.");
        AdminCSV::guardarUsuarios(rutaUsuariosCSV, padronUsuarios);
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 200, "Usuario dado de baja correctamente."));
    }

    // =========================================================================
    // PRÉSTAMOS, DEVOLUCIONES, TURNOS Y UNDO (RQF 004-009)
    // =========================================================================

    ResultadoOperacion registrarPrestamo(
        const std::string& documentoUsuario, const std::string& codigoRecurso, bool encolarSiOcupado = true, const std::string& tokenAdmin = ""
    ) {
        std::string doc = Validador::sanitizarCampoCSV(documentoUsuario);
        std::string cod = Validador::sanitizarCampoCSV(codigoRecurso);

        Usuario* u = nullptr;
        if (Validador::esNuloOProhibido(doc) || !hashUsuarios.buscar(doc, u) || u == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Usuario no registrado: " + doc));
        }
        if (u->isSancionado()) {
            return logYRetornar("SECURITY", ResultadoOperacion(false, 403, "Préstamo denegado: usuario con sanción activa."));
        }
        if (contarPrestamosActivosUsuario(doc) >= MAX_PRESTAMOS_SIMULTANEOS) {
            return logYRetornar("SECURITY", ResultadoOperacion(false, 403, "Préstamo denegado: límite de 3 préstamos simultáneos."));
        }

        Recurso* r = nullptr;
        if (Validador::esNuloOProhibido(cod) || !hashRecursos.buscar(cod, r) || r == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado: " + cod));
        }

        if (!r->isDisponible() || r->getEstado() != "disponible") {
            if (encolarSiOcupado) {
                if (r->encolarUsuarioEspera(doc)) {
                    AdminCSV::guardarEspera(rutaEsperaCSV, catalogoRecursos);
                    registrarEventoHistorial("LISTA_ESPERA", cod, doc, "Usuario encolado en espera FIFO.");
                    return logYRetornar("INFO", ResultadoOperacion(
                        true, 202,
                        "Recurso ocupado. Encolado en lista de espera (posición " +
                        std::to_string(r->getTamanoColaEspera()) + ").",
                        r->aJSON()
                    ));
                }
                // P1 Fix #2: Si el usuario ya está en la cola, no duplicar y retornar 202 (Accepted)
                return logYRetornar("INFO", ResultadoOperacion(
                    true, 202,
                    "El usuario ya está en la cola de espera de este recurso.",
                    r->aJSON()
                ));
            }
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "Recurso no disponible (estado: " + r->getEstado() + ")."));
        }

        int idP = siguienteIdPrestamo++;
        std::string fecha = Validador::obtenerTimestampActual();

        r->setDisponible(false);
        r->setEstado("prestado");
        r->incrementarPrestamos();
        matrizTurnos.establecerDisponibilidadSede(cod, r->getSedeId(), false);

        PrestamoActivo activo(idP, cod, doc, r->getSedeId(), fecha);
        prestamosActivos.insertarFinal(activo);

        RegistroPrestamoCSV regCSV;
        regCSV.idPrestamo = idP;
        regCSV.codigoRecurso = cod;
        regCSV.documentoUsuario = doc;
        regCSV.sedeId = r->getSedeId();
        regCSV.fechaPrestamo = fecha;
        regCSV.activo = true;
        registroCompletoPrestamos.push_back(regCSV);

        OperacionUndo cmd;
        cmd.tipo = UNDO_PRESTAMO;
        cmd.idPrestamo = idP;
        cmd.codigoRecurso = cod;
        cmd.documentoUsuario = doc;
        cmd.sedeId = r->getSedeId();
        cmd.fechaOriginal = fecha;
        cmd.descripcion = "Préstamo #" + std::to_string(idP) + " de " + cod + " a " + doc;
        pilaUndo.push(cmd);

        registrarEventoHistorial("PRESTAMO", cod, doc, "Préstamo aprobado (ID " + std::to_string(idP) + ").");
        persistirTodo();
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 201, "Préstamo aprobado correctamente.", activo.aJSON()));
    }

    ResultadoOperacion registrarDevolucion(const std::string& codigoRecurso, const std::string& tokenAdmin = "") {
        std::string cod = Validador::sanitizarCampoCSV(codigoRecurso);
        Recurso* r = nullptr;
        if (Validador::esNuloOProhibido(cod) || !hashRecursos.buscar(cod, r) || r == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado para devolución: " + cod));
        }

        PrestamoActivo encontrado;
        bool removido = prestamosActivos.eliminarSi(
            [&](const PrestamoActivo& p) { return p.codigoRecurso == cod; },
            encontrado
        );
        if (!removido) {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "El recurso no tiene préstamo activo."));
        }

        std::string fechaDev = Validador::obtenerTimestampActual();
        for (int i = 0; i < registroCompletoPrestamos.getTamano(); ++i) {
            if (registroCompletoPrestamos[i].idPrestamo == encontrado.idPrestamo) {
                registroCompletoPrestamos[i].activo = false;
                registroCompletoPrestamos[i].fechaDevolucion = fechaDev;
                break;
            }
        }

        r->setDisponible(true);
        r->setEstado("disponible");
        matrizTurnos.establecerDisponibilidadSede(cod, r->getSedeId(), true);

        std::string siguienteEnEspera = "";
        bool habiaEspera = r->desencolarSiguienteEspera(siguienteEnEspera);

        OperacionUndo cmd;
        cmd.tipo = UNDO_DEVOLUCION;
        cmd.idPrestamo = encontrado.idPrestamo;
        cmd.codigoRecurso = cod;
        cmd.documentoUsuario = encontrado.documentoUsuario;
        cmd.usuarioDesencoladoEspera = habiaEspera ? siguienteEnEspera : "";
        cmd.sedeId = encontrado.sedeId;
        cmd.fechaOriginal = encontrado.fechaPrestamo;
        cmd.descripcion = "Devolución de " + cod + " por " + encontrado.documentoUsuario;
        pilaUndo.push(cmd);

        std::string detalle = "Devolución registrada (ID " + std::to_string(encontrado.idPrestamo) + ").";
        if (habiaEspera) detalle += " Siguiente turno FIFO: " + siguienteEnEspera;
        registrarEventoHistorial("DEVOLUCION", cod, encontrado.documentoUsuario, detalle);
        persistirTodo();
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }

        std::ostringstream oss;
        oss << "{\"recurso\":" << r->aJSON()
            << ",\"siguienteEnEspera\":\"" << Validador::escaparJSON(siguienteEnEspera) << "\"}";
        return logYRetornar("INFO", ResultadoOperacion(
            true, 200,
            habiaEspera ? ("Devolución registrada. Siguiente en espera FIFO: " + siguienteEnEspera)
                        : "Devolución registrada correctamente.",
            oss.str()
        ));
    }

    ResultadoOperacion reservarTurnoSemanal(const std::string& codigoRecurso, int sedeId, int dia, const std::string& tokenAdmin = "") {
        std::string cod = Validador::sanitizarCampoCSV(codigoRecurso);
        if (!hashRecursos.existe(cod)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado para turno: " + cod));
        }
        // P1 Fix #2: Validar existeRecurso en MatrizDisponibilidad (evitar filas fantasma)
        if (!matrizTurnos.existeRecurso(cod)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no existe en matriz de disponibilidad: " + cod));
        }
        if (sedeId < 0 || sedeId >= grafoSedes.getNumSedes() || dia < 0 || dia >= MatrizDisponibilidad::DIAS_SEMANA) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Sede o día fuera de rango (día: 0-6)."));
        }
        if (!matrizTurnos.estaDisponible(cod, sedeId, dia)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "El turno seleccionado ya está ocupado."));
        }

        OperacionUndo cmd;
        cmd.tipo = UNDO_TURNO_MATRIZ;
        cmd.codigoRecurso = cod;
        cmd.sedeId = sedeId;
        cmd.snapshotMatriz = matrizTurnos.capturarSnapshot();
        cmd.descripcion = "Reserva de turno en matriz (" + cod + ", sede " + std::to_string(sedeId) + ")";

        matrizTurnos.reservarTurno(cod, sedeId, dia);
        pilaUndo.push(cmd);
        registrarEventoHistorial("RESERVA_TURNO", cod, "", cmd.descripcion);
        AdminCSV::guardarTurnos(rutaTurnosCSV, catalogoRecursos, matrizTurnos);
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 200, "Turno reservado exitosamente."));
    }

    ResultadoOperacion consultarDisponibilidadSemanal(const std::string& codigoRecurso) const {
        std::string cod = Validador::sanitizarCampoCSV(codigoRecurso);
        if (!matrizTurnos.existeRecurso(cod)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado en matriz: " + cod));
        }
        std::ostringstream oss;
        oss << "{\"codigo\":\"" << Validador::escaparJSON(cod) << "\",\"sedes\":[";
        for (int s = 0; s < grafoSedes.getNumSedes(); ++s) {
            if (s > 0) oss << ",";
            oss << "{\"sedeId\":" << s
                << ",\"nombre\":\"" << Validador::escaparJSON(grafoSedes.getNombreSede(s))
                << "\",\"dias\":[";
            for (int d = 0; d < MatrizDisponibilidad::DIAS_SEMANA; ++d) {
                if (d > 0) oss << ",";
                oss << (matrizTurnos.estaDisponible(cod, s, d) ? "true" : "false");
            }
            oss << "]}";
        }
        oss << "]}";
        return ResultadoOperacion(true, 200, "Disponibilidad semanal consultada.", oss.str());
    }

    ResultadoOperacion deshacerUltimaOperacion(const std::string& tokenAdmin = "") {
        OperacionUndo cmd;
        if (!pilaUndo.pop(cmd)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "No hay operaciones recientes para deshacer."));
        }
        std::cout << "[DEBUG] [GestorCRAI] Ejecutando Undo: " << cmd.descripcion << std::endl;

        switch (cmd.tipo) {
            case UNDO_PRESTAMO: {
                PrestamoActivo dummy;
                prestamosActivos.eliminarSi([&](const PrestamoActivo& p) { return p.idPrestamo == cmd.idPrestamo; }, dummy);
                for (int i = registroCompletoPrestamos.getTamano() - 1; i >= 0; --i) {
                    if (registroCompletoPrestamos[i].idPrestamo == cmd.idPrestamo) {
                        registroCompletoPrestamos.eliminarEn(i);
                        break;
                    }
                }
                Recurso* r = nullptr;
                if (hashRecursos.buscar(cmd.codigoRecurso, r) && r != nullptr) {
                    r->setDisponible(true);
                    r->setEstado("disponible");
                    r->decrementarPrestamos();
                    matrizTurnos.establecerDisponibilidadSede(cmd.codigoRecurso, r->getSedeId(), true);
                }
                break;
            }
            case UNDO_DEVOLUCION: {
                Recurso* r = nullptr;
                if (hashRecursos.buscar(cmd.codigoRecurso, r) && r != nullptr) {
                    r->setDisponible(false);
                    r->setEstado("prestado");
                    matrizTurnos.establecerDisponibilidadSede(cmd.codigoRecurso, r->getSedeId(), false);
                    if (!cmd.usuarioDesencoladoEspera.empty()) {
                        r->reencolarAlFrenteEspera(cmd.usuarioDesencoladoEspera);
                    }
                }
                prestamosActivos.insertarFinal(PrestamoActivo(
                    cmd.idPrestamo, cmd.codigoRecurso, cmd.documentoUsuario, cmd.sedeId, cmd.fechaOriginal
                ));
                for (int i = 0; i < registroCompletoPrestamos.getTamano(); ++i) {
                    if (registroCompletoPrestamos[i].idPrestamo == cmd.idPrestamo) {
                        registroCompletoPrestamos[i].activo = true;
                        registroCompletoPrestamos[i].fechaDevolucion = "";
                        break;
                    }
                }
                break;
            }
            case UNDO_TURNO_MATRIZ:
                matrizTurnos.restaurarSnapshot(cmd.snapshotMatriz);
                AdminCSV::guardarTurnos(rutaTurnosCSV, catalogoRecursos, matrizTurnos);
                break;
            case UNDO_ALTA_RECURSO:
                matrizTurnos.eliminarRecurso(cmd.codigoRecurso);
                removerRecursoDeEstructuras(cmd.codigoRecurso, true);
                break;
            case UNDO_BAJA_RECURSO: {
                Recurso* restaurado = construirRecursoDesdeDTO(cmd.estadoPrevioRecurso);
                if (restaurado != nullptr) vincularRecursoEnEstructuras(restaurado);
                break;
            }
            case UNDO_ACTUALIZAR_RECURSO: {
                Recurso* r = nullptr;
                if (hashRecursos.buscar(cmd.codigoRecurso, r) && r != nullptr) {
                    int sedeActual = r->getSedeId();
                    avlRecursosPorNombre.eliminar(claveAVL(r->getNombre(), r->getCodigo()));
                    arbolCategorias.desasociarElemento(r->getSubcategoria(), r);
                    r->setNombre(cmd.estadoPrevioRecurso.nombre);
                    r->setEstado(cmd.estadoPrevioRecurso.estado);
                    r->setDisponible(cmd.estadoPrevioRecurso.disponible);
                    r->setSedeId(cmd.estadoPrevioRecurso.sedeId);
                    r->setSubcategoria(cmd.estadoPrevioRecurso.subcategoria);
                    r->setDetalleExtra(cmd.estadoPrevioRecurso.detalleExtra);
                    avlRecursosPorNombre.insertar(claveAVL(r->getNombre(), r->getCodigo()), r);
                    arbolCategorias.asociarElemento(r->getSubcategoria(), r);
                    if (sedeActual != r->getSedeId()) {
                        matrizTurnos.establecerDisponibilidadSede(cmd.codigoRecurso, sedeActual, true);
                    }
                    matrizTurnos.establecerDisponibilidadSede(cmd.codigoRecurso, r->getSedeId(), r->isDisponible());
                }
                break;
            }
            case UNDO_ALTA_USUARIO: {
                Usuario* u = nullptr;
                if (hashUsuarios.buscar(cmd.documentoUsuario, u) && u != nullptr) {
                    hashUsuarios.eliminar(cmd.documentoUsuario);
                    for (int i = 0; i < padronUsuarios.getTamano(); ++i) {
                        if (padronUsuarios[i] == u) {
                            padronUsuarios.eliminarEn(i);
                            break;
                        }
                    }
                    delete u;
                }
                break;
            }
            case UNDO_BAJA_USUARIO: {
                Usuario* u = new Usuario(cmd.estadoPrevioUsuario.documento,
                                         cmd.estadoPrevioUsuario.nombre,
                                         cmd.estadoPrevioUsuario.apellido,
                                         cmd.estadoPrevioUsuario.sancionado);
                padronUsuarios.push_back(u);
                hashUsuarios.insertar(u->getDocumento(), u);
                break;
            }
            case UNDO_ACTUALIZAR_USUARIO: {
                Usuario* u = nullptr;
                if (hashUsuarios.buscar(cmd.documentoUsuario, u) && u != nullptr) {
                    u->setNombre(cmd.estadoPrevioUsuario.nombre);
                    u->setApellido(cmd.estadoPrevioUsuario.apellido);
                    u->setSancionado(cmd.estadoPrevioUsuario.sancionado);
                }
                break;
            }
            default:
                return logYRetornar("ERROR", ResultadoOperacion(false, 400, "Operación Undo no reconocida."));
        }

        registrarEventoHistorial("UNDO", cmd.codigoRecurso, cmd.documentoUsuario, "Se deshizo: " + cmd.descripcion);
        persistirTodo();
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 200, "Operación revertida: " + cmd.descripcion));
    }

    // =========================================================================
    // ORDENAMIENTOS, CATEGORÍAS, HISTORIAL, GRAFO Y REPORTES (RQF 007, 011-017)
    // =========================================================================

    ResultadoOperacion listarRecursosOrdenados(const std::string& criterio) const {
        std::string crit = Validador::aMinusculas(Validador::normalizarTexto(criterio));
        VectorDinamico<Recurso*> lista;
        if (crit == "nombre") {
            lista = avlRecursosPorNombre.recorridoInorden();
        } else {
            lista = catalogoRecursos;
            if (crit == "mas_prestados") {
                bucketSortPorMasPrestados(lista);
            } else if (crit == "antiguedad") {
                lista.heapSort([](const Recurso* a, const Recurso* b) {
                    return a->getAnioAdquisicion() < b->getAnioAdquisicion();
                });
            } else if (crit == "categoria") {
                lista.insertionSort([](const Recurso* a, const Recurso* b) {
                    return a->getCategoria() < b->getCategoria();
                });
            } else if (crit == "estado") {
                lista.insertionSort([](const Recurso* a, const Recurso* b) {
                    return a->getEstado() < b->getEstado();
                });
            } else {
                lista.mergeSort([](const Recurso* a, const Recurso* b) {
                    return a->getCodigo() < b->getCodigo();
                });
            }
        }
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < lista.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << lista[i]->aJSON();
        }
        oss << "]";
        return ResultadoOperacion(true, 200, "Catálogo ordenado por " + (crit.empty() ? "codigo" : crit), oss.str());
    }

    ResultadoOperacion consultarCategoria(const std::string& nombreCategoria, const std::string& buscarCodigo = "") const {
        std::string cat = Validador::sanitizarCampoCSV(nombreCategoria);
        if (Validador::esNuloOProhibido(cat)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Nombre de categoría inválido."));
        }
        if (!arbolCategorias.existeCategoria(cat)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Categoría o subcategoría no encontrada: " + cat));
        }
        VectorDinamico<Recurso*> recs = arbolCategorias.consultarTodoBajoCategoria(cat);
        // P0 Fix: Conectar VectorDinamico::busquedaBinaria() a flujo real
        // Ordenar por código para poder usar búsqueda binaria
        recs.mergeSort([](const Recurso* a, const Recurso* b) {
            return a->getCodigo() < b->getCodigo();
        });

        // Si se proporciona un código a buscar, usar búsqueda binaria manual (el template requiere T=Recurso*)
        if (!buscarCodigo.empty()) {
            std::string codigoBuscar = Validador::sanitizarCampoCSV(buscarCodigo);
            int izquierda = 0;
            int derecha = recs.getTamano() - 1;
            int idx = -1;
            while (izquierda <= derecha) {
                int medio = izquierda + (derecha - izquierda) / 2;
                std::string codigoMedio = recs[medio]->getCodigo();
                if (codigoMedio == codigoBuscar) {
                    idx = medio;
                    break;
                }
                if (codigoMedio < codigoBuscar) {
                    izquierda = medio + 1;
                } else {
                    derecha = medio - 1;
                }
            }
            if (idx >= 0) {
                std::ostringstream oss;
                oss << "{\"categoria\":\"" << Validador::escaparJSON(cat)
                    << "\",\"total\":1,\"recursos\":[" << recs[idx]->aJSON() << "]}";
                return ResultadoOperacion(true, 200, "Recurso encontrado en categoría (búsqueda binaria O(log n)).", oss.str());
            } else {
                std::ostringstream oss;
                oss << "{\"categoria\":\"" << Validador::escaparJSON(cat)
                    << "\",\"total\":0,\"recursos\":[]}";
                return ResultadoOperacion(true, 200, "Recurso no encontrado en categoría.", oss.str());
            }
        }

        std::ostringstream oss;
        oss << "{\"categoria\":\"" << Validador::escaparJSON(cat)
            << "\",\"total\":" << recs.getTamano() << ",\"recursos\":[";
        for (int i = 0; i < recs.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << recs[i]->aJSON();
        }
        oss << "]}";
        return ResultadoOperacion(true, 200, "Consulta de subárbol completada.", oss.str());
    }

    ResultadoOperacion crearSubcategoria(const std::string& padre, const std::string& hija, const std::string& tokenAdmin = "") {
        std::string p = Validador::sanitizarCampoCSV(padre);
        std::string h = Validador::sanitizarCampoCSV(hija);
        if (Validador::esNuloOProhibido(p) || Validador::esNuloOProhibido(h)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 400, "Nombres de categoría inválidos."));
        }
        if (!arbolCategorias.insertarSubcategoria(p, h)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 409, "No se pudo crear la subcategoría (padre inexistente o duplicado)."));
        }
        AdminCSV::guardarCategorias(rutaCategoriasCSV, arbolCategorias);
        if (!tokenAdmin.empty()) {
            Administrador* admin = obtenerAdminDeSesion(tokenAdmin);
            if (admin) admin->registrarOperacion();
        }
        return logYRetornar("INFO", ResultadoOperacion(true, 201, "Subcategoría creada correctamente."));
    }

    ResultadoOperacion obtenerJerarquiaCategorias() const {
        VectorDinamico<std::string> lineas = arbolCategorias.listarJerarquia();
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < lineas.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << "\"" << Validador::escaparJSON(lineas[i]) << "\"";
        }
        oss << "]";
        return ResultadoOperacion(true, 200, "Jerarquía de categorías.", oss.str());
    }

    ResultadoOperacion obtenerHistorialGeneral(bool masRecientePrimero = true) const {
        VectorDinamico<RegistroHistorial> items = masRecientePrimero
            ? historialGeneral.obtenerAtras()
            : historialGeneral.obtenerAdelante();
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < items.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << items[i].aJSON();
        }
        oss << "]";
        return ResultadoOperacion(true, 200, "Historial general consultado.", oss.str());
    }

    ResultadoOperacion obtenerHistorialRecurso(const std::string& codigo, bool masRecientePrimero = true) const {
        std::string cod = Validador::sanitizarCampoCSV(codigo);
        Recurso* r = nullptr;
        if (Validador::esNuloOProhibido(cod) || !hashRecursos.buscar(cod, r) || r == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado: " + cod));
        }
        VectorDinamico<RegistroHistorial> items = masRecientePrimero
            ? r->obtenerHistorialAtras()
            : r->obtenerHistorialAdelante();
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < items.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << items[i].aJSON();
        }
        oss << "]";
        return ResultadoOperacion(true, 200, "Historial individual del recurso " + cod, oss.str());
    }

    ResultadoOperacion obtenerHistorialUsuario(const std::string& documento, bool masRecientePrimero = true) const {
        std::string doc = Validador::sanitizarCampoCSV(documento);
        Usuario* u = nullptr;
        if (Validador::esNuloOProhibido(doc) || !hashUsuarios.buscar(doc, u) || u == nullptr) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Usuario no encontrado: " + doc));
        }
        VectorDinamico<RegistroHistorial> items = masRecientePrimero
            ? u->obtenerHistorialAtras()
            : u->obtenerHistorialAdelante();
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < items.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << items[i].aJSON();
        }
        oss << "]";
        return ResultadoOperacion(true, 200, "Historial individual del usuario " + doc, oss.str());
    }

    ResultadoOperacion obtenerInfoSesion(const std::string& token) const {
        SesionAdmin sesion;
        if (token.empty() || !sesionesActivas.buscar(token, sesion)) {
            return ResultadoOperacion(false, 401, "Sesión inválida o expirada.");
        }
        std::ostringstream oss;
        oss << "{\"documento\":\"" << Validador::escaparJSON(sesion.documentoAdmin)
            << "\",\"nombreCompleto\":\"" << Validador::escaparJSON(sesion.nombreCompleto)
            << "\",\"fechaInicio\":\"" << Validador::escaparJSON(sesion.fechaInicio) << "\"}";
        return ResultadoOperacion(true, 200, "Sesión activa.", oss.str());
    }

    ResultadoOperacion listarPrestamosActivos() const {
        VectorDinamico<PrestamoActivo> activos = prestamosActivos.aVector();
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < activos.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << activos[i].aJSON();
        }
        oss << "]";
        return ResultadoOperacion(true, 200, "Préstamos activos.", oss.str());
    }

    ResultadoOperacion obtenerRedSedes() const {
        std::ostringstream oss;
        oss << "{\"redConexa\":" << (grafoSedes.esRedConexa() ? "true" : "false") << ",\"sedes\":[";
        for (int i = 0; i < grafoSedes.getNumSedes(); ++i) {
            if (i > 0) oss << ",";
            oss << "{\"id\":" << i
                << ",\"nombre\":\"" << Validador::escaparJSON(grafoSedes.getNombreSede(i))
                << "\",\"direccion\":\"" << Validador::escaparJSON(grafoSedes.getDireccionSede(i))
                << "\",\"conexiones\":[";
            VectorDinamico<GrafoSedes::AristaSede> ady = grafoSedes.getAdyacentes(i);
            for (int j = 0; j < ady.getTamano(); ++j) {
                if (j > 0) oss << ",";
                oss << "{\"destinoId\":" << ady[j].destinoId
                    << ",\"destinoNombre\":\"" << Validador::escaparJSON(grafoSedes.getNombreSede(ady[j].destinoId))
                    << "\",\"distanciaMetros\":" << ady[j].distanciaMetros << "}";
            }
            oss << "]}";
        }
        oss << "]}";
        return ResultadoOperacion(true, 200, "Topología de sedes del CRAI.", oss.str());
    }

    ResultadoOperacion buscarRecursoEnOtraSedeBFS(int sedeOrigen, const std::string& codigo, int maxDistanciaMetros = 10000) const {
        std::string cod = Validador::sanitizarCampoCSV(codigo);
        if (!hashRecursos.existe(cod)) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "Recurso no encontrado para búsqueda en sedes: " + cod));
        }
        GrafoSedes::ResultadoRutaSede res =
            grafoSedes.buscarSedeCercanaConRecursoBFS(sedeOrigen, cod, matrizTurnos, -1, maxDistanciaMetros);

        std::ostringstream oss;
        oss << "{\"encontrada\":" << (res.encontrada ? "true" : "false")
            << ",\"esViable\":" << (res.esViable ? "true" : "false")
            << ",\"sedeDestinoId\":" << res.sedeDestinoId
            << ",\"nombreSedeDestino\":\"" << Validador::escaparJSON(res.nombreSedeDestino)
            << "\",\"saltos\":" << res.saltos
            << ",\"distanciaTotalMetros\":" << res.distanciaTotalMetros
            << ",\"ruta\":[";
        for (int i = 0; i < res.rutaSedes.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << "\"" << Validador::escaparJSON(res.rutaSedes[i]) << "\"";
        }
        oss << "]}";
        return ResultadoOperacion(true, 200,
            res.encontrada ? "Sede alternativa localizada por BFS." : "Sin disponibilidad en otras sedes.",
            oss.str());
    }

    ResultadoOperacion calcularRutaMasCortaSedes(int idOrigen, int idDestino) const {
        VectorDinamico<std::string> ruta = grafoSedes.caminoMasCortoBFS(idOrigen, idDestino);
        if (ruta.getTamano() == 0) {
            return logYRetornar("WARN", ResultadoOperacion(false, 404, "No existe ruta válida entre las sedes indicadas."));
        }
        std::ostringstream oss;
        oss << "{\"saltos\":" << (ruta.getTamano() - 1) << ",\"ruta\":[";
        for (int i = 0; i < ruta.getTamano(); ++i) {
            if (i > 0) oss << ",";
            oss << "\"" << Validador::escaparJSON(ruta[i]) << "\"";
        }
        oss << "]}";
        return ResultadoOperacion(true, 200, "Camino más corto calculado con BFS.", oss.str());
    }

    ResultadoOperacion generarInformeIndicadores() const {
        int disponibles = 0, prestados = 0, mantenimiento = 0, totalEnEspera = 0;
        for (int i = 0; i < catalogoRecursos.getTamano(); ++i) {
            const Recurso* r = catalogoRecursos[i];
            if (r->getEstado() == "disponible") disponibles++;
            else if (r->getEstado() == "prestado") prestados++;
            else mantenimiento++;
            totalEnEspera += r->getTamanoColaEspera();
        }
        int sancionados = 0;
        for (int i = 0; i < padronUsuarios.getTamano(); ++i) {
            if (padronUsuarios[i]->isSancionado()) sancionados++;
        }
        VectorDinamico<Recurso*> top = catalogoRecursos;
        bucketSortPorMasPrestados(top);
        int limiteTop = (top.getTamano() < 5) ? top.getTamano() : 5;

        std::ostringstream oss;
        oss << "{\"totalRecursos\":" << catalogoRecursos.getTamano()
            << ",\"disponibles\":" << disponibles
            << ",\"prestados\":" << prestados
            << ",\"mantenimiento\":" << mantenimiento
            << ",\"totalUsuarios\":" << padronUsuarios.getTamano()
            << ",\"usuariosSancionados\":" << sancionados
            << ",\"prestamosActivos\":" << prestamosActivos.getTamano()
            << ",\"usuariosEnEsperaTotal\":" << totalEnEspera
            << ",\"operacionesEnPilaUndo\":" << pilaUndo.getTamano()
            << ",\"metricasEstructuras\":{"
            << "\"hashFactorCarga\":" << hashRecursos.getFactorCarga()
            << ",\"hashColisiones\":" << hashRecursos.getTotalColisiones()
            << ",\"avlAltura\":" << avlRecursosPorNombre.getAltura()
            << ",\"grafoRedConexa\":" << (grafoSedes.esRedConexa() ? "true" : "false")
            << "},\"topMasPrestados\":[";
        for (int i = 0; i < limiteTop; ++i) {
            if (i > 0) oss << ",";
            oss << top[i]->aJSON();
        }
        oss << "]}";
        return ResultadoOperacion(true, 200, "Informe de indicadores generado.", oss.str());
    }
};

#endif // GESTOR_CRAI_H
