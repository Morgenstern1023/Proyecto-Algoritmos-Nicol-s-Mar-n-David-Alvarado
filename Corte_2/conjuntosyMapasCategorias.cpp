// Esta sección se encarga de organizar los recursos en categorias
#include <iostream>
#include <map>
#include <set>
#include <string>

using namespace std;

map<string, set<string>> categorias; // Mapa que asocia cada categoría con un conjunto de recursos


// Crear categoria si no existe y agregar recurso a la categoría

void crearCategoriaSiNoExiste(const string& categoria) {
    if (categorias.find(categoria) == categorias.end()) {
        categorias[categoria] = set<string>();
    }
}

void agregarRecursoACategoria(const string& categoria, const string& recurso) {
    categorias[categoria].insert(recurso);
}

void mostrarRecursosPorCategoria(const string& categoria) {
    if (categorias.find(categoria) != categorias.end()) {
        cout << "Recursos en la categoría " << categoria << ":" << endl;
        for (const auto& recurso : categorias[categoria]) {
            cout << "- " << recurso << endl;
        }
    } else {
        cout << "No hay recursos en la categoría " << categoria << endl;
    }
}