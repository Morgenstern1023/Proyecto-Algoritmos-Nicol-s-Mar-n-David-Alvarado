/**
 * @file tests_main.cpp
 * @brief Suite de pruebas unitarias (caso normal, límite, error y persistencia entre reinicios) para ASan y CI/CD.
 * @author Oscar David Alvarado Navarrete (DA) & Nicolás Marín Quintero (NM)
 */

#include "Servicios/GestorCRAI.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <thread>

static void limpiarArchivosTest(const std::string& prefijo) {
    std::remove((prefijo + "_inv.csv").c_str());
    std::remove((prefijo + "_usu.csv").c_str());
    std::remove((prefijo + "_pre.csv").c_str());
    std::remove((prefijo + "_esp.csv").c_str());
    std::remove((prefijo + "_his.csv").c_str());
    std::remove((prefijo + "_tur.csv").c_str());
    std::remove((prefijo + "_cat.csv").c_str());
}

static void testEstructurasLineales() {
    VectorDinamico<int> v;
    for (int i = 0; i < 20; ++i) v.push_back(20 - i);
    VectorDinamico<int> vCopia = v;
    v.mergeSort([](const int& a, const int& b) { return a < b; });
    assert(v[0] == 1 && v[19] == 20);
    assert(vCopia[0] == 20);

    vCopia.heapSort([](const int& a, const int& b) { return a < b; });
    assert(vCopia[0] == 1 && vCopia[19] == 20);

    int idx = v.busquedaBinaria(15, [](const int& a, const int& b) { return a < b; });
    assert(idx == 14);

    bool lanzoExcepcion = false;
    try {
        (void)v[999];
    } catch (const std::out_of_range&) {
        lanzoExcepcion = true;
    }
    assert(lanzoExcepcion);

    Pila<int> p;
    int x = 0;
    assert(!p.pop(x));
    p.push(1); p.push(2); p.push(3);
    Pila<int> pCopia = p;
    assert(pCopia.pop(x) && x == 3);
    assert(pCopia.pop(x) && x == 2);

    Cola<std::string> c;
    std::string s;
    assert(!c.desencolar(s));
    c.encolar("U1"); c.encolar("U2");
    assert(c.desencolar(s) && s == "U1");
    assert(c.desencolar(s) && s == "U2");
    assert(c.estaVacia());

    ListaDoble<int> ld;
    ld.insertarFinal(10);
    ld.insertarFinal(20);
    auto ad = ld.obtenerAdelante();
    auto at = ld.obtenerAtras();
    assert(ad[0] == 10 && ad[1] == 20);
    assert(at[0] == 20 && at[1] == 10);
    std::cout << "[TEST OK] Estructuras lineales + Regla de los 3." << std::endl;
}

