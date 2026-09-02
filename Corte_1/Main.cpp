#include <iostream>
#include "VectorPropioClaseInformaciónUsuarios.h"
#include <tuple>
#include <string>
#include <stdexcept>
#include "VectorPropioClaseInventario.h"
#include "ClaseUsuario.h"
#include "ClaseRecursos.h"
using namespace std;

int main() {
    string seguir = "Si";
    Administrador admin(string(""),string(""),string(""), string(""));
    InformacionUsuarios infoUsuario;
    Recurso adminRecursos("","","");
    inventario inv;
    while (seguir=="Si") {
        cout << "A continuación hay una lista con lo que este prototipo del programa puede hacer" << endl;
        cout << "1. Gestión de usuarios" << endl;
        cout << "2. Gestión de recursos" << endl;
        cout << "Escriba el que quiera realizar: " << endl;
        int opcion;
        cin >> opcion;
        if(opcion==1) {
                string documentoUsuario;
                cout << "Ha seleccionado la gestión de usuarios" << endl;
                cout << "Escriba 1 si quiere añadir un usuario" << endl;
                cout << "Escriba 2 si quiere eliminar un usuario" << endl;
                cout << "Escriba 3 si quiere consultar un usuario" << endl;
                cout << "Escriba 4 si quiere consultar todos los usuarios" << endl;
                cout << "Escriba 5 para consultar las sanciones de un usuario" << endl;

                int subopcion;
                cin >> subopcion;
                switch(subopcion) {
                    case 1:
                        cout << "Añadir un usuario" << endl;
                        admin.registrarUsuario(infoUsuario);
                        break;
                        
                    case 2:
                        cout << "Eliminar un usuario" << endl;
                        admin.eliminarUsuario(infoUsuario);
                        break;
                    case 3:
                        cout << "Consultar un usuario" << endl;
                        admin.consultarUsuario(infoUsuario);
                        break;
                    case 4:
                        cout << "Consultar todos los usuarios" << endl;
                        admin.consultarUsuarios(infoUsuario);
                        break;
                    case 5:
                        cout << "Consultar las sanciones de un usuario" << endl;
                        admin.consultarSanciones(infoUsuario, documentoUsuario);
                        break;
                    default:
                        cout << "Opción no válida" << endl;
                }
            }
        
            
                
        else if (opcion==2) {
                cout << "Ha seleccionado la gestión de recursos" << endl;
                cout << "Escriba 1 si quiere añadir un recurso" << endl;
                cout << "Escriba 2 si quiere eliminar un recurso" << endl;
                cout << "Escriba 3 si quiere consultar un recurso" << endl;
                cout << "Escriba 4 si quiere consultar todos los recursos" << endl;
                int subopcionRecursos;
                cin >> subopcionRecursos;
                switch(subopcionRecursos) {
                    case 1:
                        cout << "Añadir un recurso" << endl;
                        adminRecursos.registrarRecurso(inv);
                        break;
                    case 2:
                        cout << "Eliminar un recurso" << endl;
                        adminRecursos.eliminarRecurso(inv);
                        break;
                    case 3:
                        cout << "Consultar un recurso" << endl;
                        adminRecursos.consultarRecurso(inv);
                        break;
                    default:
                        cout << "Opción no válida" << endl;
                }
            }
        
        cout << "¿Desea continuar? (Si/No): ";
        cin >> seguir;
    }
    cout << "Programa finalizado." << endl;
    return 0;
}
