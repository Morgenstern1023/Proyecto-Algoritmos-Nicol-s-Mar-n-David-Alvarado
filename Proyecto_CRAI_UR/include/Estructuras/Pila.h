/**
 * @file Pila.h
 * @brief Estructura LIFO genérica basada en nodos propios para deshacer operaciones (RQF 009).
 * @author Nicolás Marín Quintero (NM)
 */

#ifndef PILA_H
#define PILA_H

#include "Nodo.h"

/**
 * @brief Pila LIFO genérica con Regla de los 3.
 * @tparam T Tipo de dato almacenado.
 */
template <typename T>
class Pila {
private:
    NodoSimple<T>* cima;
    int tamano;

    void limpiar() {
        while (cima != nullptr) {
            NodoSimple<T>* temp = cima;
            cima = cima->siguiente;
            delete temp;
        }
        tamano = 0;
    }

    void copiarDesde(const Pila<T>& otra) {
        if (otra.cima == nullptr) return;
        int count = otra.tamano;
        T* tempArr = new T[count];
        NodoSimple<T>* actual = otra.cima;
        int idx = count - 1;
        while (actual != nullptr && idx >= 0) {
            tempArr[idx--] = actual->dato;
            actual = actual->siguiente;
        }
        try {
            for (int i = 0; i < count; ++i) {
                push(tempArr[i]);
            }
        } catch (...) {
            delete[] tempArr;
            throw;
        }
        delete[] tempArr;
    }

public:
    Pila() : cima(nullptr), tamano(0) {}

    Pila(const Pila<T>& otra) : cima(nullptr), tamano(0) {
        copiarDesde(otra);
    }

    Pila<T>& operator=(const Pila<T>& otra) {
        if (this == &otra) {
            return *this;
        }
        limpiar();
        copiarDesde(otra);
        return *this;
    }

    ~Pila() {
        limpiar();
    }

    /**
     * @brief Apila un elemento en la cima en O(1).
     */
    void push(const T& valor) {
        NodoSimple<T>* nuevo = new NodoSimple<T>(valor);
        nuevo->siguiente = cima;
        cima = nuevo;
        tamano++;
    }

    /**
     * @brief Desapila el elemento superior en O(1).
     */
    bool pop(T& salida) {
        if (cima == nullptr) return false;
        NodoSimple<T>* temp = cima;
        salida = temp->dato;
        cima = cima->siguiente;
        delete temp;
        tamano--;
        return true;
    }

    bool peek(T& salida) const {
        if (cima == nullptr) return false;
        salida = cima->dato;
        return true;
    }

    bool estaVacia() const { return cima == nullptr; }
    int getTamano() const { return tamano; }
};

#endif // PILA_H
