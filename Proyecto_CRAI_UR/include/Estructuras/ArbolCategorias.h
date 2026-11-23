/**
 * @file ArbolCategorias.h
 * @brief Árbol N-ario LCRS con recursión/backtracking para categorías de profundidad arbitraria (RQF 012-013).
 * @author Nicolás Marín Quintero (NM)
 */

#ifndef ARBOL_CATEGORIAS_H
#define ARBOL_CATEGORIAS_H

#include "ListaSimple.h"
#include "VectorDinamico.h"
#include <iostream>
#include <string>

template <typename T>
class ArbolCategorias {
private:
    struct NodoCategoria {
        std::string nombre;
        ListaSimple<T> elementos;
        NodoCategoria* primerHijo;
        NodoCategoria* siguienteHermano;

        explicit NodoCategoria(const std::string& nom)
            : nombre(nom), elementos(), primerHijo(nullptr), siguienteHermano(nullptr) {}
    };

    NodoCategoria* raiz;

    NodoCategoria* buscarNodoRec(NodoCategoria* actual, const std::string& nombre) const {
        if (actual == nullptr) return nullptr;
        if (actual->nombre == nombre) return actual;
        NodoCategoria* enHijos = buscarNodoRec(actual->primerHijo, nombre);
        if (enHijos != nullptr) return enHijos;
        return buscarNodoRec(actual->siguienteHermano, nombre);
    }

    void recolectarSubarbolRec(const NodoCategoria* nodo, VectorDinamico<T>& acum) const {
        if (nodo == nullptr) return;
        const NodoSimple<T>* elem = nodo->elementos.getCabeza();
        while (elem != nullptr) {
            acum.push_back(elem->dato);
            elem = elem->siguiente;
        }
        const NodoCategoria* hijo = nodo->primerHijo;
        while (hijo != nullptr) {
            recolectarSubarbolRec(hijo, acum);
            hijo = hijo->siguienteHermano;
        }
    }

    void listarCategoriasRec(const NodoCategoria* nodo, int nivel, VectorDinamico<std::string>& salida) const {
        if (nodo == nullptr) return;
        std::string prefijo;
        for (int i = 0; i < nivel; ++i) prefijo += "  ";
        salida.push_back(prefijo + nodo->nombre);
        const NodoCategoria* hijo = nodo->primerHijo;
        while (hijo != nullptr) {
            listarCategoriasRec(hijo, nivel + 1, salida);
            hijo = hijo->siguienteHermano;
        }
    }

    void destruirRec(NodoCategoria* nodo) {
        if (nodo == nullptr) return;
        destruirRec(nodo->primerHijo);
        destruirRec(nodo->siguienteHermano);
        delete nodo;
    }

    NodoCategoria* clonarRec(const NodoCategoria* nodo) const {
        if (nodo == nullptr) return nullptr;
        NodoCategoria* copia = new NodoCategoria(nodo->nombre);
        copia->elementos = nodo->elementos;
        copia->primerHijo = clonarRec(nodo->primerHijo);
        copia->siguienteHermano = clonarRec(nodo->siguienteHermano);
        return copia;
    }

public:
    explicit ArbolCategorias(const std::string& nombreRaiz = "CRAI")
        : raiz(new NodoCategoria(nombreRaiz)) {}

    ArbolCategorias(const ArbolCategorias<T>& otro) : raiz(nullptr) {
        raiz = clonarRec(otro.raiz);
    }

    ArbolCategorias<T>& operator=(const ArbolCategorias<T>& otro) {
        if (this == &otro) return *this;
        destruirRec(raiz);
        raiz = clonarRec(otro.raiz);
        return *this;
    }

    ~ArbolCategorias() {
        destruirRec(raiz);
        raiz = nullptr;
    }

    bool existeCategoria(const std::string& nombreCategoria) const {
        return buscarNodoRec(raiz, nombreCategoria) != nullptr;
    }

    bool insertarSubcategoria(const std::string& nombrePadre, const std::string& nombreHija) {
        if (nombreHija.empty()) {
            std::cout << "[WARN] [ArbolCategorias] Nombre de subcategoría vacío." << std::endl;
            return false;
        }
        if (buscarNodoRec(raiz, nombreHija) != nullptr) {
            std::cout << "[WARN] [ArbolCategorias] Categoría duplicada rechazada: " << nombreHija << std::endl;
            return false;
        }
        NodoCategoria* padre = buscarNodoRec(raiz, nombrePadre);
        if (padre == nullptr) {
            std::cout << "[WARN] [ArbolCategorias] Categoría padre no encontrada: " << nombrePadre << std::endl;
            return false;
        }
        NodoCategoria* nueva = new NodoCategoria(nombreHija);
        if (padre->primerHijo == nullptr) {
            padre->primerHijo = nueva;
        } else {
            NodoCategoria* hermano = padre->primerHijo;
            while (hermano->siguienteHermano != nullptr) hermano = hermano->siguienteHermano;
            hermano->siguienteHermano = nueva;
        }
        return true;
    }

    bool asociarElemento(const std::string& nombreCategoria, const T& elemento) {
        NodoCategoria* cat = buscarNodoRec(raiz, nombreCategoria);
        if (cat == nullptr) {
            std::cout << "[WARN] [ArbolCategorias] Categoría inexistente al asociar: " << nombreCategoria << std::endl;
            return false;
        }
        if (!cat->elementos.contiene(elemento)) {
            cat->elementos.insertarFinal(elemento);
        }
        return true;
    }

    bool desasociarElemento(const std::string& nombreCategoria, const T& elemento) {
        NodoCategoria* cat = buscarNodoRec(raiz, nombreCategoria);
        if (cat == nullptr) return false;
        return cat->elementos.eliminar(elemento);
    }

    VectorDinamico<T> consultarTodoBajoCategoria(const std::string& nombreCategoria) const {
        VectorDinamico<T> resultado;
        NodoCategoria* objetivo = buscarNodoRec(raiz, nombreCategoria);
        if (objetivo == nullptr) {
            std::cout << "[WARN] [ArbolCategorias] Categoría consultada no existe: " << nombreCategoria << std::endl;
            return resultado;
        }
        recolectarSubarbolRec(objetivo, resultado);
        return resultado;
    }

    VectorDinamico<std::string> listarJerarquia() const {
        VectorDinamico<std::string> salida;
        listarCategoriasRec(raiz, 0, salida);
        return salida;
    }

    // Helper para persistencia: recolecta pares (categoria, padre) via callback
    template <typename Func>
    void recolectarParesPadreHijo(Func&& func) const {
        recolectarParesPadreHijoRec(raiz, std::forward<Func>(func));
    }

private:
    template <typename Func>
    void recolectarParesPadreHijoRec(const NodoCategoria* nodo, Func&& func) const {
        if (nodo == nullptr) return;
        const NodoCategoria* hijo = nodo->primerHijo;
        while (hijo != nullptr) {
            func(hijo->nombre, nodo->nombre);
            recolectarParesPadreHijoRec(hijo, std::forward<Func>(func));
            hijo = hijo->siguienteHermano;
        }
    }
};

#endif // ARBOL_CATEGORIAS_H
