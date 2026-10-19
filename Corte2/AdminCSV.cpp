#include "AdminCSV.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

//se implementó en el boceto inicial para esta entrega, antes de implementar el getline
//se decidió dejar si a futuro se quisiera procesar una fila completa como un vector de columnas
//reutilizable para cualquier archivo CSV que se quiera procesar como un vector de columnas


vector<string> AdminCSV::dividirLinea(const string& linea) const {
    vector<string> columnas;
    stringstream ss(linea);
    string campo;

    while (getline(ss, campo, ',')) {
        columnas.push_back(campo);
    }

    return columnas;
}

//lo mismo que el anterior, pero sirve para errores de typeado (espacios extra al inicio o al final)
//lineas con \r o \n

string AdminCSV::normalizarTexto(const string& valor) const {
    string texto = valor;
    // Eliminar espacios en blanco y caracteres de control (\t, \r, \n) al inicio del texto
    while (!texto.empty() && (texto.front() == ' ' || texto.front() == '\t' || texto.front() == '\r' || texto.front() == '\n')) {
        texto.erase(texto.begin());
    }
    // Eliminar espacios en blanco y caracteres de control (\t, \r, \n) al final del texto
    while (!texto.empty() && (texto.back() == ' ' || texto.back() == '\t' || texto.back() == '\r' || texto.back() == '\n')) {
        texto.pop_back();
    }
    // se decidió partir el bucle en dos para que sea más legible y fácil de mantener 
    // tambíen para poder aislar problemas no calculados
    // retorna el texto sin espacios ni caracteres de control al inicio y al final (entendible para el algoritmo en esos casos)
    return texto;
}

bool AdminCSV::cargarInventario(const string& rutaArchivo, vector<ItemInventario>& items) {
    ifstream archivo(rutaArchivo);
    // Verificar si el archivo se abrió correctamente antes de continuar con la lectura
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo en la ruta: " << rutaArchivo << endl;
        return false;
    }

    string linea;
    int numeroLinea = 0;
    // Intenta leer la primera línea del archivo CSV (titulos de las columnas) y descartarla
    // si no puede, avisa IMPORTANTE(Cierra el archivo) y retorna false
    //se podría hacer un do-while para leer la primera línea y luego continuar con el resto del archivo pero 
    //se optó por un simple if para mayor claridad en este caso
    if (!getline(archivo, linea)) {
        cerr << "Advertencia: el archivo CSV esta vacio." << endl;
        archivo.close();
        return false;
    }

    numeroLinea++;

    while (getline(archivo, linea)) {
        numeroLinea++;
        // Ignora líneas vacías para evitar errores de formato
        if (linea.empty()) {
            continue;
        }
        // Crear un stringstream para procesar la línea actual del CSV
        //string stream es como un cursor que recorre la linea actual del archivo
        stringstream ss(linea);
        string celda;
        ItemInventario item;
        // Intenta procesar cada dato empleando el normalizado, si se encuentra algún error se captura en el bloque catch
        try {
            getline(ss, celda, ',');
            item.categoria = normalizarTexto(celda);

            getline(ss, celda, ',');
            item.codigo = normalizarTexto(celda);

            getline(ss, celda, ',');
            item.estado = normalizarTexto(celda);

            getline(ss, celda, ',');
            string disponible = normalizarTexto(celda);
            item.disponible = (disponible == "1" || disponible == "true" || disponible == "TRUE");

            items.push_back(item);
        } catch (const exception& e) {
            cerr << "[ADVERTENCIA] Error de formato en la linea " << numeroLinea
                 << " del CSV. Se omite esta fila. Detalle: " << e.what() << endl;
        }
    }
    // Cierra el archivo después de procesar todas las líneas y retorna true indicando éxito
    archivo.close();
    return true;
}

bool AdminCSV::guardarInventario(const string& rutaArchivo, const vector<ItemInventario>& items) {
    // Intenta abrir el archivo para escritura, si no puede, avisa y retorna false
    // ios::out indica que se abre el archivo para escritura
    // ios::trunc indica que se "trunca" el archivo, es decir, se borra su contenido previo antes de escribir
    // esto para asegurarse de que siempre se escriba la versión más reciente del inventario
    ofstream archivo(rutaArchivo, ios::out | ios::trunc);
    // Verifica si el archivo se abrió correctamente antes de cualquier intento de sobreescribirlo.
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para guardar en: " << rutaArchivo << endl;
        return false;
    }
    // Escribe la cabecera del CSV antes de los datos del inventario
    archivo << "categoria,codigo,estado,disponible\n";

    // Se recorren los items y se escriben en el archivo CSV uno por uno
    for (const auto& item : items) {
        archivo << item.categoria << ','
                << item.codigo << ','
                << item.estado << ','
                // ? Convierte el valor booleano de disponible a "1" si es true y "0" si es false
                << (item.disponible ? "1" : "0") << '\n';
    }

    archivo.close();
    cout << "Archivo actualizado correctamente en: " << rutaArchivo << endl;
    return true;
}
