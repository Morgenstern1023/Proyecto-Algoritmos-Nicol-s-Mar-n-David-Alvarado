#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <stdexcept>
using namespace std;

//Se define la clase InformacionUsuarios que maneja un vector dinamico de tuplas con los atributos de  cada uno delos usuarios
class InformacionUsuarios {

        // Se inicializan los atributos del vector, data apunta a un arreglo dinamico
    //tamaño es el numero de elementos que tiene el vector y capacidad es la capacidad maxima del vector
    private:
        tuple<string, string, string, bool>* data;
        int tamaño;
        int capacidad;

        // Metodo que redimensiona el vector cuando se llena, duplicando su capacidad 
        void resize() {
            //Se duplica la capacidad y se crea un nuevo arreglo con la capacidad doble, luego copia los datos del anterior arreglo
            capacidad *= 2;
            tuple<string, string, string, bool>* newData = new tuple<string, string, string, bool>[capacidad];
            for (int i = 0; i < tamaño; i++) {
                newData[i] = data[i];   
            }
            // Se libera la memoria del arreglo anterior y se apunta al nuevo arreglo para evitar fugas de memoria
            delete[] data;
            
            data = newData;
        }
    public:
        // Tamaño inicial del vector es 4 (modificable) y se inicializa el arreglo dinamico
        InformacionUsuarios() {
            tamaño = 0;
            capacidad = 4;
            data = new tuple<string, string, string, bool>[capacidad];
        }
        // Metodo que agrega un elemento al vector, si esta lleno se llama al metodo resize
        void push_back(const tuple<string, string, string, bool>& value) {
            if (tamaño == capacidad) {
                resize();
            }
            data[tamaño++] = value;
        }
        // Metodo que elimina un elemento del vector en el indice, si es invalido lanza una excepcion de index fuera de rango
        void remove(int index) {
            if (index < 0 || index >= tamaño) {
                throw out_of_range("Index out of range");
            }
            for (int i = index; i < tamaño - 1; i++) {
                data[i] = data[i + 1];
            }
            tamaño--;
        }
        // Metodo que obtiene un elemento del vector en el indice, si es invalido lanza una excepcion de index fuera de rango
        tuple<string, string, string, bool> get(int index) {
            if (index < 0 || index >= tamaño) {
                throw out_of_range("Index out of range");
            }
            return data[index];
        }
        // Metodo que obtiene el tamaño del vector
        int gettamaño() {
            return tamaño;
        }

        //Metodo que muestra todos los usuarios que se agregan de la clase usuario
        void mostrarUsuarios() {
            for (int i = 0; i < tamaño; i++) {
                auto [nombre, apellido, documento, sanciones] = data[i];
                cout << "Nombre: " << nombre << ", Apellido: " << apellido << ", Documento: " << documento << ", Sanciones: " << sanciones << endl;
            }
        }
        // Destructor que libera la memoria del arreglo dinamico para evitar fugas de memoria
        ~InformacionUsuarios() {
            delete[] data;
        }

        // Metodo que ordena los usuarios por numero de documento (bucket sort)

        void bucketSortPorDocumento() {
            if (tamaño <= 1) return;

            // Encontrar el valor máximo del documento
            int maxDocumento = 0;
            for (int i = 0; i < tamaño; i++) {
                auto [nombre, apellido, documento, sanciones] = data[i];
                int doc = stoi(documento);
                if (doc > maxDocumento) {
                    maxDocumento = doc;
                }
            }

            // Crear los buckets
            int numBuckets = maxDocumento + 1;
            vector<tuple<string, string, string, bool>>* buckets = new vector<tuple<string, string, string, bool>>[numBuckets];

            // Distribuir los elementos en los buckets
            for (int i = 0; i < tamaño; i++) {
                auto [nombre, apellido, documento, sanciones] = data[i];
                int doc = stoi(documento);
                buckets[doc].push_back(data[i]);
            }

            // Reconstruir el arreglo original a partir de los buckets
            int index = 0;
            for (int i = 0; i < numBuckets; i++) {
                for (auto& elem : buckets[i]) {
                    data[index++] = elem;
                }
            }

            delete[] buckets;
        }

        void heapSortPorDocumento() {
            auto heapify = [&](int n, int i, auto&& heapify_ref) -> void {
                int largest = i;
                int left = 2 * i + 1;
                int right = 2 * i + 2;

                if (left < n && stoi(std::get<2>(data[left])) > stoi(std::get<2>(data[largest]))) {
                    largest = left;
                }
                if (right < n && stoi(std::get<2>(data[right])) > stoi(std::get<2>(data[largest]))) {
                    largest = right;
                }
                if (largest != i) {
                    swap(data[i], data[largest]);
                    heapify_ref(n, largest, heapify_ref);
                }
            };

            for (int i = tamaño / 2 - 1; i >= 0; i--) {
                heapify(tamaño, i, heapify);
            }

            for (int i = tamaño - 1; i >= 0; i--) {
                swap(data[0], data[i]);
                heapify(i, 0, heapify);
            }
        }

        // Metodo de busqueda binaria por numero de documento
        int busquedaBinariaPorDocumento(const string& documento) {
            int izquierda = 0;
            int derecha = tamaño - 1;
            int doc = stoi(documento);
            while (izquierda <= derecha) {
                int medio = izquierda + (derecha - izquierda) / 2;
                // Si no esta el std se daña (no tocar)
                int docMedio = stoi(std::get<2>(get(medio)));
                if (docMedio == doc) {
                    return medio;
                } else if (docMedio < doc) {
                    izquierda = medio + 1;
                } else {
                    derecha = medio - 1;
                }
            }
            return -1; // No encontrado
        }
};