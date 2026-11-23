/**
 * @file VectorDinamico.h
 * @brief Vector dinámico genérico en memoria contigua con ordenamientos y búsqueda binaria.
 * @author Nicolás Marín Quintero (NM)
 */

#ifndef VECTOR_DINAMICO_H
#define VECTOR_DINAMICO_H

#include <iostream>
#include <stdexcept>

/**
 * @brief Vector dinámico con crecimiento geométrico amortizado O(1) y Regla de los 3.
 * @tparam T Tipo de dato almacenado.
 */
template <typename T>
class VectorDinamico {
private:
    T* datos;       ///< Arreglo dinámico en memoria contigua.
    int tamano;     ///< Cantidad actual de elementos.
    int capacidad;  ///< Capacidad máxima reservada.

    /**
     * @brief Duplica la capacidad del arreglo contiguo.
     * @note Complejidad Temporal: \Theta(n).
     */
    void redimensionar() {
        capacidad = (capacidad <= 0) ? 4 : capacidad * 2;
        T* nuevosDatos = new T[capacidad];
        for (int i = 0; i < tamano; ++i) {
            nuevosDatos[i] = datos[i];
        }
        delete[] datos;
        datos = nuevosDatos;
        std::cout << "[DEBUG] [VectorDinamico] Redimensionado a capacidad: " << capacidad << std::endl;
    }

    template <typename FuncComp>
    void mezclaMerge(int izquierda, int medio, int derecha, FuncComp comparar, T* temp) {
        int i = izquierda;
        int j = medio + 1;
        int k = izquierda;

        while (i <= medio && j <= derecha) {
            if (comparar(datos[i], datos[j]) || !comparar(datos[j], datos[i])) {
                temp[k++] = datos[i++];
            } else {
                temp[k++] = datos[j++];
            }
        }
        while (i <= medio) temp[k++] = datos[i++];
        while (j <= derecha) temp[k++] = datos[j++];
        for (int idx = izquierda; idx <= derecha; ++idx) {
            datos[idx] = temp[idx];
        }
    }

    template <typename FuncComp>
    void ordenarMergeRec(int izquierda, int derecha, FuncComp comparar, T* temp) {
        if (izquierda < derecha) {
            int medio = izquierda + (derecha - izquierda) / 2;
            ordenarMergeRec(izquierda, medio, comparar, temp);
            ordenarMergeRec(medio + 1, derecha, comparar, temp);
            mezclaMerge(izquierda, medio, derecha, comparar, temp);
        }
    }

    template <typename FuncComp>
    void heapify(int n, int i, FuncComp comparar) {
        int mayor = i;
        int izq = 2 * i + 1;
        int der = 2 * i + 2;

        if (izq < n && comparar(datos[mayor], datos[izq])) {
            mayor = izq;
        }
        if (der < n && comparar(datos[mayor], datos[der])) {
            mayor = der;
        }
        if (mayor != i) {
            T aux = datos[i];
            datos[i] = datos[mayor];
            datos[mayor] = aux;
            heapify(n, mayor, comparar);
        }
    }

public:
    explicit VectorDinamico(int capInicial = 4)
        : datos(nullptr), tamano(0), capacidad(capInicial > 0 ? capInicial : 4) {
        datos = new T[capacidad];
    }

    /**
     * @brief Constructor de Copia (Regla de los 3).
     */
    VectorDinamico(const VectorDinamico<T>& otro)
        : datos(nullptr), tamano(otro.tamano), capacidad(otro.capacidad) {
        datos = (capacidad > 0) ? new T[capacidad] : nullptr;
        for (int i = 0; i < tamano; ++i) {
            datos[i] = otro.datos[i];
        }
    }

    /**
     * @brief Operador de Asignación (Regla de los 3).
     */
    VectorDinamico<T>& operator=(const VectorDinamico<T>& otro) {
        if (this == &otro) {
            return *this;
        }
        delete[] datos;
        tamano = otro.tamano;
        capacidad = otro.capacidad;
        datos = (capacidad > 0) ? new T[capacidad] : nullptr;
        for (int i = 0; i < tamano; ++i) {
            datos[i] = otro.datos[i];
        }
        return *this;
    }

    /**
     * @brief Destructor (Regla de los 3).
     */
    ~VectorDinamico() {
        delete[] datos;
        datos = nullptr;
        tamano = 0;
        capacidad = 0;
    }

