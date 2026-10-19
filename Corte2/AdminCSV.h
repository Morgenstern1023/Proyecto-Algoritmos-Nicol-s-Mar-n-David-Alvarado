#pragma once

#include <string>
#include <vector>

using namespace std;

struct ItemInventario {
    string categoria;
    string codigo;
    string estado;
    bool disponible;
};

class AdminCSV {
public:
    bool cargarInventario(const string& rutaArchivo, vector<ItemInventario>& items);
    bool guardarInventario(const string& rutaArchivo, const vector<ItemInventario>& items);

private:
    vector<string> dividirLinea(const string& linea) const;
    string normalizarTexto(const string& valor) const;
};
