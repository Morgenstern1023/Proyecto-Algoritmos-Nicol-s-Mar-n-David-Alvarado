#pragma once
#include <iostream>
#include <string>
#include "Disponibilidad.h"

using namespace std;

class NodoDoble {
public:
    // Variable para almacenar la acción o información del nodo
    string accion;
    /*Punteros para el siguiente y anterior nodo*/ 
    NodoDoble* next;
    NodoDoble* prev;
    NodoDoble(string act);
};

class ListaDobleHistorial {
private:
    struct NodoSnapshot {
        MatrizDisponibilidad::SnapshotEstado data;
        NodoSnapshot* siguiente;

        explicit NodoSnapshot(const MatrizDisponibilidad::SnapshotEstado& estado);
        ~NodoSnapshot(); 
    };

    // Punteros para el primer y último nodo de la lista
    //head: primer elemento
    NodoDoble* head;
    //tail: último elemento
    NodoDoble* tail;
    NodoSnapshot* cima;
    MatrizDisponibilidad* matrizActual;

public:
    // Constructor y destructor de la lista 
    ListaDobleHistorial();
    ~ListaDobleHistorial();
    // Métodos para agregar acciones y navegar por la lista
    void agregarAccion(string accion);
    void navegarAdelante();
    void navegarAtras();

    void enlazarMatriz(MatrizDisponibilidad* matriz);
    void guardarSnapshotActual();
    bool deshacerUltimoCambio();
    bool haySnapshots() const;

private:
    // Métodos auxiliares para la navegación recursiva
    void navegarAdelante(NodoDoble* temp);
    void navegarAtras(NodoDoble* temp);
    void pushSnapshot(const MatrizDisponibilidad::SnapshotEstado& estado);
    bool popSnapshot(MatrizDisponibilidad& matriz);
};