#include "TablaHash.h"

#include <iostream>

using namespace std;

int TablaHash::hashSimple(const string& clave) const {
    unsigned int hash = 0;
    for (char c : clave) {
        //la u es un entero no asignado (hace que el valor siempre de un natural)
        //unsigned char es para lo mismo al leer el carácter
        //static_cast<unsigned char>(c) asegura que el carácter se trate como un valor entero no negativo
        //el conjunto se super mega asegura que nada salga fuera de lo esperado
        hash = (hash * 31u) + static_cast<unsigned char>(c);
    }
    return static_cast<int>(hash);
}
//salting para evitar la colisión de hashes entre diferentes categorías
int TablaHash::valorPorCategoria(const string& codigo, const string& categoria) const {
    int base = hashSimple(codigo);

    if (categoria == "computador" || categoria == "Computador") {
        return base + 1;
    }
    if (categoria == "sala" || categoria == "Sala") {
        return base + 2;
    }
    if (categoria == "libro" || categoria == "Libro") {
        return base + 3;
    }

    return base;
}

void TablaHash::insertar(const string& clave, int valor) {
    // Inserta un nuevo par clave-valor en la tabla hash o actualiza el valor si la clave ya existe
    datos_[clave] = valor;
}

bool TablaHash::existe(const string& clave) const {
    // Verifica si la clave especificada existe en la tabla hash
    return datos_.find(clave) != datos_.end();
}

int TablaHash::buscar(const string& clave) const {
    // busca el valor apuntado por el codigo
    auto it = datos_.find(clave);
    if (it == datos_.end()) {
        // si no se encuentra, retorna -1
        return -1;
    }
    // caso contrario, retorna el valor asociado a la clave
    return it->second;
}

void TablaHash::eliminar(const string& clave) {
    // Elimina el elemento asociado a la clave especificada de la tabla hash
    datos_.erase(clave);
}

size_t TablaHash::tamano() const {
    // Retorna el número de elementos actualmente almacenados en la tabla hash
    return datos_.size();
}

void TablaHash::limpiar() {
    // Elimina todos los elementos de la tabla hash, dejándola vacía
    datos_.clear();
}

void TablaHash::mostrar() const {
    cout << "Contenido de la tabla hash:" << endl;
    for (const auto& par : datos_) {
        cout << "- " << par.first << " -> " << par.second << endl;
    }
}
