/**
 * @file Cola.h
 * @brief Estructura FIFO genérica basada en nodos propios para listas de espera (RQF 008).
 * @author Nicolás Marín Quintero (NM)
 */

#ifndef COLA_H
#define COLA_H

#include "Nodo.h"

/**
 * @brief Cola FIFO genérica con punteros frente/finalCola y Regla de los 3.
 * @tparam T Tipo de dato almacenado.
 */
template <typename T>
class Cola {
private:
    NodoSimple<T>* frente;
    NodoSimple<T>* finalCola;
    int tamano;

    void limpiar() {
        while (frente != nullptr) {
            NodoSimple<T>* temp = frente;
            frente = frente->siguiente;
            delete temp;
        }
        frente = nullptr;
        finalCola = nullptr;
        tamano = 0;
    }

public:
    Cola() : frente(nullptr), finalCola(nullptr), tamano(0) {}

    Cola(const Cola<T>& otra) : frente(nullptr), finalCola(nullptr), tamano(0) {
        NodoSimple<T>* actual = otra.frente;
        while (actual != nullptr) {
            encolar(actual->dato);
            actual = actual->siguiente;
        }
    }

    Cola<T>& operator=(const Cola<T>& otra) {
        if (this == &otra) {
            return *this;
        }
        limpiar();
        NodoSimple<T>* actual = otra.frente;
        while (actual != nullptr) {
            encolar(actual->dato);
            actual = actual->siguiente;
        }
        return *this;
    }

    ~Cola() {
        limpiar();
    }

    /**
     * @brief Encola al final en O(1).
     */
    void encolar(const T& valor) {
        NodoSimple<T>* nuevo = new NodoSimple<T>(valor);
        if (finalCola == nullptr) {
            frente = finalCola = nuevo;
        } else {
            finalCola->siguiente = nuevo;
            finalCola = nuevo;
        }
        tamano++;
    }

    /**
     * @brief Desencola del frente en O(1).
     */
    bool desencolar(T& salida) {
        if (frente == nullptr) return false;
        NodoSimple<T>* temp = frente;
        salida = temp->dato;
        frente = frente->siguiente;
        if (frente == nullptr) {
            finalCola = nullptr;
        }
        delete temp;
        tamano--;
        return true;
    }

    bool verFrente(T& salida) const {
        if (frente == nullptr) return false;
        salida = frente->dato;
        return true;
    }

    bool estaVacia() const { return frente == nullptr; }
    int getTamano() const { return tamano; }
};

#endif // COLA_H
