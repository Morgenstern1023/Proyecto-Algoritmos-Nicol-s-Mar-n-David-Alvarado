/**
 * @file GrafoSedes.h
 * @brief Grafo no dirigido con lista de adyacencia propia, BFS (camino más corto + viabilidad) y DFS (RQF 014-016).
 * @author Oscar David Alvarado Navarrete (DA)
 */

#ifndef GRAFO_SEDES_H
#define GRAFO_SEDES_H

#include "Cola.h"
#include "ListaSimple.h"
#include "MatrizDisponibilidad.h"
#include "VectorDinamico.h"
#include <iostream>
#include <string>

class GrafoSedes {
public:
    struct AristaSede {
        int destinoId;
        int distanciaMetros;

        AristaSede(int dest = -1, int dist = 0)
            : destinoId(dest), distanciaMetros(dist) {}

        bool operator==(const AristaSede& otra) const {
            return destinoId == otra.destinoId;
        }
    };

    struct ResultadoRutaSede {
        bool encontrada;
        bool esViable;
        int sedeDestinoId;
        std::string nombreSedeDestino;
        int saltos;
        int distanciaTotalMetros;
        VectorDinamico<std::string> rutaSedes;

        ResultadoRutaSede()
            : encontrada(false), esViable(false), sedeDestinoId(-1),
              nombreSedeDestino(""), saltos(-1), distanciaTotalMetros(0),
              rutaSedes() {}
    };

private:
    struct VerticeSede {
        int id;
        std::string nombre;
        std::string direccion;
        ListaSimple<AristaSede> adyacentes;

        VerticeSede() : id(-1), nombre(""), direccion(""), adyacentes() {}
    };

    VerticeSede* vertices;
    int numSedes;
    int capacidadSedes;

    void redimensionar() {
        int nuevaCap = (capacidadSedes <= 0) ? 6 : capacidadSedes * 2;
        VerticeSede* nuevos = new VerticeSede[nuevaCap];
        for (int i = 0; i < numSedes; ++i) nuevos[i] = vertices[i];
        delete[] vertices;
        vertices = nuevos;
        capacidadSedes = nuevaCap;
    }

    void copiarDesde(const GrafoSedes& otro) {
        numSedes = otro.numSedes;
        capacidadSedes = otro.capacidadSedes;
        vertices = (capacidadSedes > 0) ? new VerticeSede[capacidadSedes] : nullptr;
        for (int i = 0; i < numSedes; ++i) vertices[i] = otro.vertices[i];
    }

    void dfsRec(int actual, bool* visitados, VectorDinamico<std::string>& recorrido) const {
        visitados[actual] = true;
        recorrido.push_back(vertices[actual].nombre);
        const NodoSimple<AristaSede>* arista = vertices[actual].adyacentes.getCabeza();
        while (arista != nullptr) {
            int vecino = arista->dato.destinoId;
            if (vecino >= 0 && vecino < numSedes && !visitados[vecino]) {
                dfsRec(vecino, visitados, recorrido);
            }
            arista = arista->siguiente;
        }
    }

    VectorDinamico<std::string> reconstruirRuta(int idOrigen, int idDestino, const int* padre) const {
        VectorDinamico<std::string> inversa;
        int cursor = idDestino;
        int pasos = 0;
        while (cursor != -1 && pasos <= numSedes) {
            inversa.push_back(vertices[cursor].nombre);
            if (cursor == idOrigen) break;
            cursor = padre[cursor];
            pasos++;
        }
        VectorDinamico<std::string> ordenada(inversa.getTamano() > 0 ? inversa.getTamano() : 4);
        for (int i = inversa.getTamano() - 1; i >= 0; --i) {
            ordenada.push_back(inversa[i]);
        }
        return ordenada;
    }

public:
    explicit GrafoSedes(int capInicial = 8)
        : vertices(nullptr), numSedes(0), capacidadSedes(capInicial > 0 ? capInicial : 8) {
        vertices = new VerticeSede[capacidadSedes];
    }

    GrafoSedes(const GrafoSedes& otro) : vertices(nullptr), numSedes(0), capacidadSedes(0) {
        copiarDesde(otro);
    }

    GrafoSedes& operator=(const GrafoSedes& otro) {
        if (this == &otro) return *this;
        delete[] vertices;
        vertices = nullptr;
        copiarDesde(otro);
        return *this;
    }

    ~GrafoSedes() {
        delete[] vertices;
        vertices = nullptr;
        numSedes = 0;
        capacidadSedes = 0;
    }

