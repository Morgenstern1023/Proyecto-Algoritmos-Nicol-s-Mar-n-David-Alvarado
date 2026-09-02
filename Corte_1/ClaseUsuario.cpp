#include "VectorPropioClaseInformaciónUsuarios.h"
#include "ClaseRecursos.h"
#include "VectorPropioClaseInventario.h"
#include <iostream>
#include <string>
#include <tuple>
#include <stdexcept>
using namespace std;

// Se define la clase Usuario que representa a un usuario del sistema, con atributos como nombre, apellido, numero de documento y sanciones.
//Tambien se define la clase Administrador que hereda de Usuario y agrega atributos y metodos relacionados con la gestion de prestamos y registro de usuarios.

class Usuario {
    private:
        // Se inicializan los atributos de la clase Usuario, nombre, apellido, numeroDocumento y sanciones
        string nombre;
        string apellido;
        string numeroDocumento;
        bool sanciones;
    public:
    // Constructor de la clase Usuario, se inicializan los atributos de la clase
        Usuario(string nombre, string apellido, string numeroDocumento) {
            this->nombre = nombre;
            this->apellido = apellido;
            this->numeroDocumento = numeroDocumento;
            this->sanciones = false;

        }

        //Getters y Setters
        string getNombre() {
            return nombre;
        }
        void setNombre(string nombre) {
            this->nombre = nombre;
        }
        string getApellido() {
            return apellido;
        }
        void setApellido(string apellido) {
            this->apellido = apellido;
        }
        string getNumeroDocumento() {
            return numeroDocumento;
        }
        void setNumeroDocumento(string numeroDocumento) {
            this->numeroDocumento = numeroDocumento;
        }
        bool getSanciones() {
            return sanciones;
        }
        void setSanciones(bool sanciones) {
            this->sanciones = sanciones;
        }

        void consultarDisponibilidad() {
            // Metodo que permite al usuario consultar la disponibilidad de un recurso
            
            cout << "Escriba el codigo del recurso para consultar su disponibilidad: ";
            string codigoRecurso;
            cin >> codigoRecurso;
            //Se busca el recurso en el inventario por codigo y muestra su disponibilidad, como ya existe en la clase Recurso entonces se puede instanciar un objeto Recurso y usar sus metodos.
            Recurso recurso("", "", codigoRecurso);
            cout << "La disponibilidad del recurso con codigo " << codigoRecurso << " es " << (recurso.getDisponibilidad() ? "Disponible" : "No disponible") << endl;
        }


};

class Administrador : public Usuario {
    private:
        // Se inicializan los atributos de la clase Administrador, nombre, apellido, numeroDocumento y sanciones
        string nombre;
        string apellido;
        string numeroDocumento;
        bool sanciones;
        string claveOperaciones; // Clave para realizar operaciones administrativas
        int totalOperaciones; // Total de operaciones administrativas realizadas
    public:
        // Constructor de la clase Administrador, se inicializan los atributos de la clase
        Administrador(string nombre, string apellido, string numeroDocumento, string claveOperaciones) : Usuario(nombre, apellido, numeroDocumento) {

            this->nombre = nombre;
            this->apellido = apellido;
            this->numeroDocumento = numeroDocumento;
            this->sanciones = false;
            this->claveOperaciones = claveOperaciones;
            this->totalOperaciones = 0;
            InformacionUsuarios infoUsuario;
        }

        //Getters y Setters
        string getNombre() {   
            return nombre;
        }
        void setNombre(string nombre) {
            this->nombre = nombre;
        }
        string getApellido() {
            return apellido;
        }
        void setApellido(string apellido) {
            this->apellido = apellido;
        }
        string getNumeroDocumento() {
            return numeroDocumento;
        }
        void setNumeroDocumento(string numeroDocumento) {
            this->numeroDocumento = numeroDocumento;
        }
        bool getSanciones() {
            return sanciones;
        }
        void setSanciones(bool sanciones) {
            this->sanciones = sanciones;
        }