static void testEstructurasNoLineales() {
    TablaHash<int> th(1009);
    th.insertar("LIB001", 100);
    th.insertar("AÑO_Ñ_01", 200);
    int v = 0;
    assert(th.buscar("LIB001", v) && v == 100);
    assert(th.buscar("AÑO_Ñ_01", v) && v == 200);
    assert(th.eliminar("LIB001"));
    assert(!th.buscar("LIB001", v));

    ArbolAVL<int, int> avl;
    for (int i = 1; i <= 63; ++i) avl.insertar(i, i * 10);
    assert(avl.getAltura() <= 7);
    assert(avl.buscar(42, v) && v == 420);
    assert(avl.eliminar(42) && !avl.buscar(42, v));

    ArbolCategorias<std::string> ac("CRAI");
    assert(ac.insertarSubcategoria("CRAI", "Ingenieria"));
    assert(ac.insertarSubcategoria("Ingenieria", "Algoritmos"));
    assert(!ac.insertarSubcategoria("CRAI", "Algoritmos"));
    assert(ac.asociarElemento("Algoritmos", "LIB001"));
    assert(ac.consultarTodoBajoCategoria("Ingenieria").getTamano() == 1);

    MatrizDisponibilidad mat;
    GrafoSedes grafo;
    grafo.precargarSedesCRAI(mat);
    assert(grafo.esRedConexa());
    assert(mat.inicializarRecurso("COM001"));
    assert(mat.inicializarRecurso("SAL002"));

    // Eliminar último recurso de la matriz: no debe dejar entrada fantasma en indicePorCodigo
    assert(mat.eliminarRecurso("SAL002"));
    assert(!mat.existeRecurso("SAL002"));
    assert(mat.existeRecurso("COM001"));

    auto snap = mat.capturarSnapshot();
    mat.establecerDisponibilidadSede("COM001", 0, false);
    mat.establecerDisponibilidadSede("COM001", 1, false);
    mat.establecerDisponibilidadSede("COM001", 4, false);
    auto resBFS = grafo.buscarSedeCercanaConRecursoBFS(0, "COM001", mat, -1, 10000);
    assert(resBFS.encontrada && resBFS.nombreSedeDestino == "Sede Norte" && !resBFS.esViable);
    assert(mat.restaurarSnapshot(snap));
    assert(mat.estaDisponible("COM001", 0, 0));

    // Decodificación URL para categorías con espacios
    assert(Validador::decodificarURL("Ciencias%20Basicas") == "Ciencias Basicas");
    assert(Validador::decodificarURL("Equipos+de+Computo") == "Equipos de Computo");
    std::cout << "[TEST OK] TablaHash, AVL, ArbolCategorias, Matriz (swap-and-pop), GrafoSedes y URL decode." << std::endl;
}

static void testGestorCooldownUnlockYCorrupcionCSV() {
    const std::string p = "/tmp/test_crai_sec";
    limpiarArchivosTest(p);

    {
        std::ofstream out(p + "_inv.csv");
        out << "categoria,codigo,estado,disponible\n";
        out << "libro,LIB001,disponible,1\n";
        out << "libro,nullptr,disponible,1\n";
        out << "invalido,XXX999,disponible,1\n";
        out << "computador,COM002,disponible,abc\n";
        out << "sala,SAL003,disponible,1\n";
    }

    GestorCRAI gestor(
        p + "_inv.csv", p + "_usu.csv", p + "_pre.csv", p + "_esp.csv",
        p + "_his.csv", p + "_tur.csv", p + "_cat.csv", 2
    );
    assert(gestor.inicializar());

    // RQF 010: búsqueda O(1) de recursos y usuarios
    assert(gestor.consultarRecursoPorCodigo("LIB001").exito);
    assert(gestor.consultarRecursoPorCodigo("SAL003").exito);
    assert(!gestor.consultarRecursoPorCodigo("nullptr").exito);
    assert(gestor.consultarUsuarioPorDocumento("ADMIN01").exito);
    assert(gestor.consultarUsuarioPorDocumento("NOEXISTE").codigoHttp == 404);
    assert(gestor.consultarUsuarioPorDocumento("nullptr").codigoHttp == 400);

    // PE-19: categoría inexistente debe retornar 404 explícito
    assert(gestor.consultarCategoria("CategoriaFantasma").codigoHttp == 404);
    assert(gestor.consultarCategoria("Ciencias Basicas").exito);

    // 1. Cooldown tras 3 fallos
    assert(gestor.iniciarSesionAdmin("ADMIN01", "mala1").codigoHttp == 401);
    assert(gestor.iniciarSesionAdmin("ADMIN01", "mala2").codigoHttp == 401);
    assert(gestor.iniciarSesionAdmin("ADMIN01", "mala3").codigoHttp == 423);
    assert(gestor.iniciarSesionAdmin("ADMIN01", "CraiUR2026!").codigoHttp == 423);

    // 2. Desbloqueo inmediato por terminal (--unlock)
    assert(gestor.desbloquearAdminPorTerminal("ADMIN01").exito);
    auto loginOk = gestor.iniciarSesionAdmin("ADMIN01", "CraiUR2026!");
    assert(loginOk.exito && loginOk.codigoHttp == 200);

    // 3. Expiración natural del Cooldown (2s)
    assert(gestor.iniciarSesionAdmin("ADMIN01", "mala1").codigoHttp == 401);
    assert(gestor.iniciarSesionAdmin("ADMIN01", "mala2").codigoHttp == 401);
    assert(gestor.iniciarSesionAdmin("ADMIN01", "mala3").codigoHttp == 423);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    assert(gestor.iniciarSesionAdmin("ADMIN01", "CraiUR2026!").exito);

    // 4. Préstamo, blindaje actualizarRecurso, espera FIFO, devolución y Undo
    assert(!gestor.registrarUsuario("nullptr", "Ana", "Gomez").exito);
    assert(gestor.registrarUsuario("1001", "Ana", "Gomez").exito);
    assert(gestor.registrarUsuario("1002", "Luis", "Perez").exito);

    // Prohibido poner "prestado" manualmente en actualizarRecurso
    assert(gestor.actualizarRecurso("LIB001", "", "prestado", -1, "", "").codigoHttp == 403);

    assert(gestor.registrarPrestamo("1001", "LIB001", true).exito);
    // Prohibido cambiar estado o sede mientras está prestado
    assert(gestor.actualizarRecurso("LIB001", "", "disponible", -1, "", "").codigoHttp == 409);
    assert(gestor.actualizarRecurso("LIB001", "", "", 2, "", "").codigoHttp == 409);

    auto espera = gestor.registrarPrestamo("1002", "LIB001", true);
    assert(espera.exito && espera.codigoHttp == 202);
    assert(!gestor.darDeBajaRecurso("LIB001").exito);

    assert(gestor.registrarDevolucion("LIB001").exito);
    assert(gestor.deshacerUltimaOperacion().exito);

    limpiarArchivosTest(p);
    std::cout << "[TEST OK] Cooldown anti fuerza bruta, --unlock, CSV corrupto, blindaje CRUD y Undo." << std::endl;
}

