/**
 * @file ListaDoble.h
 * @brief Lista doblemente enlazada genérica con recorridos recursivos adelante y atrás (RQF 007).
 * @author Oscar David Alvarado Navarrete (DA)
 */

#ifndef LISTA_DOBLE_H
#define LISTA_DOBLE_H

#include "Nodo.h"
#include "VectorDinamico.h"

/**
 * @brief Lista doble genérica con punteros head/tail y Regla de los 3.
 * @tparam T Tipo de dato almacenado.
 */
template <typename T>
class ListaDoble {
private:
    NodoDoble<T>* head;
    NodoDoble<T>* tail;
    int tamano;

    void recolectarAdelanteRec(const NodoDoble<T>* actual, VectorDinamico<T>& vec) const {
        if (actual == nullptr) return;
        vec.push_back(actual->dato);
        recolectarAdelanteRec(actual->siguiente, vec);
    }

    void recolectarAtrasRec(const NodoDoble<T>* actual, VectorDinamico<T>& vec) const {
        if (actual == nullptr) return;
        vec.push_back(actual->dato);
        recolectarAtrasRec(actual->anterior, vec);
    }

    void limpiar() {
        NodoDoble<T>* actual = head;
        while (actual != nullptr) {
            NodoDoble<T>* sig = actual->siguiente;
            delete actual;
            actual = sig;
        }
        head = nullptr;
        tail = nullptr;
        tamano = 0;
    }

public:
    ListaDoble() : head(nullptr), tail(nullptr), tamano(0) {}

    ListaDoble(const ListaDoble<T>& otra) : head(nullptr), tail(nullptr), tamano(0) {
        NodoDoble<T>* actual = otra.head;
        while (actual != nullptr) {
            insertarFinal(actual->dato);
            actual = actual->siguiente;
        }
    }

    ListaDoble<T>& operator=(const ListaDoble<T>& otra) {
        if (this == &otra) {
            return *this;
        }
        limpiar();
        NodoDoble<T>* actual = otra.head;
        while (actual != nullptr) {
            insertarFinal(actual->dato);
            actual = actual->siguiente;
        }
        return *this;
    }

    ~ListaDoble() {
        limpiar();
    }

    /**
     * @brief Inserta al final en O(1).
     */
    void insertarFinal(const T& valor) {
        NodoDoble<T>* nuevo = new NodoDoble<T>(valor);
        if (head == nullptr) {
            head = tail = nuevo;
        } else {
            tail->siguiente = nuevo;
            nuevo->anterior = tail;
            tail = nuevo;
        }
        tamano++;
    }

    /**
     * @brief Elimina el último nodo en O(1).
     */
    bool eliminarUltimo() {
        if (tail == nullptr) {
            return false;
        }
        NodoDoble<T>* temp = tail;
        if (head == tail) {
            head = tail = nullptr;
        } else {
            tail = tail->anterior;
            tail->siguiente = nullptr;
        }
        delete temp;
        tamano--;
        return true;
    }

    /**
     * @brief Recorrido recursivo de head hacia tail.
     * @note Complejidad Temporal: \Theta(m). Espacial: O(m) pila de recursión.
     */
    VectorDinamico<T> obtenerAdelante() const {
        VectorDinamico<T> vec(tamano > 0 ? tamano : 4);
        recolectarAdelanteRec(head, vec);
        return vec;
    }

    /**
     * @brief Recorrido recursivo de tail hacia head.
     * @note Complejidad Temporal: \Theta(m). Espacial: O(m) pila de recursión.
     */
    VectorDinamico<T> obtenerAtras() const {
        VectorDinamico<T> vec(tamano > 0 ? tamano : 4);
        recolectarAtrasRec(tail, vec);
        return vec;
    }

    int getTamano() const { return tamano; }
    bool estaVacia() const { return head == nullptr; }
};

#endif // LISTA_DOBLE_H