    int agregarSede(const std::string& nombre, const std::string& direccion = "") {
        if (nombre.empty()) {
            std::cout << "[WARN] [GrafoSedes] Nombre de sede vacío rechazado." << std::endl;
            return -1;
        }
        for (int i = 0; i < numSedes; ++i) {
            if (vertices[i].nombre == nombre) return i;
        }
        if (numSedes >= capacidadSedes) redimensionar();
        int id = numSedes;
        vertices[id].id = id;
        vertices[id].nombre = nombre;
        vertices[id].direccion = direccion;
        numSedes++;
        return id;
    }

    bool conectarSedes(int idOrigen, int idDestino, int distanciaMetros = 1000) {
        if (idOrigen < 0 || idOrigen >= numSedes ||
            idDestino < 0 || idDestino >= numSedes ||
            idOrigen == idDestino || distanciaMetros < 0) {
            std::cout << "[WARN] [GrafoSedes] Conexión inválida (" << idOrigen
                      << " <-> " << idDestino << ", dist=" << distanciaMetros << ")." << std::endl;
            return false;
        }
        AristaSede a1(idDestino, distanciaMetros);
        if (!vertices[idOrigen].adyacentes.contiene(a1)) {
            vertices[idOrigen].adyacentes.insertarFinal(a1);
        }
        AristaSede a2(idOrigen, distanciaMetros);
        if (!vertices[idDestino].adyacentes.contiene(a2)) {
            vertices[idDestino].adyacentes.insertarFinal(a2);
        }
        return true;
    }

    void precargarSedesCRAI(MatrizDisponibilidad& matriz) {
        int idClaustro = agregarSede("Claustro", "Calle 12C # 6-25, La Candelaria");
        int idQuinta   = agregarSede("Quinta de Mutis", "Carrera 24 # 63C-69");
        int idGSB      = agregarSede("Emprendimiento y GSB", "Calle 200 entre Autopista Norte y Cra 7");
        int idNorte    = agregarSede("Sede Norte", "Autopista Norte # 200");
        int idMederi   = agregarSede("Hospital Mayor Mederi", "Calle 24 # 29-45");

        matriz.agregarSede("Claustro");
        matriz.agregarSede("Quinta de Mutis");
        matriz.agregarSede("Emprendimiento y GSB");
        matriz.agregarSede("Sede Norte");
        matriz.agregarSede("Hospital Mayor Mederi");

        conectarSedes(idClaustro, idMederi, 3800);
        conectarSedes(idClaustro, idQuinta, 6500);
        conectarSedes(idMederi, idQuinta, 4200);
        conectarSedes(idQuinta, idNorte, 14000);
        conectarSedes(idNorte, idGSB, 800);

        std::cout << "[INFO] [GrafoSedes] Precargadas " << numSedes
                  << " sedes reales del CRAI Universidad del Rosario." << std::endl;
    }

    VectorDinamico<std::string> caminoMasCortoBFS(int idOrigen, int idDestino) const {
        VectorDinamico<std::string> ruta;
        if (idOrigen < 0 || idOrigen >= numSedes || idDestino < 0 || idDestino >= numSedes) {
            std::cout << "[WARN] [GrafoSedes] IDs fuera de rango en caminoMasCortoBFS." << std::endl;
            return ruta;
        }
        if (idOrigen == idDestino) {
            ruta.push_back(vertices[idOrigen].nombre);
            return ruta;
        }
        std::cout << "[DEBUG] [GrafoSedes] Ejecutando BFS desde sede " << idOrigen
                  << " hacia sede " << idDestino << "..." << std::endl;

        bool* visitados = new bool[numSedes];
        int* padre = new int[numSedes];
        try {
            for (int i = 0; i < numSedes; ++i) {
                visitados[i] = false;
                padre[i] = -1;
            }
            Cola<int> colaBFS;
            visitados[idOrigen] = true;
            colaBFS.encolar(idOrigen);
            bool encontrado = false;

            while (!colaBFS.estaVacia()) {
                int actual = -1;
                colaBFS.desencolar(actual);
                if (actual == idDestino) {
                    encontrado = true;
                    break;
                }
                const NodoSimple<AristaSede>* arista = vertices[actual].adyacentes.getCabeza();
                while (arista != nullptr) {
                    int vecino = arista->dato.destinoId;
                    if (vecino >= 0 && vecino < numSedes && !visitados[vecino]) {
                        visitados[vecino] = true;
                        padre[vecino] = actual;
                        colaBFS.encolar(vecino);
                    }
                    arista = arista->siguiente;
                }
            }
            if (encontrado) {
                ruta = reconstruirRuta(idOrigen, idDestino, padre);
            }
        } catch (...) {
            delete[] visitados;
            delete[] padre;
            throw;
        }
        delete[] visitados;
        delete[] padre;
        return ruta;
    }

