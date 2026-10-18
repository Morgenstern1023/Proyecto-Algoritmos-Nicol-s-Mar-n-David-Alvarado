#include <iostream>
#include <string>
using namespace std;

class ColaListadeEspera {
    struct Nodo {
        string documentoUsuario;
        string codigoRecurso;
        Nodo* siguiente;
    };
    Nodo* frente;
    Nodo* cola;

public:
    ColaListadeEspera() : frente(nullptr), cola(nullptr) {}

    Nodo* getFrente() {
        return frente;
    }

    void encolar(string documentoUsuario, string codigoRecurso) {
        Nodo* nuevoNodo = new Nodo{documentoUsuario, codigoRecurso, nullptr};
        if (cola != nullptr) {
            cola->siguiente = nuevoNodo;
        }
        cola = nuevoNodo;
        if (frente == nullptr) {
            frente = nuevoNodo;
        }
    }

    void desencolar() {
        if (frente != nullptr) {
            Nodo* temp = frente;
            frente = frente->siguiente;
            if (frente == nullptr) {
                cola = nullptr;
            }
            delete temp;
        }
    }

    bool estaVacia() {
        return frente == nullptr;
    }

    void mostrarCola() {
        Nodo* actual = frente;
        while (actual != nullptr) {
            cout << "Documento: " << actual->documentoUsuario << ", Recurso: " << actual->codigoRecurso << endl;
            actual = actual->siguiente;
        }
    }
};

ColaListadeEspera cola;
    