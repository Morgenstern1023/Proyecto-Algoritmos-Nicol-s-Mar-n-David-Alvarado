#pragma once
#include <iostream>
#include <string>

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
// Punteros para el primer y último nodo de la lista
    //head: primer elemento
    NodoDoble* head;
    //tail: último elemento
    NodoDoble* tail;

public:
    // Constructor y destructor de la lista 
    ListaDobleHistorial();
    ~ListaDobleHistorial();
    // Métodos para agregar acciones y navegar por la lista
    void agregarAccion(string accion);
    void navegarAdelante();
    void navegarAtras();
};