    /**
     * @brief Inserta un elemento al final.
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(1) amortizado, Peor O(n).
     */
    void push_back(const T& valor) {
        if (tamano >= capacidad) {
            redimensionar();
        }
        datos[tamano++] = valor;
    }

    /**
     * @brief Elimina el elemento en la posición indicada.
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(n), Peor O(n).
     */
    void eliminarEn(int indice) {
        if (indice < 0 || indice >= tamano) {
            std::cerr << "[ERROR] [VectorDinamico] Índice fuera de rango en eliminarEn("
                      << indice << "), tamaño=" << tamano << std::endl;
            throw std::out_of_range("[ERROR] Índice fuera de rango en VectorDinamico::eliminarEn");
        }
        for (int i = indice; i < tamano - 1; ++i) {
            datos[i] = datos[i + 1];
        }
        tamano--;
    }

    /**
     * @brief Acceso indexado mutable en O(1).
     */
    T& operator[](int indice) {
        if (indice < 0 || indice >= tamano) {
            std::cerr << "[ERROR] [VectorDinamico] Índice fuera de rango en operator[]("
                      << indice << "), tamaño=" << tamano << std::endl;
            throw std::out_of_range("[ERROR] Índice fuera de rango en VectorDinamico::operator[]");
        }
        return datos[indice];
    }

    /**
     * @brief Acceso indexado constante en O(1).
     */
    const T& operator[](int indice) const {
        if (indice < 0 || indice >= tamano) {
            std::cerr << "[ERROR] [VectorDinamico] Índice fuera de rango en operator[] const("
                      << indice << "), tamaño=" << tamano << std::endl;
            throw std::out_of_range("[ERROR] Índice fuera de rango en VectorDinamico::operator[] const");
        }
        return datos[indice];
    }

    int getTamano() const { return tamano; }
    int getCapacidad() const { return capacidad; }
    bool estaVacio() const { return tamano == 0; }

    /**
     * @brief Insertion Sort (Algoritmo básico).
     * @note Complejidad Temporal: Mejor \Omega(n), Promedio \Theta(n^2), Peor O(n^2). Espacial: O(1).
     */
    template <typename FuncComp>
    void insertionSort(FuncComp comparar) {
        for (int i = 1; i < tamano; ++i) {
            T clave = datos[i];
            int j = i - 1;
            while (j >= 0 && comparar(clave, datos[j])) {
                datos[j + 1] = datos[j];
                j--;
            }
            datos[j + 1] = clave;
        }
    }

    /**
     * @brief Merge Sort (Algoritmo avanzado estable).
     * @note Complejidad Temporal: \Omega(n \log n), \Theta(n \log n), O(n \log n). Espacial: O(n).
     */
    template <typename FuncComp>
    void mergeSort(FuncComp comparar) {
        if (tamano <= 1) return;
        T* temp = new T[tamano];
        try {
            ordenarMergeRec(0, tamano - 1, comparar, temp);
        } catch (...) {
            delete[] temp;
            throw;
        }
        delete[] temp;
    }

    /**
     * @brief Heap Sort (Algoritmo avanzado in-place).
     * @note Complejidad Temporal: \Omega(n \log n), \Theta(n \log n), O(n \log n). Espacial: O(\log n) pila.
     */
    template <typename FuncComp>
    void heapSort(FuncComp comparar) {
        if (tamano <= 1) return;
        for (int i = tamano / 2 - 1; i >= 0; --i) {
            heapify(tamano, i, comparar);
        }
        for (int i = tamano - 1; i > 0; --i) {
            T aux = datos[0];
            datos[0] = datos[i];
            datos[i] = aux;
            heapify(i, 0, comparar);
        }
    }

    /**
     * @brief Búsqueda Binaria iterativa sobre vector ordenado.
     * @note Complejidad Temporal: Mejor \Omega(1), Promedio \Theta(\log n), Peor O(\log n). Espacial: O(1).
     */
    template <typename FuncComp>
    int busquedaBinaria(const T& clave, FuncComp comparar) const {
        int izquierda = 0;
        int derecha = tamano - 1;
        while (izquierda <= derecha) {
            int medio = izquierda + (derecha - izquierda) / 2;
            if (!comparar(datos[medio], clave) && !comparar(clave, datos[medio])) {
                return medio;
            }
            if (comparar(datos[medio], clave)) {
                izquierda = medio + 1;
            } else {
                derecha = medio - 1;
            }
        }
        return -1;
    }
};

#endif // VECTOR_DINAMICO_H
