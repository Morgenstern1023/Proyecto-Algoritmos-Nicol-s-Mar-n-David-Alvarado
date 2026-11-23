/**
 * @file MatrizDisponibilidad.h
 * @brief Matriz 3D contigua (recurso x sede x día) indexada en O(1) con TablaHash propia y Snapshots (RQF 004, 009).
 * @author Oscar David Alvarado Navarrete (DA)
 */

#ifndef MATRIZ_DISPONIBILIDAD_H
#define MATRIZ_DISPONIBILIDAD_H

#include "TablaHash.h"
#include <iostream>
#include <string>

class MatrizDisponibilidad {
public:
    static const int DIAS_SEMANA = 7;

    struct SnapshotEstado {
        std::string* codigos;
        std::string* nombresSedes;
        bool* matrizDisp;
        TablaHash<int>* indicePorCodigo;
        int totalRecursos;
        int totalSedes;
        int capacidadRecursos;
        int capacidadSedes;

        SnapshotEstado()
            : codigos(nullptr), nombresSedes(nullptr), matrizDisp(nullptr),
              indicePorCodigo(nullptr), totalRecursos(0), totalSedes(0),
              capacidadRecursos(0), capacidadSedes(0) {}

        SnapshotEstado(const SnapshotEstado& otro)
            : codigos(nullptr), nombresSedes(nullptr), matrizDisp(nullptr),
              indicePorCodigo(nullptr), totalRecursos(otro.totalRecursos),
              totalSedes(otro.totalSedes), capacidadRecursos(otro.capacidadRecursos),
              capacidadSedes(otro.capacidadSedes) {
            if (otro.codigos != nullptr && capacidadRecursos > 0) {
                codigos = new std::string[capacidadRecursos];
                for (int i = 0; i < capacidadRecursos; ++i) codigos[i] = otro.codigos[i];
            }
            if (otro.nombresSedes != nullptr && capacidadSedes > 0) {
                nombresSedes = new std::string[capacidadSedes];
                for (int i = 0; i < capacidadSedes; ++i) nombresSedes[i] = otro.nombresSedes[i];
            }
            if (otro.matrizDisp != nullptr && capacidadRecursos > 0 && capacidadSedes > 0) {
                int total = capacidadRecursos * capacidadSedes * DIAS_SEMANA;
                matrizDisp = new bool[total];
                for (int i = 0; i < total; ++i) matrizDisp[i] = otro.matrizDisp[i];
            }
            if (otro.indicePorCodigo != nullptr) {
                indicePorCodigo = new TablaHash<int>(*otro.indicePorCodigo);
            }
        }

        SnapshotEstado& operator=(const SnapshotEstado& otro) {
            if (this == &otro) return *this;
            delete[] codigos;
            delete[] nombresSedes;
            delete[] matrizDisp;
            delete indicePorCodigo;
            codigos = nullptr;
            nombresSedes = nullptr;
            matrizDisp = nullptr;
            indicePorCodigo = nullptr;

            totalRecursos = otro.totalRecursos;
            totalSedes = otro.totalSedes;
            capacidadRecursos = otro.capacidadRecursos;
            capacidadSedes = otro.capacidadSedes;

            if (otro.codigos != nullptr && capacidadRecursos > 0) {
                codigos = new std::string[capacidadRecursos];
                for (int i = 0; i < capacidadRecursos; ++i) codigos[i] = otro.codigos[i];
            }
            if (otro.nombresSedes != nullptr && capacidadSedes > 0) {
                nombresSedes = new std::string[capacidadSedes];
                for (int i = 0; i < capacidadSedes; ++i) nombresSedes[i] = otro.nombresSedes[i];
            }
            if (otro.matrizDisp != nullptr && capacidadRecursos > 0 && capacidadSedes > 0) {
                int total = capacidadRecursos * capacidadSedes * DIAS_SEMANA;
                matrizDisp = new bool[total];
                for (int i = 0; i < total; ++i) matrizDisp[i] = otro.matrizDisp[i];
            }
            if (otro.indicePorCodigo != nullptr) {
                indicePorCodigo = new TablaHash<int>(*otro.indicePorCodigo);
            }
            return *this;
        }

        ~SnapshotEstado() {
            delete[] codigos;
            delete[] nombresSedes;
            delete[] matrizDisp;
            delete indicePorCodigo;
            codigos = nullptr;
            nombresSedes = nullptr;
            matrizDisp = nullptr;
            indicePorCodigo = nullptr;
        }

