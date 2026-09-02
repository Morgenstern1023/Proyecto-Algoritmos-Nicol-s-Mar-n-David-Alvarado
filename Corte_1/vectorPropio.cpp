// Crear un vector propio el cual puede crecer indefinidamente
#include <iostream>
#include <string>
#include <tuple>
using namespace std;

class Vector {
    private:
    // Se inicializan los atributos del vector, data apunta a un arreglo dinamico
    //tamaño es el numero de elementos que tiene el vector y capacidad es la capacidad maxima del vector
        tuple<string, string, string, bool, string, string>* data;
        int tamaño;
        int capacidad;

        // Metodo que redimensiona el vector cuando se llena, duplicando su capacidad 
        void resize() {
            //Se duplica la capacidad y se crea un nuevo arreglo con la capacidad doble, luego copia los datos del anterior arreglo
            capacidad *= 2;
            tuple<string, string, string, bool, string, string>* newData = new tuple<string, string, string, bool, string, string>[capacidad];
            for (int i = 0; i < tamaño; i++) {
                newData[i] = data[i];
            }
            // Se libera la memoria del arreglo anterior y se apunta al nuevo arreglo para evitar fugas de memoria
            delete[] data;
            data = newData;
        }
    public:
        // Tamaño inicial del vector es 4 (modificable) y se inicializa el arreglo dinamico
        Vector() {
            tamaño = 0;
            capacidad = 4;
            data = new tuple<string, string, string, bool, string, string>[capacidad];
        }
        // Metodo que agrega un elemento al vector, si esta lleno se llama al metodo retamaño
        void push_back(const tuple<string, string, string, bool, string, string>& value) {
            if (tamaño == capacidad) {
                resize();
            }
            data[tamaño++] = value;
        }
        // Metodo que obtiene un elemento del vector en el indice, si es invalido lanza una excepcion de index fuera de rango
        tuple<string, string, string, bool, string, string> get(int index) {
            if (index < 0 || index >= tamaño) {
                throw out_of_range("Index out of range");
            }
            return data[index];
        }
        // Metodo que obtiene el tamaño del vector
        int gettamaño() {
            return tamaño;
        }
        // Destructor que libera la memoria del arreglo dinamico para evitar fugas de memoria
        ~Vector() {
            delete[] data;
        }
};