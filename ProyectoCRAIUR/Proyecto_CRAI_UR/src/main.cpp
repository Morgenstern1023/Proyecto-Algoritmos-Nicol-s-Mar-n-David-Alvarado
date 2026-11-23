/**
 * @file main.cpp
 * @brief Backend REST local (cpp-httplib), consola de logs en tiempo real y utilidad CLI `--unlock <DOC>`.
 * @author Oscar David Alvarado Navarrete (DA)
 */

#include "Servicios/GestorCRAI.h"
#include "httplib.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

using json = nlohmann::json;

namespace {

void abrirNavegadorMultiplataforma(const std::string& url) {
    std::string comando;
#ifdef _WIN32
    comando = "start \"\" \"" + url + "\"";
#elif __APPLE__
    comando = "open \"" + url + "\" >/dev/null 2>&1 &";
#elif __linux__
    comando = "xdg-open \"" + url + "\" >/dev/null 2>&1 &";
#else
    std::cout << "[INFO] Sistema no reconocido. Abre manualmente: " << url << std::endl;
    return;
#endif
    int rc = std::system(comando.c_str());
    if (rc != 0) {
        std::cout << "[WARN] No se pudo lanzar el navegador automáticamente. Abre: " << url << std::endl;
    }
}

std::string leerStringJSON(const json& body, const std::string& clave, const std::string& defecto = "") {
    if (!body.contains(clave) || body[clave].is_null()) return defecto;
    if (body[clave].is_string()) return body[clave].get<std::string>();
    return defecto;
}

int leerIntJSON(const json& body, const std::string& clave, int defecto = 0) {
    if (!body.contains(clave) || body[clave].is_null()) return defecto;
    if (body[clave].is_number_integer()) return body[clave].get<int>();
    if (body[clave].is_string()) {
        int val = defecto;
        if (Validador::parsearEnteroSeguro(body[clave].get<std::string>(), val, -1000000, 1000000)) return val;
    }
    return defecto;
}

bool leerBoolJSON(const json& body, const std::string& clave, bool defecto = false) {
    if (!body.contains(clave) || body[clave].is_null()) return defecto;
    if (body[clave].is_boolean()) return body[clave].get<bool>();
    return defecto;
}

void responder(httplib::Response& res, const ResultadoOperacion& r) {
    res.status = r.codigoHttp;
    res.set_content(r.aJSON(), "application/json; charset=utf-8");
}

} // namespace