        bool esValido() const {
            return codigos != nullptr && nombresSedes != nullptr &&
                   matrizDisp != nullptr && indicePorCodigo != nullptr;
        }
    };

private:
    std::string* codigos;
    std::string* nombresSedes;
    bool* matrizDisp;
    TablaHash<int>* indicePorCodigo;
    int totalRecursos;
    int capacidadRecursos;
    int totalSedes;
    int capacidadSedes;

    int obtenerIndice(int recurso, int sede, int dia) const {
        return (recurso * capacidadSedes * DIAS_SEMANA) + (sede * DIAS_SEMANA) + dia;
    }

    void liberarArreglos() {
        delete[] codigos;
        delete[] nombresSedes;
        delete[] matrizDisp;
        delete indicePorCodigo;
        codigos = nullptr;
        nombresSedes = nullptr;
        matrizDisp = nullptr;
        indicePorCodigo = nullptr;
    }

    void copiarDesde(const MatrizDisponibilidad& otra) {
        totalRecursos = otra.totalRecursos;
        capacidadRecursos = otra.capacidadRecursos;
        totalSedes = otra.totalSedes;
        capacidadSedes = otra.capacidadSedes;

        codigos = new std::string[capacidadRecursos];
        for (int i = 0; i < capacidadRecursos; ++i) codigos[i] = otra.codigos[i];
        nombresSedes = new std::string[capacidadSedes];
        for (int i = 0; i < capacidadSedes; ++i) nombresSedes[i] = otra.nombresSedes[i];
        int total = capacidadRecursos * capacidadSedes * DIAS_SEMANA;
        matrizDisp = new bool[total];
        for (int i = 0; i < total; ++i) matrizDisp[i] = otra.matrizDisp[i];
        indicePorCodigo = (otra.indicePorCodigo != nullptr)
            ? new TablaHash<int>(*otra.indicePorCodigo)
            : new TablaHash<int>(1009);
    }

    void redimensionarRecursos() {
        int nuevaCap = capacidadRecursos * 2;
        std::string* nuevosCodigos = new std::string[nuevaCap];
        int totalNuevas = nuevaCap * capacidadSedes * DIAS_SEMANA;
        bool* nuevaMatriz = new bool[totalNuevas];
        for (int i = 0; i < totalNuevas; ++i) nuevaMatriz[i] = true;
        for (int i = 0; i < totalRecursos; ++i) nuevosCodigos[i] = codigos[i];
        for (int r = 0; r < totalRecursos; ++r)
            for (int s = 0; s < totalSedes; ++s)
                for (int d = 0; d < DIAS_SEMANA; ++d)
                    nuevaMatriz[obtenerIndice(r, s, d)] = matrizDisp[obtenerIndice(r, s, d)];

        delete[] codigos;
        delete[] matrizDisp;
        codigos = nuevosCodigos;
        matrizDisp = nuevaMatriz;
        capacidadRecursos = nuevaCap;
        std::cout << "[DEBUG] [MatrizDisponibilidad] Capacidad de recursos ampliada a: "
                  << capacidadRecursos << std::endl;
    }

    void redimensionarSedes() {
        int nuevaCapSedes = capacidadSedes * 2;
        std::string* nuevasSedes = new std::string[nuevaCapSedes];
        int totalNuevas = capacidadRecursos * nuevaCapSedes * DIAS_SEMANA;
        bool* nuevaMatriz = new bool[totalNuevas];
        for (int i = 0; i < totalNuevas; ++i) nuevaMatriz[i] = true;
        for (int s = 0; s < totalSedes; ++s) nuevasSedes[s] = nombresSedes[s];
        for (int r = 0; r < totalRecursos; ++r) {
            for (int s = 0; s < totalSedes; ++s) {
                for (int d = 0; d < DIAS_SEMANA; ++d) {
                    int viejoIdx = (r * capacidadSedes * DIAS_SEMANA) + (s * DIAS_SEMANA) + d;
                    int nuevoIdx = (r * nuevaCapSedes * DIAS_SEMANA) + (s * DIAS_SEMANA) + d;
                    nuevaMatriz[nuevoIdx] = matrizDisp[viejoIdx];
                }
            }
        }
        delete[] nombresSedes;
        delete[] matrizDisp;
        nombresSedes = nuevasSedes;
        matrizDisp = nuevaMatriz;
        capacidadSedes = nuevaCapSedes;
    }

public:
    explicit MatrizDisponibilidad(int capRecInicial = 16, int capSedesInicial = 6)
        : codigos(nullptr), nombresSedes(nullptr), matrizDisp(nullptr),
          indicePorCodigo(nullptr), totalRecursos(0),
          capacidadRecursos(capRecInicial > 0 ? capRecInicial : 16),
          totalSedes(0), capacidadSedes(capSedesInicial > 0 ? capSedesInicial : 6) {
        codigos = new std::string[capacidadRecursos];
        nombresSedes = new std::string[capacidadSedes];
        int total = capacidadRecursos * capacidadSedes * DIAS_SEMANA;
        matrizDisp = new bool[total];
        for (int i = 0; i < total; ++i) matrizDisp[i] = true;
        indicePorCodigo = new TablaHash<int>(1009);
    }

