#pragma once
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

class MatrizDisponibilidad {
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

public:
    MatrizDisponibilidad(int capRecursosInicial = 2, int capSedesInicial = 3);
    ~MatrizDisponibilidad();

    void agregarSede(string nombreSede);
    void inicializarRecurso(string codigo);
    void reservarTurno(string codigo, int sede, int dia);
    void mostrarMatriz(string codigo);
};