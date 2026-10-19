#include "Historial.h"
#include "Disponibilidad.h"

//los dos puntos es como cuando usamos std, y no usamos using namespace std; para no tener que escribir std::cout, std::string, etc.

/*  IMPORTANTE:
    COMPORTAMIENTO NODO:
    root -> next -> next -> next -> ... -> nullptr
    root <- prev <- prev <- prev <- ... <- nullptr
    root es el primer nodo, next apunta al siguiente nodo, prev apunta al nodo anterior,
    y nullptr indica el final de la lista (no hay más nodos).
*/

// Constructor
NodoDoble::NodoDoble(string act) {
    //this->accion(atributo) = act(parametro)
    this->accion = act;
    // Inicializar los punteros next y prev a nullptr
    this->next = nullptr;
    this->prev = nullptr;
}

ListaDobleHistorial::NodoSnapshot::NodoSnapshot(const MatrizDisponibilidad::SnapshotEstado& estado)
    : data(estado), siguiente(nullptr) {}

ListaDobleHistorial::NodoSnapshot::~NodoSnapshot() {
    // `data` destruye los arreglos deep-copy de este snapshot.
}

ListaDobleHistorial::ListaDobleHistorial() {
    // Inicializar los punteros head y tail a nullptr
    head = nullptr;
    tail = nullptr;
    cima = nullptr;
    matrizActual = nullptr;
}

ListaDobleHistorial::~ListaDobleHistorial() {
    // Liberar memoria de todos los nodos de la lista
    NodoDoble* actual = head;
    while (actual != nullptr) {
        NodoDoble* siguiente = actual->next;
        delete actual;
        actual = siguiente;
    }

    while (cima != nullptr) {
        NodoSnapshot* temp = cima;
        cima = cima->siguiente;
        delete temp;
    }
}

void ListaDobleHistorial::enlazarMatriz(MatrizDisponibilidad* matriz) {
    // Enlazar la matriz de disponibilidad actual con la lista de snapshots.
    matrizActual = matriz;
}

void ListaDobleHistorial::pushSnapshot(const MatrizDisponibilidad::SnapshotEstado& estado) {
    // crea nuevo snapshot 
    NodoSnapshot* nuevo = new NodoSnapshot(estado);
    // Enlazar el nuevo snapshot con la cima actual de la pila.
    nuevo->siguiente = cima;
    // actualiza 
    cima = nuevo;
}

bool ListaDobleHistorial::popSnapshot(MatrizDisponibilidad& matriz) {
    //revisa si hay snapshots disponibles antes de intentar hacer pop.
    if (cima == nullptr) {
        cout << "No hay snapshots para deshacer." << endl;
        return false;
    }

    NodoSnapshot* temp = cima;
    // Restaurar el estado de la matriz desde el snapshot en la cima de la pila.
    matriz.restaurarEstado(temp->data);
    // Actualiza cima de la pila al siguiente snapshot.
    cima = temp->siguiente;
    // Liberar memoria del snapshot que se acaba de usar.
    delete temp;
    return true;
}

void ListaDobleHistorial::guardarSnapshotActual() {
    // si no hay matriz enlazada, no se puede guardar el snapshot.
    if (matrizActual == nullptr) {
        cout << "No hay matriz enlazada para guardar el snapshot." << endl;
        return;
    }

    // Captura el estado actual de la matriz y guardarlo en la pila de snapshots.
    pushSnapshot(matrizActual->capturarEstado());
    cout << "[Snapshot] Estado real del sistema guardado en la pila." << endl;
}

bool ListaDobleHistorial::deshacerUltimoCambio() {
    // Verifica si hay una matriz enlazada antes de intentar deshacer.
    if (matrizActual == nullptr) {
        cout << "No hay una matriz enlazada para deshacer." << endl;
        return false;
    }
    // Verifica si hay snapshots disponibles antes de intentar deshacer.
    if (cima == nullptr) {
        cout << "No hay cambios guardados para deshacer." << endl;
        return false;
    }

    // Intenta hacer pop del snapshot más reciente y restaurar el estado de la matriz.
    bool ok = popSnapshot(*matrizActual);
    if (ok) {
        cout << "[Undo] Estado del sistema restaurado correctamente." << endl;
    }
    return ok;
}

bool ListaDobleHistorial::haySnapshots() const {
    return cima != nullptr;
}

// Método para agregar una acción al historial
void ListaDobleHistorial::agregarAccion(string accion) {
    // Crear un nuevo nodo con la acción proporcionada
    NodoDoble* nuevo = new NodoDoble(accion);
    // Verificar si la lista está vacía (head es nullptr)
    if (head == nullptr) {
        // Si la lista está vacía, el nuevo nodo será tanto el head como el tail
        head = tail = nuevo;
    } else {
        // Si la lista no está vacía, agrega el nuevo nodo al final de la lista ()
        tail->next = nuevo;
        // Establecer el puntero previo del nuevo nodo al nodo actual tail
        nuevo->prev = tail;
        // Actualizar el puntero tail para que apunte al nuevo nodo
        tail = nuevo;
    }
}

// Método para navegar hacia adelante en el historial (desde el nodo más antiguo hasta el más reciente)

void ListaDobleHistorial::navegarAdelante() {
    //si está vacio, mostrar mensaje y salir
    if (head == nullptr) {
        cout << "El historial esta vacio." << endl;
        return;
    }
    //si no, llama a la función que imprime la acción almacenada en el nodo actual y llama recursivamente a la función con el nodo siguiente
    cout << "\n[Historial: Mas antiguo -> Mas reciente]" << endl;
    navegarAdelante(head);
}

void ListaDobleHistorial::navegarAdelante(NodoDoble* temp) {
    if (temp == nullptr) {
        return;
    }
    cout << " -> " << temp->accion << endl;
    navegarAdelante(temp->next);
}
//desde el más reciente hasta el más antiguo
//para cumplir con poder deshacer accion (último movimiento hecho))
//emplear recursión para navegar hacia atrás en el historial
void ListaDobleHistorial::navegarAtras() {
    //si está vacio, mostrar mensaje y salir
    if (tail == nullptr) {
        cout << "El historial esta vacio." << endl;
        return;
    }
    //si no, llama a la función que imprime a la acción almacenada en el nodo actual y llama recursivamente a la función con el nodo previo
    cout << "\n[Historial: Mas reciente -> Mas antiguo]" << endl;
    navegarAtras(tail);
}


void ListaDobleHistorial::navegarAtras(NodoDoble* temp) {
    if (temp == nullptr) {
        return;
    }
    cout << " -> " << temp->accion << endl;
    navegarAtras(temp->prev);
}