    MatrizDisponibilidad(const MatrizDisponibilidad& otra)
        : codigos(nullptr), nombresSedes(nullptr), matrizDisp(nullptr),
          indicePorCodigo(nullptr), totalRecursos(0), capacidadRecursos(0),
          totalSedes(0), capacidadSedes(0) {
        copiarDesde(otra);
    }

    MatrizDisponibilidad& operator=(const MatrizDisponibilidad& otra) {
        if (this == &otra) return *this;
        liberarArreglos();
        copiarDesde(otra);
        return *this;
    }

    ~MatrizDisponibilidad() {
        liberarArreglos();
    }

    int buscarIndiceRecurso(const std::string& codigo) const {
        int idx = -1;
        if (indicePorCodigo != nullptr && indicePorCodigo->buscar(codigo, idx)) return idx;
        return -1;
    }

    bool existeRecurso(const std::string& codigo) const {
        return indicePorCodigo != nullptr && indicePorCodigo->existe(codigo);
    }

    int agregarSede(const std::string& nombreSede) {
        if (nombreSede.empty()) {
            std::cout << "[WARN] [MatrizDisponibilidad] Nombre de sede vacío rechazado." << std::endl;
            return -1;
        }
        for (int i = 0; i < totalSedes; ++i) {
            if (nombresSedes[i] == nombreSede) return i;
        }
        if (totalSedes >= capacidadSedes) redimensionarSedes();
        int id = totalSedes;
        nombresSedes[totalSedes++] = nombreSede;
        return id;
    }

    bool inicializarRecurso(const std::string& codigo) {
        if (codigo.empty() || existeRecurso(codigo)) return false;
        if (totalRecursos >= capacidadRecursos) redimensionarRecursos();
        int fila = totalRecursos;
        codigos[fila] = codigo;
        indicePorCodigo->insertar(codigo, fila);
        totalRecursos++;
        return true;
    }

    bool reservarTurno(const std::string& codigo, int sedeId, int dia) {
        int recIdx = buscarIndiceRecurso(codigo);
        if (recIdx == -1 || sedeId < 0 || sedeId >= totalSedes || dia < 0 || dia >= DIAS_SEMANA) {
            std::cout << "[WARN] [MatrizDisponibilidad] Parámetros inválidos en reservarTurno." << std::endl;
            return false;
        }
        int idx = obtenerIndice(recIdx, sedeId, dia);
        if (!matrizDisp[idx]) return false;
        matrizDisp[idx] = false;
        return true;
    }

    bool liberarTurno(const std::string& codigo, int sedeId, int dia) {
        int recIdx = buscarIndiceRecurso(codigo);
        if (recIdx == -1 || sedeId < 0 || sedeId >= totalSedes || dia < 0 || dia >= DIAS_SEMANA) {
            std::cout << "[WARN] [MatrizDisponibilidad] Parámetros inválidos en liberarTurno." << std::endl;
            return false;
        }
        matrizDisp[obtenerIndice(recIdx, sedeId, dia)] = true;
        return true;
    }

    bool estaDisponible(const std::string& codigo, int sedeId, int dia) const {
        int recIdx = buscarIndiceRecurso(codigo);
        if (recIdx == -1 || sedeId < 0 || sedeId >= totalSedes || dia < 0 || dia >= DIAS_SEMANA) return false;
        return matrizDisp[obtenerIndice(recIdx, sedeId, dia)];
    }

