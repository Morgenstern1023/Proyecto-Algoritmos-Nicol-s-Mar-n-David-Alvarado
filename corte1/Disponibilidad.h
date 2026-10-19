#pragma once
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

class MatrizDisponibilidad {
public:
    struct SnapshotEstado {
        string* codigos;
        string* nombresSedes;
        bool* matrizDisp;
        int totalRecursos;
        int totalSedes;
        int capacidadRecursos;
        int capacidadSedes;

        SnapshotEstado();
        SnapshotEstado(const SnapshotEstado& otro);
        SnapshotEstado& operator=(const SnapshotEstado& otro);
        ~SnapshotEstado();
    };

private:
    string* codigos;
    string* nombresSedes;
    bool* matrizDisp;
    int totalRecursos;
    int capacidadRecursos;
    int totalSedes;
    int capacidadSedes;

    int obtenerIndice(int recurso, int sede, int dia);
    void resizeRecursos();
    int buscarIndiceRecursoPorCodigo(string codigo);
    void liberarArreglos();
    void copiarDesdeSnapshot(SnapshotEstado& snapshot);

public:
    MatrizDisponibilidad(int capRecursosInicial = 2, int capSedesInicial = 3);
    ~MatrizDisponibilidad();

    bool existeRecurso(const string& codigo) const;
    SnapshotEstado capturarEstado() const;
    void restaurarEstado(SnapshotEstado& snapshot);

    void agregarSede(string nombreSede);
    void inicializarRecurso(string codigo);
    void reservarTurno(string codigo, int sede, int dia);
    void mostrarMatriz(string codigo);
};