static void testPersistenciaCompletaEIntegridadEntreReinicios() {
    const std::string p = "/tmp/test_crai_persist";
    limpiarArchivosTest(p);

    // Crear inventario con 1 recurso prestado HUÉRFANO (LIB_ORPHAN) y 2 disponibles
    {
        std::ofstream out(p + "_inv.csv");
        out << "categoria,codigo,nombre,estado,disponible,sede_id,anio_adquisicion,veces_prestado,subcategoria,detalle_extra\n";
        out << "libro,LIB_ORPHAN,Libro Huerfano,prestado,0,0,2022,5,Ingenieria,Editorial UR\n";
        out << "libro,LIB100,Estructuras de Datos,disponible,1,0,2024,2,Ingenieria,Editorial UR\n";
        out << "sala,SAL100,Sala Colaborativa 1,disponible,1,1,2025,1,Espacios Colaborativos,8\n";
    }

    // CICLO 1: Inicializar, comprobar saneamiento del huérfano y mutar las 7 persistencias
    {
        GestorCRAI g1(
            p + "_inv.csv", p + "_usu.csv", p + "_pre.csv", p + "_esp.csv",
            p + "_his.csv", p + "_tur.csv", p + "_cat.csv", 60
        );
        assert(g1.inicializar());

        // LIB_ORPHAN no tenía préstamo en prestamos.csv -> debe haberse corregido a disponible
        auto rOrphan = g1.consultarRecursoPorCodigo("LIB_ORPHAN");
        assert(rOrphan.exito && rOrphan.dataJSON.find("\"estado\":\"disponible\"") != std::string::npos);

        // Registrar usuarios (uno sancionado)
        assert(g1.registrarUsuario("2001", "Oscar", "Alvarado").exito);
        assert(g1.registrarUsuario("2002", "Nicolas", "Marin").exito);
        assert(g1.registrarUsuario("2003", "Usuario", "Sancionado").exito);
        assert(g1.actualizarUsuario("2003", "", "", true, true).exito);

        // Subcategoría personalizada (categorias.csv)
        assert(g1.crearSubcategoria("Ingenieria", "Algoritmos Avanzados").exito);

        // Turno semanal reservado en matriz (turnos.csv)
        assert(g1.reservarTurnoSemanal("SAL100", 1, 3).exito);

        // Préstamo activo (prestamos.csv) + cola de espera FIFO (espera.csv)
        assert(g1.registrarPrestamo("2001", "LIB100", true).exito);
        assert(g1.registrarPrestamo("2002", "LIB100", true).codigoHttp == 202);
        // Usuario sancionado bloqueado (403)
        assert(g1.registrarPrestamo("2003", "LIB_ORPHAN", true).codigoHttp == 403);
    }

    // CICLO 2: Destruido g1, recrear g2 desde disco y validar TODA la persistencia real
    {
        GestorCRAI g2(
            p + "_inv.csv", p + "_usu.csv", p + "_pre.csv", p + "_esp.csv",
            p + "_his.csv", p + "_tur.csv", p + "_cat.csv", 60
        );
        assert(g2.inicializar());

        // 1. Usuarios y sanciones persistidos (usuarios.csv)
        auto u1 = g2.consultarUsuarioPorDocumento("2001");
        auto u3 = g2.consultarUsuarioPorDocumento("2003");
        assert(u1.exito && u1.dataJSON.find("Oscar") != std::string::npos);
        assert(u3.exito && u3.dataJSON.find("\"sancionado\":true") != std::string::npos);

        // 2. Préstamo activo persistido Y recurso mantenido en 'prestado' (prestamos.csv + inventario.csv)
        auto recPrest = g2.consultarRecursoPorCodigo("LIB100");
        assert(recPrest.exito);
        assert(recPrest.dataJSON.find("\"estado\":\"prestado\"") != std::string::npos);
        assert(recPrest.dataJSON.find("\"enEspera\":1") != std::string::npos);
        auto activos = g2.listarPrestamosActivos();
        assert(activos.exito && activos.dataJSON.find("LIB100") != std::string::npos);
        assert(activos.dataJSON.find("2001") != std::string::npos);

        // 3. Cola FIFO persistida (espera.csv): al devolver LIB100 debe notificar a 2002
        auto dev = g2.registrarDevolucion("LIB100");
        assert(dev.exito && dev.dataJSON.find("\"siguienteEnEspera\":\"2002\"") != std::string::npos);

        // 4. Turno de matriz persistido (turnos.csv): sede 1 día 3 debe seguir ocupado (409)
        assert(g2.reservarTurnoSemanal("SAL100", 1, 3).codigoHttp == 409);

        // 5. Subcategoría personalizada persistida (categorias.csv)
        assert(g2.consultarCategoria("Algoritmos Avanzados").exito);

        // 6. Historial general e individual persistidos en orden cronológico (historial.csv)
        auto histRec = g2.obtenerHistorialRecurso("LIB100", false);
        assert(histRec.exito && histRec.dataJSON.find("PRESTAMO") != std::string::npos);
        assert(histRec.dataJSON.find("DEVOLUCION") != std::string::npos);
        auto histUsu = g2.obtenerHistorialUsuario("2001", false);
        assert(histUsu.exito && histUsu.dataJSON.find("ALTA_USUARIO") != std::string::npos);
    }

    limpiarArchivosTest(p);
    std::cout << "[TEST OK] Persistencia completa entre reinicios (7 CSVs) e integridad de huérfanos al arranque." << std::endl;
}

int main() {
    testEstructurasLineales();
    testEstructurasNoLineales();
    testGestorCooldownUnlockYCorrupcionCSV();
    testPersistenciaCompletaEIntegridadEntreReinicios();
    std::cout << "\n=== TODAS LAS PRUEBAS SUPERADAS SIN FUGAS DE MEMORIA ===" << std::endl;
    return 0;
}
