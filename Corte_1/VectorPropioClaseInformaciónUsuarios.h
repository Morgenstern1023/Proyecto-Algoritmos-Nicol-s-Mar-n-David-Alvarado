#pragma once
#include <iostream>
#include <string>
#include <tuple>
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

};