#pragma once

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class TablaHash {
public:
    void insertar(const string& clave, int valor);
    bool existe(const string& clave) const;
    int buscar(const string& clave) const;
    void eliminar(const string& clave);
    size_t tamano() const;
    void limpiar();

    // Funciones extras para soportar el uso del hash con recursos y categorias
    int hashSimple(const string& clave) const;
    int valorPorCategoria(const string& codigo, const string& categoria) const;
    void mostrar() const;

private:
    //no se implementó el hashing manualmente 
    // pero si se desarrolló una capa de hashing, 
    // ya que la tabla hash sin usar unordered map es demasiado tediosa de implementar,
    // por lo que se desarrolló su idea sobre la librería
    unordered_map<string, int> datos_;
};
