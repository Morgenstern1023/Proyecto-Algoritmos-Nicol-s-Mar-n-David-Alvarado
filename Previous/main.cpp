#include <iostream>
#include <string>
#include <vector>

#include "Corte1/Disponibilidad.h"
#include "Corte1/Historial.h"
#include "Corte2/AdminCSV.h"
#include "Corte2/TablaHash.h"

using namespace std;

int main() {
    // Inicializar el sistema de disponibilidad y historial
    MatrizDisponibilidad matriz;
    ListaDobleHistorial historial;
    historial.enlazarMatriz(&matriz);
    // Inicializar el administrador de CSV y la tabla hash
    AdminCSV archivoCSV;
    TablaHash tablaHash;
    vector<ItemInventario> inventario;
    const string rutaInventario = "dependencias/inventario.csv";
    // Ruta del archivo CSV de inventario
    int opcion = 0;
    // Bucle principal del sistema
    do {
        cout << "\033[1;32mSISTEMA DE DISPONIBILIDAD Y HISTORIAL\033[0m" << endl;
        cout << "\033[1;34m1. Cargar inventario desde CSV\033[0m" << endl;
        cout << "\033[1;34m2. Agregar sede\033[0m" << endl;
        cout << "\033[1;34m3. Inicializar recurso\033[0m" << endl;
        cout << "\033[1;34m4. Reservar turno\033[0m" << endl;
        cout << "\033[1;34m5. Mostrar disponibilidad de un recurso\033[0m" << endl;
        cout << "\033[1;34m6. Ver historial (antiguo -> reciente)\033[0m" << endl;
        cout << "\033[1;34m7. Ver historial (reciente -> antiguo)\033[0m" << endl;
        cout << "\033[1;34m8. Deshacer ultimo cambio (undo real)\033[0m" << endl;
        cout << "\033[1;34m9. Mostrar cantidad de recursos en hash\033[0m" << endl;
        cout << "\033[1;34m0. Salir\033[0m" << endl;
        cout << "Opcion: ";
        cin >> opcion;
        // Validar la entrada del usuario
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Opcion invalida. Intenta de nuevo." << endl;
            continue;
        }
            // Procesamiento entrada
        //cargar inventario desde el archivo csv
        if (opcion == 1) {
            if (archivoCSV.cargarInventario(rutaInventario, inventario)) {
                cout << "Inventario cargado desde: " << rutaInventario << endl;
                cout << "Registros cargados: " << inventario.size() << endl;

                for (const auto& item : inventario) {
                    if (!item.codigo.empty() && !matriz.existeRecurso(item.codigo)) {
                        matriz.inicializarRecurso(item.codigo);
                    }
                    if (!item.codigo.empty()) {
                        tablaHash.insertar(item.codigo, 0);
                    }
                }

                historial.agregarAccion("Se cargo el archivo CSV de inventario.");
            } else {
                cout << "No se pudo cargar el inventario." << endl;
            }
        }
        // agregar sede
        else if (opcion == 2) {
            string nombreSede;
            cout << "Nombre de la sede: ";
            cin >> nombreSede;

            historial.guardarSnapshotActual();
            matriz.agregarSede(nombreSede);
            historial.agregarAccion("Se agrego la sede: " + nombreSede);
            cout << "Sede agregada correctamente." << endl;
        }
        // inicializar recurso
        else if (opcion == 3) {
            string codigo;
            cout << "Codigo del recurso: ";
            cin >> codigo;

            historial.guardarSnapshotActual();
            matriz.inicializarRecurso(codigo);
            tablaHash.insertar(codigo, 0);
            historial.agregarAccion("Se inicializo el recurso: " + codigo);
            cout << "Recurso inicializado." << endl;
        }
        // reservar turno
        else if (opcion == 4) {
            string codigo;
            int sede, dia;
            cout << "Codigo del recurso: ";
            cin >> codigo;
            cout << "Indice de la sede: ";
            cin >> sede;
            cout << "Dia de la semana (0-6): ";
            cin >> dia;

            historial.guardarSnapshotActual();
            matriz.reservarTurno(codigo, sede, dia);
            historial.agregarAccion("Se reservo turno para " + codigo + " en sede " + to_string(sede));
        }
        // mostrar disponibilidad de un recurso
        else if (opcion == 5) {
            string codigo;
            cout << "Codigo del recurso: ";
            cin >> codigo;
            matriz.mostrarMatriz(codigo);
        }
        // ver historial (antiguo -> reciente)
        else if (opcion == 6) {
            historial.navegarAdelante();
        }
        // ver historial (reciente -> antiguo)
        else if (opcion == 7) {
            historial.navegarAtras();
        }
        // deshacer ultimo cambio (undo real)
        else if (opcion == 8) {
            if (historial.deshacerUltimoCambio()) {
                cout << "Se aplico el undo del ultimo cambio real del sistema." << endl;
            } else {
                cout << "No hay cambios para deshacer." << endl;
            }
        }
        // mostrar cantidad de recursos en hash
        else if (opcion == 9) {
            cout << "Elementos actuales en la tabla hash: " << tablaHash.tamano() << endl;
        }
        // salir del sistema
        else if (opcion == 0) {
            cout << "Saliendo del sistema..." << endl;
        }
        else {
            cout << "Opcion no valida." << endl;
        }

    } while (opcion != 0);

    cout << "\033[1;32mGracias por usar el sistema \033[1;33m:p \033[1;34m:D\033[0m" << endl;
    return 0;
}