        void setClaveOperaciones(string claveOperaciones) {
            this->claveOperaciones = claveOperaciones;
        }
        string getClaveOperaciones() {
            return claveOperaciones;
        }
        void setTotalOperaciones(int totalOperaciones) {
            this->totalOperaciones = totalOperaciones;
        }
        int getTotalOperaciones() {
            return totalOperaciones;
        }
        void aprobarPrestamo() {
            // Metodo que permite al administrador aprobar un prestamo de un recurso
            //Ingresar el codigo del recurso a prestar
            cout << "Ingrese el codigo del recurso a prestar: ";
            string codigoRecurso;
            cin >> codigoRecurso;
            //Se debe verificar si el recurso con el codigo ingresado está disponible llamando a prestarRecurso
            inventario inv; // Se debe pasar el inventario correspondiente
            Recurso objetoRecurso("", "", codigoRecurso); //Se crea el objeto Recurso con el codigo ingresado
            objetoRecurso.prestarRecurso(codigoRecurso, inv); // Llamada al metodo prestarRecurso de la clase recurso
            //Si no esta disponible, se notifica al usuario y se retorna
            if (!objetoRecurso.prestarRecurso(codigoRecurso, inv)) {
                cout << "El recurso con codigo " << codigoRecurso << " no esta disponible." << endl;
                cout << "No se puede aprobar el prestamo, pase a la lista de espera." << endl;
                return;
            }
            else {
                cout << "El recurso con codigo " << codigoRecurso << " esta disponible." << endl;
            }
            //Si esta disponible se pasa a revisar las sanciones del usuario antes de aprobar el prestamo
            cout << "Escriba el documento del usuario: ";
            string documentoUsuario;
            cin >> documentoUsuario;
            InformacionUsuarios infoUsuario; // Se debe pasar la información correspondiente del usuario
            consultarSanciones(infoUsuario, documentoUsuario);
            // En caso de aprobar el prestamo se debe pasar a la lista enlazada de prestamos y cambiar disponibilidad
            // Implementación pendiente
            cout << "El administrador " << nombre << " " << apellido << " ha aprobado un prestamo." << endl;
            cout << "Escriba la fecha y la hora del prestamo (formato: dd/mm/aaaa hh:mm): ";
            string fechaHoraPrestamo;
            cin >> fechaHoraPrestamo;
        }
        void rechazarPrestamo() {
            // Metodo que permite al administrador rechazar un prestamo de un recurso
            cout << "El administrador " << nombre << " " << apellido << " ha rechazado un prestamo." << endl;
            // Implementación pendiente
        }
        
        void registrarDevolucion() {
            // Metodo que permite al administrador registrar la devolucion de un recurso
            cout << "El administrador " << nombre << " " << apellido << " ha registrado una devolucion." << endl;
            cout << "Escriba la fecha y la hora de la devolucion (formato: dd/mm/aaaa hh:mm): ";
            string fechaHoraDevolucion;
            cin >> fechaHoraDevolucion;
            //Pasar disponibilidad del recurso a true al registrar la devolucion en inventario
            // Implementación pendiente: se debe actualizar la disponibilidad del recurso en el inventario
        }

        void consultarSanciones(InformacionUsuarios& infoUsuario, string documentoUsuario) {
            // Metodo que permite al administrador consultar las sanciones de un usuario
            cout << "El administrador " << nombre << " " << apellido << " ha consultado las sanciones de un usuario." << endl;
            //cout << "Ingrese el documento del usuario a consultar: ";
            //string documentoUsuarioConsultar;
            //cin >> documentoUsuarioConsultar;
            string documentoUsuarioConsultar = documentoUsuario;
            // Se obtiene el tamaño del vector dinamico de usuarios para iterar sobre el mismo y usar el get en el for
            int tamaño = infoUsuario.gettamaño();
            // Se itera sobre el vector dinamico de usuarios para buscar el usuario con el documento especificado que es el tercer dato o indice 2
            //Se imprime si el usuario tiene sanciones
            for (int i = 0; i < tamaño; i++) {
                if (get<2>(infoUsuario.get(i)) == documentoUsuarioConsultar) {

                    cout << "Sanciones: " << (get<3>(infoUsuario.get(i)) ? "Si" : "No") << endl;
                    break;
                }
                if (i == tamaño - 1) {
                    cout << "Usuario no encontrado." << endl;
                }
            }
            return;
        }

