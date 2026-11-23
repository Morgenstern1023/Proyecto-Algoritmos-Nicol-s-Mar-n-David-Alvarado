/**
 * @file Nodo.h
 * @brief Definición de nodos enlazados genéricos para estructuras lineales.
 * @author Nicolás Marín Quintero (NM)
 */

#ifndef NODO_H
#define NODO_H

#include <iostream>

/**
 * @brief Nodo simple genérico para Listas Simples, Pilas y Colas.
 * @tparam T Tipo de dato almacenado en el nodo.
 */
template <typename T>
class NodoSimple {
public:
    T dato;                   ///< Valor almacenado en el nodo.
    NodoSimple<T>* siguiente; ///< Puntero al siguiente nodo.

    explicit NodoSimple(const T& val = T()) : dato(val), siguiente(nullptr) {}

    ~NodoSimple() {
        siguiente = nullptr;
    }
};

/**
 * @brief Nodo doblemente enlazado genérico para Listas Dobles e Historiales.
 * @tparam T Tipo de dato almacenado en el nodo.
 */
template <typename T>
class NodoDoble {
public:
    T dato;                  ///< Valor almacenado en el nodo.
    NodoDoble<T>* siguiente; ///< Puntero al nodo siguiente.
    NodoDoble<T>* anterior;  ///< Puntero al nodo anterior.

    explicit NodoDoble(const T& val = T())
        : dato(val), siguiente(nullptr), anterior(nullptr) {}

    ~NodoDoble() {
        siguiente = nullptr;
        anterior = nullptr;
    }
};

#endif // NODO_H
