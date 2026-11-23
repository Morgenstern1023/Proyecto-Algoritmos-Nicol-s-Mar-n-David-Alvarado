/**
 * @file TablaHash.h
 * @brief Tabla Hash propia con polinomio de Horner (p=31, M=1009) y encadenamiento separado (RQF 010, 019).
 * @author Oscar David Alvarado Navarrete (DA) & Nicolás Marín Quintero (NM)
 */

#ifndef TABLA_HASH_H
#define TABLA_HASH_H

#include "VectorDinamico.h"
#include <iostream>
#include <string>

/**
 * @brief Tabla Hash genérica indexada por std::string con búsqueda O(1) promedio y Regla de los 3.
 * @tparam V Tipo de dato almacenado.
 */
template <typename V>
class TablaHash {
private:
    struct NodoHash {
        std::string clave;
        V valor;
        NodoHash* siguiente;

        NodoHash(const std::string& k, const V& v)
            : clave(k), valor(v), siguiente(nullptr) {}
    };

    NodoHash** cubetas;
    int capacidad;
    int numElementos;
    int totalColisiones;

    /**
     * @brief Dispersión polinomial de Horner sobre capacidad prima M.
     * @note Complejidad Temporal: \Theta(L) con L acotada.
     */
    int funcionHorner(const std::string& clave) const {
        unsigned long long h = 0;
        const unsigned long long BASE_PRIMA = 31ULL;
        for (size_t i = 0; i < clave.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(clave[i]);
            h = (h * BASE_PRIMA + c) % static_cast<unsigned long long>(capacidad);
        }
        return static_cast<int>(h);
    }

    void liberarTodo() {
        if (cubetas != nullptr) {
            for (int i = 0; i < capacidad; ++i) {
                NodoHash* actual = cubetas[i];
                while (actual != nullptr) {
                    NodoHash* sig = actual->siguiente;
                    delete actual;
                    actual = sig;
                }
            }
            delete[] cubetas;
            cubetas = nullptr;
        }
        numElementos = 0;
        totalColisiones = 0;
    }

    void copiarDesde(const TablaHash<V>& otra) {
        capacidad = otra.capacidad;
        numElementos = otra.numElementos;
        totalColisiones = otra.totalColisiones;
        cubetas = new NodoHash*[capacidad];

        for (int i = 0; i < capacidad; ++i) {
            cubetas[i] = nullptr;
            NodoHash* actualOtro = otra.cubetas[i];
            NodoHash* colaLocal = nullptr;
            while (actualOtro != nullptr) {
                NodoHash* copia = new NodoHash(actualOtro->clave, actualOtro->valor);
                if (cubetas[i] == nullptr) cubetas[i] = copia;
                else colaLocal->siguiente = copia;
                colaLocal = copia;
                actualOtro = actualOtro->siguiente;
            }
        }
    }

public:
    explicit TablaHash(int capPrimo = 1009)
        : cubetas(nullptr), capacidad(capPrimo), numElementos(0), totalColisiones(0) {
        if (capacidad <= 0) {
            std::cout << "[WARN] [TablaHash] Capacidad inválida (" << capPrimo
                      << "); se ajusta a primo 1009." << std::endl;
            capacidad = 1009;
        }
        cubetas = new NodoHash*[capacidad];
        for (int i = 0; i < capacidad; ++i) {
            cubetas[i] = nullptr;
        }
    }

    TablaHash(const TablaHash<V>& otra)
        : cubetas(nullptr), capacidad(0), numElementos(0), totalColisiones(0) {
        copiarDesde(otra);
    }

    TablaHash<V>& operator=(const TablaHash<V>& otra) {
        if (this == &otra) {
            return *this;
        }
        liberarTodo();
        copiarDesde(otra);
        return *this;
    }

    ~TablaHash() {
        liberarTodo();
    }

    /**
     * @brief Inserta o actualiza un par clave-valor.
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(1 + \alpha), Peor O(n).
     */
    void insertar(const std::string& clave, const V& valor) {
        if (clave.empty()) {
            std::cout << "[WARN] [TablaHash] Intento de insertar clave vacía ignorado." << std::endl;
            return;
        }
        int indice = funcionHorner(clave);
        NodoHash* actual = cubetas[indice];
        while (actual != nullptr) {
            if (actual->clave == clave) {
                actual->valor = valor;
                return;
            }
            actual = actual->siguiente;
        }
        if (cubetas[indice] != nullptr) {
            totalColisiones++;
        }
        NodoHash* nuevo = new NodoHash(clave, valor);
        nuevo->siguiente = cubetas[indice];
        cubetas[indice] = nuevo;
        numElementos++;
    }

    /**
     * @brief Búsqueda inmediata en tiempo constante promedio.
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(1 + \alpha), Peor O(n).
     */
    bool buscar(const std::string& clave, V& salida) const {
        if (clave.empty()) return false;
        int indice = funcionHorner(clave);
        NodoHash* actual = cubetas[indice];
        while (actual != nullptr) {
            if (actual->clave == clave) {
                salida = actual->valor;
                return true;
            }
            actual = actual->siguiente;
        }
        return false;
    }

    bool existe(const std::string& clave) const {
        if (clave.empty()) return false;
        int indice = funcionHorner(clave);
        NodoHash* actual = cubetas[indice];
        while (actual != nullptr) {
            if (actual->clave == clave) return true;
            actual = actual->siguiente;
        }
        return false;
    }

    bool eliminar(const std::string& clave) {
        if (clave.empty()) return false;
        int indice = funcionHorner(clave);
        NodoHash* actual = cubetas[indice];
        NodoHash* anterior = nullptr;
        while (actual != nullptr) {
            if (actual->clave == clave) {
                if (anterior == nullptr) cubetas[indice] = actual->siguiente;
                else anterior->siguiente = actual->siguiente;
                delete actual;
                numElementos--;
                return true;
            }
            anterior = actual;
            actual = actual->siguiente;
        }
        return false;
    }

    VectorDinamico<V> obtenerTodosLosValores() const {
        VectorDinamico<V> res(numElementos > 0 ? numElementos : 4);
        for (int i = 0; i < capacidad; ++i) {
            NodoHash* actual = cubetas[i];
            while (actual != nullptr) {
                res.push_back(actual->valor);
                actual = actual->siguiente;
            }
        }
        return res;
    }

    double getFactorCarga() const {
        return static_cast<double>(numElementos) / static_cast<double>(capacidad);
    }
    int getTamano() const { return numElementos; }
    int getCapacidad() const { return capacidad; }
    int getTotalColisiones() const { return totalColisiones; }
};

#endif // TABLA_HASH_H