        void consultarUsuarios(InformacionUsuarios& infoUsuario) {
            // Metodo que permite al administrador consultar los usuarios registrados
            cout << "El administrador " << nombre << " " << apellido << " ha consultado los usuarios registrados." << endl;
            //Se utiliza el vector dinamico de la clase InformacionUsuarios para mostrar los usuarios registrados
            // Se llama al metodo mostrarUsuarios() del vector dinamico de la clase InformacionUsuarios 
            // Se crea un objeto de la clase InformacionUsuarios llamado infoUsuarios
            infoUsuario.mostrarUsuarios();
        }

        void consultarUsuario(InformacionUsuarios& infoUsuario) {
            // Metodo que permite al administrador consultar la informacion de un usuario especifico
            // InformacionUsuarios infoUsuarios; // Ya se pasa como referencia
            cout << "El administrador " << nombre << " " << apellido << " ha consultado la informacion de un usuario." << endl;
            cout << "Escriba el documento del usuario a consultar: ";
            string documentoUsuarioConsultar;
            cin >> documentoUsuarioConsultar;
            // Se obtiene el tamaño del vector dinamico de usuarios para iterar sobre el mismo y usar el get en el for
            int tamaño = infoUsuario.gettamaño();
            // Se itera sobre el vector dinamico de usuarios para buscar el usuario con el documento especificado que es el tercer dato o indice 2
            //Se imprime cada dato del usuario encontrado
            for (int i = 0; i < tamaño; i++) {
                if (get<2>(infoUsuario.get(i)) == documentoUsuarioConsultar) {
                    cout << "Nombre: " << get<0>(infoUsuario.get(i)) << endl;
                    cout << "Apellido: " << get<1>(infoUsuario.get(i)) << endl;
                    cout << "Documento: " << get<2>(infoUsuario.get(i)) << endl;
                    cout << "Sanciones: " << (get<3>(infoUsuario.get(i)) ? "Si" : "No") << endl;
                    break;
                }
                if (i == tamaño - 1) {
                    cout << "Usuario no encontrado." << endl;
                }
            }
        }

        void registrarUsuario(InformacionUsuarios& infoUsuario) {
            // Metodo que permite al administrador registrar un usuario
            // Se crea un objeto de la clase InformacionUsuarios llamado infoUsuarios para agregar el nuevo usuario
            cout << "Escriba el nombre del usuario: ";
            string nombreUsuario;
            cin >> nombreUsuario;
            cout << "Escriba el apellido del usuario: ";
            string apellidoUsuario;
            cin >> apellidoUsuario;
            cout << "Escriba el documento del usuario: ";
            string documentoUsuario;
            cin >> documentoUsuario;
            bool sanciones;
            sanciones = false; // Inicialmente se asume que el usuario no tiene sanciones
            // Se agrega el nuevo usuario al vector dinamico de la clase InformacionUsuarios
            infoUsuario.push_back(make_tuple(nombreUsuario, apellidoUsuario, documentoUsuario, sanciones));
            cout << "El usuario " << nombreUsuario << " " << apellidoUsuario << " ha sido registrado." << endl;
            totalOperaciones++;
        }

        void eliminarUsuario(InformacionUsuarios& infoUsuario) {
            // Metodo que permite al administrador eliminar un usuario
            // Se crea un objeto de la clase InformacionUsuarios llamado infoUsuarios para eliminar el usuario
            cout << "Escriba el documento del usuario a eliminar: ";
            string documentoUsuarioEliminar;
            cin >> documentoUsuarioEliminar;
            // Se busca el usuario en el vector dinamico de la clase InformacionUsuarios y se elimina
            int i;
            for (i = 0; i < infoUsuario.gettamaño(); i++) {
                auto [nombre, apellido, documento, sanciones] = infoUsuario.get(i);
                if (documento == documentoUsuarioEliminar) {
                    infoUsuario.remove(i);
                    break;
                }
                
            }
            //Se informa que no se encontro el usuario o que fue eliminado
            if (i == infoUsuario.gettamaño()) {
                cout << "No se encontró un usuario con documento " << documentoUsuarioEliminar << "." << endl;
            }
            else {
                cout << "El usuario con documento " << documentoUsuarioEliminar << " ha sido eliminado." << endl;
            }
            totalOperaciones++;
        }

    };