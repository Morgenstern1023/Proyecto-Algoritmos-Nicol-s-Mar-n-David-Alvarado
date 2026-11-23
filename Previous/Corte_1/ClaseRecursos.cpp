#include "VectorPropioClaseInventario.h"
#include <iostream>
#include <string>
#include <tuple>
#include <stdexcept>
using namespace std;

//Se crea la clase recurso que maneja la informacion de cada recurso disponible en el inventario y sus metodos
    class Recurso {
    private:
        // Se inicializan los atributos de la clase Recurso, nombre, tipo y disponibilidad
        string nombre;
        string tipo;
        string codigo;
        bool disponibilidad;
        string estado;
    public:
        // Constructor de la clase Recurso, se inicializan los atributos de la clase
        Recurso(string nombre, string tipo, string codigo) {
            this->nombre = nombre;
            this->tipo = tipo;
            this->codigo = codigo;
            this->disponibilidad = true;
            this->estado = "Bueno";
        }

        //Getters y Setters
        string getNombre() {
            return nombre;
        }
        void setNombre(string nombre) {
            this->nombre = nombre;
        }
        string getTipo() {
            return tipo;
        }
        void setTipo(string tipo) {
            this->tipo = tipo;
        }
        string getCodigo() {
            return codigo;
        }
        void setCodigo(string codigo) {
            this->codigo = codigo;
        }
        bool getDisponibilidad() {
            return disponibilidad;
        }
        void setDisponibilidad(bool disponibilidad) {
            this->disponibilidad = disponibilidad;
        }
        string getEstado() {
            return estado;
        }
        void setEstado(string estado) {
            this->estado = estado;
        }

        void consultarRecurso(inventario& inv) {
            // Metodo que permite consultar un recurso obteniendo toda su informacion del inventario
            cout << "Escriba el codigo del recurso a consultar: ";
            string codigo;
            cin >> codigo;
            int tamaño = inv.gettamaño();
            for (int i = 0; i < tamaño; i++) {
                if (get<2>(inv.get(i)) == codigo) {
                    cout << "El recurso de codigo " << codigo << " se llama " << get<0>(inv.get(i)) << " es de tipo " << get<1>(inv.get(i)) << ", su disponibilidad es " << (get<3>(inv.get(i)) ? "Disponible" : "No disponible") << " y su estado es " << get<4>(inv.get(i)) << endl;
                    return;
                }
            }
            cout << "El recurso con codigo " << codigo << " no se encontro en el inventario." << endl;
        }
        void registrarRecurso(inventario& inv) {
            // Metodo que permite registrar un recurso llamando la función de la clase inventario pushback
            //Se usa la referencia como parametro para que afecte tambien al inventario globalmente y no solo en el metodo, tambien en eliminar recurso
            cout << "Por favor escriba los datos del recurso (nombre, tipo, codigo, disponibilidad, estado)" << endl;
            cin >> nombre >> tipo >> codigo;
            disponibilidad = true;
            estado = "Bueno";
            inv.push_back(make_tuple(nombre, tipo, codigo, disponibilidad, estado));
            cout << "El recurso " << nombre << " ha sido registrado." << endl;
        }
        void eliminarRecurso(inventario& inv) {
            // Metodo que permite eliminar un recurso
            cout << "Escriba el codigo del recurso a eliminar: ";
            string codigo;
            cin >> codigo;
            int tamaño = inv.gettamaño();
            for (int i = 0; i < tamaño; i++) {
                if (get<2>(inv.get(i)) == codigo) {
                    inv.remove(i);
                    cout << "El recurso " << nombre << " ha sido eliminado." << endl;
                    return;
                }
            }
            cout << "El recurso " << nombre << " no se encontro en el inventario." << endl;
            return;
        }

        void devolverRecurso() {
            // Metodo que permite devolver un recurso
            //Implementación pendiente
            cout << "El recurso " << nombre << " ha sido devuelto." << endl;
            disponibilidad = true;
        }

        bool prestarRecurso(string codigoRecurso, inventario& inv) {
            // Metodo que permite prestar un recurso
            // Se crea un objeto Recurso para verificar su disponibilidad, siendo lo que se busca el codigo del recurso.
            int tamaño = inv.gettamaño();
            for (int i = 0; i < tamaño; i++) {
                if (get<2>(inv.get(i)) == codigoRecurso) {
                    disponibilidad = get<3>(inv.get(i));
                    break;
                }
            }
            // Temporalmente solo dice si esta disponible para prestar
            if (!disponibilidad) {
                cout << "El recurso " << nombre << " no esta disponible para prestar." << endl;
                return disponibilidad;
            }
            else {
                cout << "El recurso " << nombre << " esta disponible para prestar." << endl;
            }
            return disponibilidad;
        }
    };


        class Libro : public Recurso {
        private:
            // Se inicializan los atributos de la clase Libro, nombre, tipo, codigo, autor y editorial
            string autor;
        public:
            // Constructor de la clase Libro, se inicializan los atributos de la clase
            Libro(string nombre, string tipo, string codigo, string autor) : Recurso(nombre, tipo, codigo) {
                this->autor = autor;
            }

            //Getters y Setters
            string getAutor() {
                return autor;
            }
            void setAutor(string autor) {
                this->autor = autor;
            }

            void consultarInformacion() {
                // Metodo que permite consultar la informacion de un libro
                cout << "El libro " << getNombre() << " es de tipo " << getTipo() << " y su codigo es " << getCodigo() << "." << endl;
                cout << "El autor del libro es " << autor << "." << endl;
            }
        };

    class Sala : public Recurso {
    private:
        // Se inicializan los atributos de la clase Sala, nombre, tipo, codigo y capacidad
        int capacidad;
    public:
        // Constructor de la clase Sala, se inicializan los atributos de la clase
        Sala(string nombre, string tipo, string codigo, int capacidad) : Recurso(nombre, tipo, codigo) {
            this->capacidad = capacidad;
        }

        //Getters y Setters
        int getCapacidad() {
            return capacidad;
        }
        void setCapacidad(int capacidad) {
            this->capacidad = capacidad;
        }
        void consultarInformacion() {
            // Metodo que permite consultar la informacion de una sala
            cout << "La sala " << getNombre() << " es de tipo " << getTipo() << " y su codigo es " << getCodigo() << "." << endl;
            cout << "La capacidad de la sala es " << capacidad << "." << endl;
        }

        void reservarSala() {
            // Metodo que permite reservar una sala
            // Implementación pendiente para la lógica de reserva de la sala
            if (!getDisponibilidad()) {
                cout << "La sala " << getNombre() << " no esta disponible para reservar." << endl;
                return;
            }
            else {
                cout << "La sala " << getNombre() << " esta disponible para reservar." << endl;
            }
            cout << "La sala " << getNombre() << " ha sido reservada." << endl;
            cout << "Ingrese la fecha y hora de la reserva (formato: dd/mm/aaaa hh:mm): ";
            string fechaHoraReserva;
            cin >> fechaHoraReserva;
            setDisponibilidad(false);
            return;
        }
    };

    class Computador : public Recurso {
    private:
        // Se inicializan los atributos de la clase Computador, nombre, tipo, codigo y sistemaOperativo
        string sistemaOperativo;

    public:
        // Constructor de la clase Computador, se inicializan los atributos de la clase
        Computador(string nombre, string tipo, string codigo, string sistemaOperativo) : Recurso(nombre, tipo, codigo) {
            this->sistemaOperativo = sistemaOperativo;
        }
        //Getters y Setters
        string getSistemaOperativo() {
            return sistemaOperativo;
        }
        void setSistemaOperativo(string sistemaOperativo) {
            this->sistemaOperativo = sistemaOperativo;
        }
        void consultarInformacion() {
            // Metodo que permite consultar la informacion de un computador
            cout << "El computador " << getNombre() << " es de tipo " << getTipo() << " y su codigo es " << getCodigo() << "." << endl;
            cout << "El sistema operativo del computador es " << sistemaOperativo << "." << endl;
        }
        void reservarComputador() {
            // Metodo que permite reservar un computador
            if (!getDisponibilidad()) {
                cout << "El computador " << getNombre() << " no esta disponible para reservar." << endl;
                return;
            }
            else {
                cout << "El computador " << getNombre() << " esta disponible para reservar." << endl;
            }
            cout << "El computador " << getNombre() << " ha sido reservado." << endl;
            cout << "Ingrese la fecha y hora de la reserva (formato: dd/mm/aaaa hh:mm): ";
            string fechaHoraReserva;
            cin >> fechaHoraReserva;
            setDisponibilidad(false);
            return;
        }
        void liberarComputador() {
            // Metodo que permite liberar un computador
            cout << "El computador " << getNombre() << " ha sido liberado." << endl;
            setDisponibilidad(true);
        }
        void solicitarMantenimiento() {
            // Metodo que permite solicitar mantenimiento de un computador
            cout << "El computador " << getNombre() << " ha solicitado mantenimiento." << endl;
            setDisponibilidad(false);
        }
    };