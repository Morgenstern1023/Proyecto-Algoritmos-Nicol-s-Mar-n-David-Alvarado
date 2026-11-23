/**
 * @file ListaSimple.h
 * @brief Lista enlazada simple genérica para préstamos activos, adyacencias y cubetas.
 * @author Nicolás Marín Quintero (NM)
 */

#ifndef LISTA_SIMPLE_H
#define LISTA_SIMPLE_H

#include "Nodo.h"
#include "VectorDinamico.h"
#include <iostream>

/**
 * @brief Lista simplemente enlazada con punteros cabeza/cola y Regla de los 3.
 * @tparam T Tipo de dato almacenado.
 */
template <typename T>
class ListaSimple {
private:
    NodoSimple<T>* cabeza;
    NodoSimple<T>* cola;
    int tamano;

    void limpiar() {
        NodoSimple<T>* actual = cabeza;
        while (actual != nullptr) {
            NodoSimple<T>* siguiente = actual->siguiente;
            delete actual;
            actual = siguiente;
        }
        cabeza = nullptr;
        cola = nullptr;
        tamano = 0;
    }

public:
    ListaSimple() : cabeza(nullptr), cola(nullptr), tamano(0) {}

    ListaSimple(const ListaSimple<T>& otra) : cabeza(nullptr), cola(nullptr), tamano(0) {
        NodoSimple<T>* actual = otra.cabeza;
        while (actual != nullptr) {
            insertarFinal(actual->dato);
            actual = actual->siguiente;
        }
    }

    ListaSimple<T>& operator=(const ListaSimple<T>& otra) {
        if (this == &otra) {
            return *this;
        }
        limpiar();
        NodoSimple<T>* actual = otra.cabeza;
        while (actual != nullptr) {
            insertarFinal(actual->dato);
            actual = actual->siguiente;
        }
        return *this;
    }

    ~ListaSimple() {
        limpiar();
    }

    /**
     * @brief Inserta al final en O(1).
     */
    void insertarFinal(const T& valor) {
        NodoSimple<T>* nuevo = new NodoSimple<T>(valor);
        if (cabeza == nullptr) {
            cabeza = cola = nuevo;
        } else {
            cola->siguiente = nuevo;
            cola = nuevo;
        }
        tamano++;
    }

    /**
     * @brief Inserta al inicio en O(1).
     */
    void insertarInicio(const T& valor) {
        NodoSimple<T>* nuevo = new NodoSimple<T>(valor);
        nuevo->siguiente = cabeza;
        cabeza = nuevo;
        if (cola == nullptr) {
            cola = nuevo;
        }
        tamano++;
    }

    /**
     * @brief Elimina la primera coincidencia.
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(n), Peor O(n).
     */
    bool eliminar(const T& valor) {
        NodoSimple<T>* actual = cabeza;
        NodoSimple<T>* anterior = nullptr;
        while (actual != nullptr) {
            if (actual->dato == valor) {
                if (anterior == nullptr) cabeza = actual->siguiente;
                else anterior->siguiente = actual->siguiente;
                if (actual == cola) cola = anterior;
                delete actual;
                tamano--;
                return true;
            }
            anterior = actual;
            actual = actual->siguiente;
        }
        return false;
    }

    /**
     * @brief Elimina el primer elemento que satisfaga un predicado.
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(n), Peor O(n).
     */
    template <typename Predicado>
    bool eliminarSi(Predicado pred, T& eliminado) {
        NodoSimple<T>* actual = cabeza;
        NodoSimple<T>* anterior = nullptr;
        while (actual != nullptr) {
            if (pred(actual->dato)) {
                eliminado = actual->dato;
                if (anterior == nullptr) cabeza = actual->siguiente;
                else anterior->siguiente = actual->siguiente;
                if (actual == cola) cola = anterior;
                delete actual;
                tamano--;
                return true;
            }
            anterior = actual;
            actual = actual->siguiente;
        }
        return false;
    }

    bool contiene(const T& valor) const {
        NodoSimple<T>* actual = cabeza;
        while (actual != nullptr) {
            if (actual->dato == valor) return true;
            actual = actual->siguiente;
        }
        return false;
    }

    const NodoSimple<T>* getCabeza() const { return cabeza; }
    NodoSimple<T>* getCabeza() { return cabeza; }

    VectorDinamico<T> aVector() const {
        VectorDinamico<T> vec(tamano > 0 ? tamano : 4);
        NodoSimple<T>* actual = cabeza;
        while (actual != nullptr) {
            vec.push_back(actual->dato);
            actual = actual->siguiente;
        }
        return vec;
    }

    int getTamano() const { return tamano; }
    bool estaVacia() const { return cabeza == nullptr; }
};

#endif // LISTA_SIMPLE_H