    bool tieneDisponibilidadEnSede(const std::string& codigo, int sedeId) const {
        int recIdx = buscarIndiceRecurso(codigo);
        if (recIdx == -1 || sedeId < 0 || sedeId >= totalSedes) return false;
        for (int d = 0; d < DIAS_SEMANA; ++d) {
            if (matrizDisp[obtenerIndice(recIdx, sedeId, d)]) return true;
        }
        return false;
    }

    void establecerDisponibilidadSede(const std::string& codigo, int sedeId, bool disponible) {
        int recIdx = buscarIndiceRecurso(codigo);
        if (recIdx == -1 || sedeId < 0 || sedeId >= totalSedes) return;
        for (int d = 0; d < DIAS_SEMANA; ++d) {
            matrizDisp[obtenerIndice(recIdx, sedeId, d)] = disponible;
        }
    }

    SnapshotEstado capturarSnapshot() const {
        SnapshotEstado snap;
        snap.totalRecursos = totalRecursos;
        snap.totalSedes = totalSedes;
        snap.capacidadRecursos = capacidadRecursos;
        snap.capacidadSedes = capacidadSedes;

        snap.codigos = new std::string[capacidadRecursos];
        for (int i = 0; i < capacidadRecursos; ++i) snap.codigos[i] = codigos[i];
        snap.nombresSedes = new std::string[capacidadSedes];
        for (int i = 0; i < capacidadSedes; ++i) snap.nombresSedes[i] = nombresSedes[i];
        int total = capacidadRecursos * capacidadSedes * DIAS_SEMANA;
        snap.matrizDisp = new bool[total];
        for (int i = 0; i < total; ++i) snap.matrizDisp[i] = matrizDisp[i];
        snap.indicePorCodigo = new TablaHash<int>(*indicePorCodigo);
        return snap;
    }

    bool restaurarSnapshot(SnapshotEstado& snap) {
        if (!snap.esValido()) {
            std::cerr << "[ERROR] [MatrizDisponibilidad] Snapshot inválido al restaurar." << std::endl;
            return false;
        }
        liberarArreglos();
        codigos = snap.codigos;
        nombresSedes = snap.nombresSedes;
        matrizDisp = snap.matrizDisp;
        indicePorCodigo = snap.indicePorCodigo;
        totalRecursos = snap.totalRecursos;
        totalSedes = snap.totalSedes;
        capacidadRecursos = snap.capacidadRecursos;
        capacidadSedes = snap.capacidadSedes;

        snap.codigos = nullptr;
        snap.nombresSedes = nullptr;
        snap.matrizDisp = nullptr;
        snap.indicePorCodigo = nullptr;
        snap.totalRecursos = 0;
        snap.totalSedes = 0;
        snap.capacidadRecursos = 0;
        snap.capacidadSedes = 0;
        return true;
    }

    int getTotalRecursos() const { return totalRecursos; }
    int getTotalSedes() const { return totalSedes; }
    std::string getNombreSede(int sedeId) const {
        if (sedeId < 0 || sedeId >= totalSedes) return "";
        return nombresSedes[sedeId];
    }

    // P1 Fix #2: Eliminar recurso de la matriz (limpiar fila fantasma al dar de baja)
    bool eliminarRecurso(const std::string& codigo) {
        int recIdx = buscarIndiceRecurso(codigo);
        if (recIdx == -1) return false;

        // Siempre eliminar la clave del recurso en la tabla hash (incluso si era el último)
        indicePorCodigo->eliminar(codigo);

        // Mover último recurso a la posición del eliminado (swap-and-pop para mantener contigüidad)
        if (recIdx < totalRecursos - 1) {
            codigos[recIdx] = codigos[totalRecursos - 1];
            indicePorCodigo->insertar(codigos[recIdx], recIdx);
            for (int s = 0; s < totalSedes; ++s) {
                for (int d = 0; d < DIAS_SEMANA; ++d) {
                    int srcIdx = obtenerIndice(totalRecursos - 1, s, d);
                    int dstIdx = obtenerIndice(recIdx, s, d);
                    matrizDisp[dstIdx] = matrizDisp[srcIdx];
                }
            }
        }
        // Limpiar última fila y código sobrante
        codigos[totalRecursos - 1].clear();
        for (int s = 0; s < totalSedes; ++s) {
            for (int d = 0; d < DIAS_SEMANA; ++d) {
                matrizDisp[obtenerIndice(totalRecursos - 1, s, d)] = true;
            }
        }
        totalRecursos--;
        return true;
    }
};

#endif // MATRIZ_DISPONIBILIDAD_H
