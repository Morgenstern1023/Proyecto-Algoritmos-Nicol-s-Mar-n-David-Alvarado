// A continuación se implementa una clase tabla hash la cual pueda buscar inmediatamente un recurso por codigo
//Para esta entrega esta solo se utiliza para la consulta de recursos y no se implementa aun a prestamos y devoluciones

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <list>

using namespace std;

class TablaHash{
private:
    int cap;
    int n;
    std::vector<std::list<std::pair<std::string, std::string>>> cubetas;
public:
    TablaHash(int capacidad=500):cap(capacidad),n(0),cubetas(capacidad){}

int hashear(const string & clave) const {
        unsigned long long h = 0;
        for (unsigned char c : clave) h = (h * 31 + c) % cap;
        return (int)h;
    }
    void insertar(const string& clave, const string& valor) {
        int i = hashear(clave);
        for (auto& par : cubetas[i]) {
            if (par.first == clave) { par.second = valor; return; }   // ACTUALIZA
        }
        cubetas[i].push_back({clave, valor});
        n++;
    }
    bool buscar(const string& clave, string& salida) const {
        int i = hashear(clave);
        for (const auto& par : cubetas[i]) {
            if (par.first == clave) { salida = par.second; return true; }
        }
        return false;
    }
    bool eliminar(const string& clave) {
        int i = hashear(clave);
        for (auto it = cubetas[i].begin(); it != cubetas[i].end(); ++it) {
            if (it->first == clave) { cubetas[i].erase(it); n--; return true; }
        }
        return false;

    }
};

TablaHash tabla;