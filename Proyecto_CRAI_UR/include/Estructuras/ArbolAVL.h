/**
 * @file ArbolAVL.h
 * @brief Árbol Binario de Búsqueda Autobalanceado (AVL) para índice ordenado en O(log n) (RQF 011).
 * @author Oscar David Alvarado Navarrete (DA) & Nicolás Marín Quintero (NM)
 */

#ifndef ARBOL_AVL_H
#define ARBOL_AVL_H

#include "VectorDinamico.h"

/**
 * @brief Árbol AVL genérico con rotaciones LL, RR, LR y RL y Regla de los 3.
 * @tparam K Clave ordenable.
 * @tparam V Valor asociado.
 */
template <typename K, typename V>
class ArbolAVL {
private:
    struct NodoAVL {
        K clave;
        V valor;
        NodoAVL* izquierdo;
        NodoAVL* derecho;
        int altura;

        NodoAVL(const K& k, const V& v)
            : clave(k), valor(v), izquierdo(nullptr), derecho(nullptr), altura(1) {}
    };

    NodoAVL* raiz;
    int tamano;

    int maxInt(int a, int b) const { return (a > b) ? a : b; }
    int obtenerAltura(const NodoAVL* n) const { return (n != nullptr) ? n->altura : 0; }
    int factorEquilibrio(const NodoAVL* n) const {
        return (n != nullptr) ? (obtenerAltura(n->izquierdo) - obtenerAltura(n->derecho)) : 0;
    }

    NodoAVL* rotarDerecha(NodoAVL* y) {
        NodoAVL* x = y->izquierdo;
        NodoAVL* T2 = x->derecho;
        x->derecho = y;
        y->izquierdo = T2;
        y->altura = 1 + maxInt(obtenerAltura(y->izquierdo), obtenerAltura(y->derecho));
        x->altura = 1 + maxInt(obtenerAltura(x->izquierdo), obtenerAltura(x->derecho));
        return x;
    }

    NodoAVL* rotarIzquierda(NodoAVL* x) {
        NodoAVL* y = x->derecho;
        NodoAVL* T2 = y->izquierdo;
        y->izquierdo = x;
        x->derecho = T2;
        x->altura = 1 + maxInt(obtenerAltura(x->izquierdo), obtenerAltura(x->derecho));
        y->altura = 1 + maxInt(obtenerAltura(y->izquierdo), obtenerAltura(y->derecho));
        return y;
    }

    NodoAVL* balancear(NodoAVL* nodo) {
        nodo->altura = 1 + maxInt(obtenerAltura(nodo->izquierdo), obtenerAltura(nodo->derecho));
        int fe = factorEquilibrio(nodo);

        if (fe > 1 && factorEquilibrio(nodo->izquierdo) >= 0) return rotarDerecha(nodo);
        if (fe > 1 && factorEquilibrio(nodo->izquierdo) < 0) {
            nodo->izquierdo = rotarIzquierda(nodo->izquierdo);
            return rotarDerecha(nodo);
        }
        if (fe < -1 && factorEquilibrio(nodo->derecho) <= 0) return rotarIzquierda(nodo);
        if (fe < -1 && factorEquilibrio(nodo->derecho) > 0) {
            nodo->derecho = rotarDerecha(nodo->derecho);
            return rotarIzquierda(nodo);
        }
        return nodo;
    }

    NodoAVL* insertarRec(NodoAVL* nodo, const K& clave, const V& valor) {
        if (nodo == nullptr) {
            tamano++;
            return new NodoAVL(clave, valor);
        }
        if (clave < nodo->clave) nodo->izquierdo = insertarRec(nodo->izquierdo, clave, valor);
        else if (clave > nodo->clave) nodo->derecho = insertarRec(nodo->derecho, clave, valor);
        else {
            nodo->valor = valor;
            return nodo;
        }
        return balancear(nodo);
    }

