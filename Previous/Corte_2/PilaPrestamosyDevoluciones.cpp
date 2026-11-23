#include <iostream>
#include <string>

using namespace std;

// Clase que implementa una pila para registrar los prestamos y devoluciones de recursos por parte de los usuarios.

class PilaPrestamosyDevoluciones {
private:
    struct Nodo {
        string documentoUsuario;
        string codigoRecurso;
        string fechaHora; // Fecha y hora del prestamo o devolucion
        bool esPrestamo; // true si es prestamo, false si es devolucion
        Nodo* siguiente;
    };
    Nodo* cima;
public:
    PilaPrestamosyDevoluciones() : cima(nullptr) {}

    void push(string documentoUsuario, string codigoRecurso, string fechaHora, bool esPrestamo) {
        Nodo* nuevoNodo = new Nodo{documentoUsuario, codigoRecurso, fechaHora, esPrestamo, cima};
        cima = nuevoNodo;
    }

    void pop() {
        if (cima != nullptr) {
            Nodo* temp = cima;
            cima = cima->siguiente;
            delete temp;
        }
    }

    Nodo* top() {
        return cima;
    }

    bool estaVacia() {
        return cima == nullptr;
    }

    void mostrarHistorial() {
        Nodo* actual = cima;
        while (actual != nullptr) {
            cout << (actual->esPrestamo ? "Préstamo" : "Devolución") << " - Documento: " << actual->documentoUsuario << ", Recurso: " << actual->codigoRecurso << ", Fecha y hora: " << actual->fechaHora << endl;
            actual = actual->siguiente;
        }
    }

};

//Objeto global de la clase PilaPrestamosyDevoluciones
PilaPrestamosyDevoluciones pila;