int main(int argc, char* argv[]) {
    // Opción 3: Desbloqueo de emergencia exclusivo por terminal local
    if (argc == 3 && std::string(argv[1]) == "--unlock") {
        std::cout << "[SECURITY] Ejecutando desbloqueo local por terminal para: " << argv[2] << std::endl;
        GestorCRAI gestor;
        gestor.inicializar();
        ResultadoOperacion r = gestor.desbloquearAdminPorTerminal(argv[2]);
        std::cout << (r.exito ? "[INFO] " : "[ERROR] ") << r.mensaje << std::endl;
        return r.exito ? 0 : 1;
    }
    if (argc > 1) {
        std::cout << "[WARN] Argumentos desconocidos. Uso válido:\n"
                  << "  ./bin/crai_server                  (Inicia servidor REST + Web)\n"
                  << "  ./bin/crai_server --unlock <DOC>   (Remueve cooldown localmente)" << std::endl;
        return 1;
    }

    const std::string HOST = "127.0.0.1";
    const int PUERTO = 8080;
    const std::string URL_LOCAL = "http://" + HOST + ":" + std::to_string(PUERTO);

    std::cout << "============================================================" << std::endl;
    std::cout << "  SISTEMA DE GESTION CRAI - UNIVERSIDAD DEL ROSARIO (C++17) " << std::endl;
    std::cout << "  Consola exclusiva de logs y depuracion en tiempo real     " << std::endl;
    std::cout << "============================================================" << std::endl;

    GestorCRAI gestor;
    std::mutex mtxGestor;

    try {
        gestor.inicializar();
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] Error al inicializar GestorCRAI: " << e.what() << std::endl;
        return 1;
    }

    httplib::Server svr;

    svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        std::cout << "[HTTP] " << req.method << " " << req.path << " -> " << res.status << std::endl;
    });

    svr.set_exception_handler([](const httplib::Request& req, httplib::Response& res, std::exception_ptr ep) {
        std::string detalle = "Error interno desconocido";
        int codigo = 500;
        try {
            if (ep) std::rethrow_exception(ep);
        } catch (const json::parse_error& e) {
            codigo = 400;
            detalle = std::string("JSON malformado: ") + e.what();
        } catch (const std::invalid_argument& e) {
            codigo = 400;
            detalle = e.what();
        } catch (const std::out_of_range& e) {
            codigo = 404;
            detalle = e.what();
        } catch (const std::exception& e) {
            codigo = 500;
            detalle = e.what();
        }
        std::cerr << "[ERROR] Excepción en " << req.method << " " << req.path
                  << ": " << detalle << std::endl;
        responder(res, ResultadoOperacion(false, codigo, detalle));
    });

    auto verificarAdmin = [&](const httplib::Request& req, httplib::Response& res) -> bool {
        std::string token = req.get_header_value("X-Session-Token");
        std::lock_guard<std::mutex> lock(mtxGestor);
        if (!gestor.validarTokenSesion(token)) {
            std::cout << "[SECURITY] Petición no autorizada bloqueada en " << req.path << std::endl;
            responder(res, ResultadoOperacion(false, 401, "Acceso denegado. Inicia sesión como administrador."));
            return false;
        }
        return true;
    };

    if (!svr.set_mount_point("/", "./web")) {
        std::cout << "[WARN] No se encontró la carpeta ./web en el directorio actual." << std::endl;
    }

    // --- Endpoints Públicos (Salud y Autenticación Administrativa) ---
    svr.Get("/api/salud", [](const httplib::Request&, httplib::Response& res) {
        responder(res, ResultadoOperacion(true, 200, "Servidor CRAI activo (100% offline)."));
    });

    svr.Post("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
        json body = json::parse(req.body);
        std::string doc = leerStringJSON(body, "documento");
        std::string clave = leerStringJSON(body, "clave");
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.iniciarSesionAdmin(doc, clave));
    });

    svr.Post("/api/logout", [&](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("X-Session-Token");
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.cerrarSesionAdmin(token));
    });

    svr.Get("/api/sesion", [&](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_header_value("X-Session-Token");
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.obtenerInfoSesion(token));
    });

    // --- Recursos ---
    svr.Get("/api/recursos", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::string criterio = req.has_param("orden") ? req.get_param_value("orden") : "codigo";
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.listarRecursosOrdenados(criterio));
    });

    svr.Get(R"(/api/recursos/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::string codigo = Validador::decodificarURL(req.matches[1]);
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.consultarRecursoPorCodigo(codigo));
    });

    svr.Post("/api/recursos", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        json body = json::parse(req.body);
        RegistroRecursoCSV dto;
        dto.categoria = leerStringJSON(body, "categoria");
        dto.codigo = leerStringJSON(body, "codigo");
        dto.nombre = leerStringJSON(body, "nombre");
        dto.estado = leerStringJSON(body, "estado", "disponible");
        dto.sedeId = leerIntJSON(body, "sedeId", 0);
        dto.anioAdquisicion = leerIntJSON(body, "anioAdquisicion", 2026);
        dto.vecesPrestado = leerIntJSON(body, "vecesPrestado", 0);
        dto.subcategoria = leerStringJSON(body, "subcategoria", "General");
        dto.detalleExtra = leerStringJSON(body, "detalleExtra", "Estandar");
        std::lock_guard<std::mutex> lock(mtxGestor);
        std::string token = req.get_header_value("X-Session-Token");
        responder(res, gestor.registrarRecurso(dto, token));
    });

    svr.Put(R"(/api/recursos/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::string codigo = Validador::decodificarURL(req.matches[1]);
        json body = json::parse(req.body);
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.actualizarRecurso(
            codigo,
            leerStringJSON(body, "nombre"),
            leerStringJSON(body, "estado"),
            leerIntJSON(body, "sedeId", -1),
            leerStringJSON(body, "subcategoria"),
            leerStringJSON(body, "detalleExtra"),
            req.get_header_value("X-Session-Token")
        ));
    });

    svr.Delete(R"(/api/recursos/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.darDeBajaRecurso(Validador::decodificarURL(req.matches[1]), req.get_header_value("X-Session-Token")));
    });

    // --- Usuarios ---
    svr.Get("/api/usuarios", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.listarTodosUsuarios());
    });

    svr.Get(R"(/api/usuarios/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.consultarUsuarioPorDocumento(Validador::decodificarURL(req.matches[1])));
    });

    svr.Post("/api/usuarios", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        json body = json::parse(req.body);
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.registrarUsuario(
            leerStringJSON(body, "documento"),
            leerStringJSON(body, "nombre"),
            leerStringJSON(body, "apellido"),
            leerBoolJSON(body, "esAdmin", false),
            leerStringJSON(body, "clave"),
            req.get_header_value("X-Session-Token")
        ));
    });

    svr.Put(R"(/api/usuarios/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        json body = json::parse(req.body);
        bool cambiarSancion = body.contains("sancionado") && !body["sancionado"].is_null();
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.actualizarUsuario(
            Validador::decodificarURL(req.matches[1]),
            leerStringJSON(body, "nombre"),
            leerStringJSON(body, "apellido"),
            cambiarSancion,
            leerBoolJSON(body, "sancionado", false),
            req.get_header_value("X-Session-Token")
        ));
    });

    svr.Delete(R"(/api/usuarios/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.darDeBajaUsuario(Validador::decodificarURL(req.matches[1]), req.get_header_value("X-Session-Token")));
    });

    // --- Préstamos, Devoluciones, Disponibilidad Semanal y Undo ---
    svr.Get("/api/prestamos", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.listarPrestamosActivos());
    });

    svr.Post("/api/prestamos", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        json body = json::parse(req.body);
        std::lock_guard<std::mutex> lock(mtxGestor);
        std::string token = req.get_header_value("X-Session-Token");
        responder(res, gestor.registrarPrestamo(
            leerStringJSON(body, "documentoUsuario"),
            leerStringJSON(body, "codigoRecurso"),
            leerBoolJSON(body, "encolarSiOcupado", true),
            token
        ));
    });

    svr.Post("/api/devoluciones", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        json body = json::parse(req.body);
        std::lock_guard<std::mutex> lock(mtxGestor);
        std::string token = req.get_header_value("X-Session-Token");
        responder(res, gestor.registrarDevolucion(leerStringJSON(body, "codigoRecurso"), token));
    });

    svr.Get(R"(/api/disponibilidad/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.consultarDisponibilidadSemanal(Validador::decodificarURL(req.matches[1])));
    });

    svr.Post("/api/disponibilidad/reservar", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        json body = json::parse(req.body);
        std::lock_guard<std::mutex> lock(mtxGestor);
        std::string token = req.get_header_value("X-Session-Token");
        responder(res, gestor.reservarTurnoSemanal(
            leerStringJSON(body, "codigoRecurso"),
            leerIntJSON(body, "sedeId", -1),
            leerIntJSON(body, "dia", -1),
            token
        ));
    });

    svr.Post("/api/undo", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        std::string token = req.get_header_value("X-Session-Token");
        responder(res, gestor.deshacerUltimaOperacion(token));
    });

    // --- Categorías, Historial, Sedes (BFS/DFS) e Indicadores ---
    svr.Get("/api/categorias", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.obtenerJerarquiaCategorias());
    });

    svr.Post("/api/categorias", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        json body = json::parse(req.body);
        std::lock_guard<std::mutex> lock(mtxGestor);
        std::string token = req.get_header_value("X-Session-Token");
        responder(res, gestor.crearSubcategoria(leerStringJSON(body, "padre"), leerStringJSON(body, "hija"), token));
    });

    svr.Get(R"(/api/categorias/([^/]+)/recursos)", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        std::string buscarCod = req.has_param("codigo") ? req.get_param_value("codigo") : "";
        responder(res, gestor.consultarCategoria(Validador::decodificarURL(req.matches[1]), buscarCod));
    });

    svr.Get("/api/historial", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        bool recientePrimero = !(req.has_param("orden") && req.get_param_value("orden") == "antiguo");
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.obtenerHistorialGeneral(recientePrimero));
    });

    svr.Get(R"(/api/historial/recurso/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        bool recientePrimero = !(req.has_param("orden") && req.get_param_value("orden") == "antiguo");
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.obtenerHistorialRecurso(Validador::decodificarURL(req.matches[1]), recientePrimero));
    });

    svr.Get(R"(/api/historial/usuario/([^/]+))", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        bool recientePrimero = !(req.has_param("orden") && req.get_param_value("orden") == "antiguo");
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.obtenerHistorialUsuario(Validador::decodificarURL(req.matches[1]), recientePrimero));
    });

    svr.Get("/api/sedes", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.obtenerRedSedes());
    });

    svr.Get("/api/sedes/ruta", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        int origen = -1, destino = -1;
        Validador::parsearEnteroSeguro(req.get_param_value("origen"), origen, 0, 100);
        Validador::parsearEnteroSeguro(req.get_param_value("destino"), destino, 0, 100);
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.calcularRutaMasCortaSedes(origen, destino));
    });

    svr.Get("/api/sedes/buscar-recurso", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        int origen = 0, maxDist = 10000;
        Validador::parsearEnteroSeguro(req.get_param_value("origen"), origen, 0, 100);
        if (req.has_param("maxDist")) {
            Validador::parsearEnteroSeguro(req.get_param_value("maxDist"), maxDist, 0, 100000);
        }
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.buscarRecursoEnOtraSedeBFS(origen, req.get_param_value("codigo"), maxDist));
    });

    svr.Get("/api/indicadores", [&](const httplib::Request& req, httplib::Response& res) {
        if (!verificarAdmin(req, res)) return;
        std::lock_guard<std::mutex> lock(mtxGestor);
        responder(res, gestor.generarInformeIndicadores());
    });

    std::thread([URL_LOCAL]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        abrirNavegadorMultiplataforma(URL_LOCAL);
    }).detach();

    std::cout << "[INFO] Servidor REST iniciado en " << URL_LOCAL << " (Ctrl+C para detener)" << std::endl;
    if (!svr.listen(HOST, PUERTO)) {
        std::cerr << "[FATAL] No se pudo enlazar el puerto " << PUERTO << "." << std::endl;
        return 1;
    }
    return 0;
}
