#include "Disponibilidad.h"

// Constructor con parámetros opcionales para capacidad inicial
MatrizDisponibilidad::MatrizDisponibilidad(int capRecursosInicial, int capSedesInicial) {
    totalRecursos = 0;
    capacidadRecursos = capRecursosInicial;
    totalSedes = 0; 
    capacidadSedes = capSedesInicial;
    // Inicializar arreglos dinámicos
    codigos = new string[capacidadRecursos];
    nombresSedes = new string[capacidadSedes];
    // Inicializar nombres de sedes por defecto
    for(int i = 0; i < capacidadSedes; i++) {
        nombresSedes[i] = "Sede " + to_string(i + 1);
    }
    // Inicializar matriz de disponibilidad con true (disponible)
    matrizDisp = new bool[capacidadRecursos * capacidadSedes * 7];
    for (int i = 0; i < capacidadRecursos * capacidadSedes * 7; i++) {
        matrizDisp[i] = true;
    }
}

//destructor
MatrizDisponibilidad::~MatrizDisponibilidad() {
    delete[] codigos;
    delete[] nombresSedes;
    delete[] matrizDisp;
}

// Fórmula para calcular la posición en el arreglo
int MatrizDisponibilidad::obtenerIndice(int recurso, int sede, int dia) {
    return (recurso * capacidadSedes * 7) + (sede * 7) + dia;
}

void MatrizDisponibilidad::resizeRecursos() {
    //Duplicar la capacidad de recursos
    int nuevaCapacidadRecursos = capacidadRecursos * 2;
    //arreglo temporal codigos
    string* nuevosCodigos = new string[nuevaCapacidadRecursos];
    //arreglo temporal matrizDisp
    bool* nuevaMatriz = new bool[nuevaCapacidadRecursos * capacidadSedes * 7];

    //Inicializar toda la nueva matriz en true (disponible)
    for (int i = 0; i < nuevaCapacidadRecursos * capacidadSedes * 7; i++) {
        nuevaMatriz[i] = true;
    }

    //Copiar códigos antiguos a la nueva matriz de códigos
    for (int i = 0; i < totalRecursos; i++) {
        nuevosCodigos[i] = codigos[i];
    }

    //Copiar datos de disponibilidad antiguos a la nueva matriz
    for (int r = 0; r < totalRecursos; r++) {
        for (int s = 0; s < totalSedes; s++) {
            for (int d = 0; d < 7; d++) {
                int viejoIdx = obtenerIndice(r, s, d);
                // Usamos temporalmente la capacidad nueva para calcular el índice destino
                int nuevoIdx = (r * capacidadSedes * 7) + (s * 7) + d;
                nuevaMatriz[nuevoIdx] = matrizDisp[viejoIdx];
            }
        }
    }
    //Liberar memoria antigua y actualizar punteros y capacidades
    delete[] codigos;
    delete[] matrizDisp;
    codigos = nuevosCodigos;
    matrizDisp = nuevaMatriz;
    capacidadRecursos = nuevaCapacidadRecursos;
}

//Buscar el índice de un recurso por su código
int MatrizDisponibilidad::buscarIndiceRecursoPorCodigo(string codigo) {
    for (int i = 0; i < totalRecursos; i++) {
        //retorna index si encuentra el codigo, sino retorna -1
        if (codigos[i] == codigo) return i;
    }
    return -1;
}

