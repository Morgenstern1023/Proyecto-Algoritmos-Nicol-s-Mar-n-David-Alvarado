#include <iostream>
#include <string>
#include "Disponibilidad.h"
#include "Historial.h"

using namespace std;

int main() {
    // Instanciar las dos clases principales: MatrizDisponibilidad y ListaDobleHistorial de Disponibilidad y Historial respectivamente
    MatrizDisponibilidad matriz;
    ListaDobleHistorial historial;

    int opcion;
    do {
        cout << "Gracias por preferirnos :D" << endl;
        cout << "\n \033[1mDisponibilidad e Historial\033[0m" << endl;
        cout << "\033[1m1.\033[0m Agregar Sede" << endl;
        cout << "\033[1m2.\033[0m Inicializar Recurso" << endl;
        cout << "\033[1m3.\033[0m Reservar Turno" << endl;
        cout << "\033[1m4.\033[0m Ver Disponibilidad de un Recurso" << endl;
        cout << "\033[1m5.\033[0m Ver historial (Antiguo -> Reciente)" << endl;
        cout << "\033[1m6.\033[0m Ver historial (Reciente -> Antiguo)" << endl;
        cout << "\033[1m7.\033[0m Salir" << endl;
        cout << "Elige una opcion: ";
        cin >> opcion;
        if (cin.fail()){
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Error: Opcion invalida. Por favor, ingresa un numero del 1 al 7." << endl;
            continue;
        }

        if (opcion == 1) {
            string nombreSede;
            cout << "Ingresa el nombre de la sede: ";
            cin >> nombreSede;
            matriz.agregarSede(nombreSede);
            historial.agregarAccion("Se agrego la sede: " + nombreSede);

        } else if (opcion == 2) {
            string codigo;
            cout << "Ingresa el codigo del recurso: ";
            cin >> codigo;
            matriz.inicializarRecurso(codigo);
            historial.agregarAccion("Se inicializo el recurso: " + codigo);

        } else if (opcion == 3) {
            string codigo;
            int sede, dia;
            cout << "Codigo del recurso: ";
            cin >> codigo;
            cout << "Indice de la sede: ";
            cin >> sede;
            cout << "Dia de la semana (0(lunes) a 6(domingo)): ";
            cin >> dia;
            matriz.reservarTurno(codigo, sede, dia);
            historial.agregarAccion("Se reservo turno para el recurso: " + codigo);

        } else if (opcion == 4) {
            string codigo;
            cout << "Ingresa el codigo del recurso a consultar: ";
            cin >> codigo;
            matriz.mostrarMatriz(codigo);

        } else if (opcion == 5) {
            historial.navegarAdelante();

        } else if (opcion == 6) {
            historial.navegarAtras();
        }

    } while (opcion != 7);
    cout << "Gracias por usar nuestro programa, Vuelva pronto :D" << endl;
    return 0;
}