    NodoAVL* nodoMinimo(NodoAVL* nodo) const {
        NodoAVL* actual = nodo;
        while (actual != nullptr && actual->izquierdo != nullptr) actual = actual->izquierdo;
        return actual;
    }

    NodoAVL* eliminarRec(NodoAVL* nodo, const K& clave, bool& eliminado) {
        if (nodo == nullptr) return nullptr;
        if (clave < nodo->clave) nodo->izquierdo = eliminarRec(nodo->izquierdo, clave, eliminado);
        else if (clave > nodo->clave) nodo->derecho = eliminarRec(nodo->derecho, clave, eliminado);
        else {
            eliminado = true;
            if (nodo->izquierdo == nullptr || nodo->derecho == nullptr) {
                NodoAVL* hijo = (nodo->izquierdo != nullptr) ? nodo->izquierdo : nodo->derecho;
                delete nodo;
                tamano--;
                return hijo;
            }
            NodoAVL* sucesor = nodoMinimo(nodo->derecho);
            nodo->clave = sucesor->clave;
            nodo->valor = sucesor->valor;
            bool dummy = false;
            nodo->derecho = eliminarRec(nodo->derecho, sucesor->clave, dummy);
        }
        return balancear(nodo);
    }

    void inordenRec(const NodoAVL* nodo, VectorDinamico<V>& res) const {
        if (nodo == nullptr) return;
        inordenRec(nodo->izquierdo, res);
        res.push_back(nodo->valor);
        inordenRec(nodo->derecho, res);
    }

    void destruirRec(NodoAVL* nodo) {
        if (nodo == nullptr) return;
        destruirRec(nodo->izquierdo);
        destruirRec(nodo->derecho);
        delete nodo;
    }

    NodoAVL* clonarRec(const NodoAVL* nodo) const {
        if (nodo == nullptr) return nullptr;
        NodoAVL* copia = new NodoAVL(nodo->clave, nodo->valor);
        copia->altura = nodo->altura;
        copia->izquierdo = clonarRec(nodo->izquierdo);
        copia->derecho = clonarRec(nodo->derecho);
        return copia;
    }

public:
    ArbolAVL() : raiz(nullptr), tamano(0) {}

    ArbolAVL(const ArbolAVL<K, V>& otro) : raiz(nullptr), tamano(otro.tamano) {
        raiz = clonarRec(otro.raiz);
    }

    ArbolAVL<K, V>& operator=(const ArbolAVL<K, V>& otro) {
        if (this == &otro) return *this;
        destruirRec(raiz);
        raiz = clonarRec(otro.raiz);
        tamano = otro.tamano;
        return *this;
    }

    ~ArbolAVL() {
        destruirRec(raiz);
        raiz = nullptr;
        tamano = 0;
    }

    /**
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(\log n), Peor O(\log n).
     */
    void insertar(const K& clave, const V& valor) {
        raiz = insertarRec(raiz, clave, valor);
    }

    /**
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(\log n), Peor O(\log n).
     */
    bool buscar(const K& clave, V& salida) const {
        const NodoAVL* actual = raiz;
        while (actual != nullptr) {
            if (clave < actual->clave) actual = actual->izquierdo;
            else if (clave > actual->clave) actual = actual->derecho;
            else {
                salida = actual->valor;
                return true;
            }
        }
        return false;
    }

    /**
     * @note Complejidad Temporal: \Theta(\log n), Peor O(\log n).
     */
    bool eliminar(const K& clave) {
        bool eliminado = false;
        raiz = eliminarRec(raiz, clave, eliminado);
        return eliminado;
    }

    /**
     * @note Complejidad Temporal: \Theta(n).
     */
    VectorDinamico<V> recorridoInorden() const {
        VectorDinamico<V> res(tamano > 0 ? tamano : 4);
        inordenRec(raiz, res);
        return res;
    }

    int getAltura() const { return obtenerAltura(raiz); }
    int getTamano() const { return tamano; }
    bool estaVacio() const { return raiz == nullptr; }
};

#endif // ARBOL_AVL_H