void MatrizDisponibilidad::agregarSede(string nombreSede) {
    //Verificar si la sede ya existe
    for (int i = 0; i < totalSedes; i++) {
        if (nombresSedes[i] == nombreSede) {
            cout << "Error: La sede '" << nombreSede << "' ya existe." << endl;
            return;
        }
    }
    //Verificar si es necesario redimensionar el arreglo de sedes
    if (totalSedes >= capacidadSedes) {
        // Duplicar la capacidad de sedes
        int nuevaCapacidadSedes = capacidadSedes * 2;
        // Arerglo temporal para nombres de sedes y matriz de disponibilidad
        string* nuevasSedes = new string[nuevaCapacidadSedes];
        bool* nuevaMatriz = new bool[capacidadRecursos * nuevaCapacidadSedes * 7];
        // Se inicializa nuevo vector
        for (int i = 0; i < capacidadRecursos * nuevaCapacidadSedes * 7; i++) {
            nuevaMatriz[i] = true;
        }
        // Copiar nombres de sedes antiguos a la nueva matriz de nombres
        for (int s = 0; s < totalSedes; s++) {
            nuevasSedes[s] = nombresSedes[s];
        }
        // Copiar datos de disponibilidad antiguos a la nueva matriz
        for (int r = 0; r < totalRecursos; r++) {
            for (int s = 0; s < totalSedes; s++) {
                for (int d = 0; d < 7; d++) {
                    int viejoIdx = (r * capacidadSedes * 7) + (s * 7) + d;
                    int nuevoIdx = (r * nuevaCapacidadSedes * 7) + (s * 7) + d;
                    // Copiar el valor de disponibilidad a la nueva matriz
                    nuevaMatriz[nuevoIdx] = matrizDisp[viejoIdx];
                }
            }
        }
        //Liberar memoria antigua y actualizar punteros y capacidades
        delete[] nombresSedes;
        delete[] matrizDisp;

        //Actualizar punteros y capacidades
        nombresSedes = nuevasSedes;
        matrizDisp = nuevaMatriz;
        capacidadSedes = nuevaCapacidadSedes;
    }
    //Agregar la nueva sede al arreglo de nombres de sedes
    nombresSedes[totalSedes] = nombreSede;
    // Incrementar el contador de sedes
    totalSedes++;
    cout << "Sede '" << nombreSede << "' agregada exitosamente." << endl;
}

void MatrizDisponibilidad::inicializarRecurso(string codigo) {
    if (buscarIndiceRecursoPorCodigo(codigo) != -1) {
        cout << "Error: El recurso con codigo '" << codigo << "' ya existe." << endl;
        return;
    }

    if (totalRecursos == capacidadRecursos) {
        resizeRecursos();
    }

    codigos[totalRecursos] = codigo;
    totalRecursos++;
}

void MatrizDisponibilidad::reservarTurno(string codigo, int sede, int dia) {
    // Buscar el index del recurso por su código
    int indice = buscarIndiceRecursoPorCodigo(codigo);
    // Verificar que el índice sea válido y que la sede y el día estén dentro del rango válido
    if (indice != -1 && sede >= 0 && sede < totalSedes && dia >= 0 && dia < 7) {
        // Calcular el index en la matriz de disponibilidad
        int idx = obtenerIndice(indice, sede, dia);
        // Verificar si el turno está disponible y reservarlo si es así
        if (matrizDisp[idx]) {
            matrizDisp[idx] = false;
            // Informar al usuario que la reserva fue exitosa
            cout << "Turno reservado exitosamente en " << nombresSedes[sede] << "." << endl;
        } else {
            cout << "Error: El turno ya esta ocupado." << endl;
            // Informar al usuario que el turno ya está ocupado
        }
    } else {
        cout << "Error: Codigo no encontrado o indice de sede/dia fuera de rango." << endl;
        // Informar al usuario error de typeado o rango
    }
}

void MatrizDisponibilidad::mostrarMatriz(string codigo) {
    int indice = buscarIndiceRecursoPorCodigo(codigo);
    if (indice == -1) {
        // Informar al usuario que el recurso no fue encontrado
        cout << "Recurso no encontrado en la matriz." << endl;
        return;
    }
    // Mostrar la disponibilidad del recurso en formato de tabla
    cout << "\nDisponibilidad del recurso (Codigo: " << codigo << ")" << endl;
    cout << "Sede/Dia\tM  T  W  Th  F  S  Su" << endl;
    //recorrer matriz
    for (int i = 0; i < totalSedes; i++) {
        // Mostrar el nombre de la sede y la disponibilidad para cada día
        cout << nombresSedes[i] << "\t";
        for (int j = 0; j < 7; j++) {
            // Calcular el índice en la matriz de disponibilidad y mostrar si está disponible o no
            int idx = obtenerIndice(indice, i, j);
            // Mostrar si el turno está disponible ([O]) o ocupado ([X])
            cout << (matrizDisp[idx] ? "\033[92m[O]\033[0m" : "\033[91m[X]\033[0m") << " ";
        }
        cout << endl;
    }
}