    ResultadoRutaSede buscarSedeCercanaConRecursoBFS(
        int idOrigen,
        const std::string& codigoRecurso,
        const MatrizDisponibilidad& matriz,
        int dia = -1,
        int maxDistanciaMetros = 10000
    ) const {
        ResultadoRutaSede res;
        if (idOrigen < 0 || idOrigen >= numSedes || codigoRecurso.empty()) {
            std::cout << "[WARN] [GrafoSedes] Parámetros inválidos en buscarSedeCercanaConRecursoBFS." << std::endl;
            return res;
        }
        std::cout << "[DEBUG] [GrafoSedes] Buscando '" << codigoRecurso
                  << "' en sedes cercanas vía BFS desde sede " << idOrigen << "..." << std::endl;

        bool* visitados = new bool[numSedes];
        int* padre = new int[numSedes];
        int* saltos = new int[numSedes];
        int* distMetros = new int[numSedes];
        try {
            for (int i = 0; i < numSedes; ++i) {
                visitados[i] = false;
                padre[i] = -1;
                saltos[i] = -1;
                distMetros[i] = 0;
            }
            Cola<int> colaBFS;
            visitados[idOrigen] = true;
            saltos[idOrigen] = 0;
            colaBFS.encolar(idOrigen);

            while (!colaBFS.estaVacia()) {
                int actual = -1;
                colaBFS.desencolar(actual);
                if (actual != idOrigen) {
                    bool disponibleAqui = (dia >= 0 && dia < MatrizDisponibilidad::DIAS_SEMANA)
                        ? matriz.estaDisponible(codigoRecurso, actual, dia)
                        : matriz.tieneDisponibilidadEnSede(codigoRecurso, actual);
                    if (disponibleAqui) {
                        res.encontrada = true;
                        res.sedeDestinoId = actual;
                        res.nombreSedeDestino = vertices[actual].nombre;
                        res.saltos = saltos[actual];
                        res.distanciaTotalMetros = distMetros[actual];
                        res.esViable = (maxDistanciaMetros <= 0) || (distMetros[actual] <= maxDistanciaMetros);
                        res.rutaSedes = reconstruirRuta(idOrigen, actual, padre);
                        if (!res.esViable) {
                            std::cout << "[WARN] [GrafoSedes] Hallado en '" << res.nombreSedeDestino
                                      << "' (" << res.distanciaTotalMetros
                                      << " m), supera umbral de viabilidad (" << maxDistanciaMetros << " m)." << std::endl;
                        }
                        break;
                    }
                }
                const NodoSimple<AristaSede>* arista = vertices[actual].adyacentes.getCabeza();
                while (arista != nullptr) {
                    int vecino = arista->dato.destinoId;
                    if (vecino >= 0 && vecino < numSedes && !visitados[vecino]) {
                        visitados[vecino] = true;
                        padre[vecino] = actual;
                        saltos[vecino] = saltos[actual] + 1;
                        distMetros[vecino] = distMetros[actual] + arista->dato.distanciaMetros;
                        colaBFS.encolar(vecino);
                    }
                    arista = arista->siguiente;
                }
            }
        } catch (...) {
            delete[] visitados;
            delete[] padre;
            delete[] saltos;
            delete[] distMetros;
            throw;
        }
        delete[] visitados;
        delete[] padre;
        delete[] saltos;
        delete[] distMetros;
        return res;
    }

    VectorDinamico<std::string> explorarConectividadDFS(int idOrigen) const {
        VectorDinamico<std::string> recorrido;
        if (idOrigen < 0 || idOrigen >= numSedes) {
            std::cout << "[WARN] [GrafoSedes] Origen fuera de rango en DFS." << std::endl;
            return recorrido;
        }
        bool* visitados = new bool[numSedes];
        try {
            for (int i = 0; i < numSedes; ++i) visitados[i] = false;
            dfsRec(idOrigen, visitados, recorrido);
        } catch (...) {
            delete[] visitados;
            throw;
        }
        delete[] visitados;
        return recorrido;
    }

    bool esRedConexa() const {
        if (numSedes <= 1) return true;
        return explorarConectividadDFS(0).getTamano() == numSedes;
    }

    int getNumSedes() const { return numSedes; }
    std::string getNombreSede(int id) const { return (id >= 0 && id < numSedes) ? vertices[id].nombre : ""; }
    std::string getDireccionSede(int id) const { return (id >= 0 && id < numSedes) ? vertices[id].direccion : ""; }
    VectorDinamico<AristaSede> getAdyacentes(int id) const {
        if (id < 0 || id >= numSedes) return VectorDinamico<AristaSede>();
        return vertices[id].adyacentes.aVector();
    }
};

#endif // GRAFO_SEDES_H
