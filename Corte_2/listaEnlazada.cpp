// Prototipo o borrador de la lista enlazada basado en el ejemplo de e-aulas, se modificara e implementara luego de verse en el segundo corte

//crear una lista enlazada para Préstamos activos, con alta y baja frecuencia de operaciones
#include <iostream>
using namespace std;
// Se usa el template para crear una lista enlazada genérica
//Clase nodo que almacena un dato y un puntero al siguiente nodo
template <typename T>
class Nodo{
private:
// Atributos del nodo: dato y puntero al siguiente nodo
  T dato;
  Nodo<T>* next;
public: 
    // Constructor por defecto
    Nodo(){
        next = NULL;
    }
    
    // Constructor con parámetro
    Nodo(T d){
        dato = d;
        next = NULL;
    }
   //Da el dato almacenado en el nodo
    T get_dato(){
        return dato;
    }
    
    //Modifica el dato almacenado en el nodo
    void set_dato(T d){
        dato = d;
    }
    
    //Devuelve el puntero al siguiente nodo
    Nodo<T>* get_next(){
        return next;
    }
    
    //Modifica el puntero al siguiente nodo
    void set_next(Nodo<T>* n){
        next = n;
    }
    // Sobrecarga del operador de asignación y conversión a cadena de texto para facilitar la manipulación de nodos
    // Sobrecarga del operador de asignación
    Nodo<T>& operator=(const Nodo<T>& f) { 
        if (this == &f) {
            return *this;
        }
        // Copia el dato del nodo fuente y reinicia el puntero al siguiente nodo
        dato = f.dato;
        next = nullptr;
        // Copia la memoria recursivamente si el nodo siguiente no es nulo
        if (f.next != nullptr) {
            next = new Nodo<T>(*f.next);
        }
        return *this;
    }
    // Convierte el nodo a una cadena de texto
	string to_string() {
		return to_string(dato);
	}
    // Sobrecarga del operador de inserción en flujo de salida, en palabras simples permite escribir el nodo con cout
	friend std::ostream& operator<<(std::ostream& os, Nodo<T>& b) {
		return os << b.to_string();
	}
    
};

template <typename T>
// Clase lista enlazada genérica que almacena nodos de tipo T
class Lista{
private:
    // Atributos de la lista: puntero al primer nodo y tamaño de la lista
  Nodo<T>* ptr;
  int size;
public:
    // Constructor por defecto que inicializa la lista vacía
  Lista(){
      ptr = NULL;
      size = 0;
  }
  
    // Destructor que libera la memoria de todos los nodos de la lista  
  ~Lista(){
      if(ptr != NULL){
          Nodo<T>* temp =  ptr;
          Nodo<T>* next_temp =  ptr->get_next();
          
          while(next_temp != NULL){
              delete temp;
              temp = next_temp;
              next_temp =  temp->get_next();
          }
          delete temp;
      }
  }
  //Función que agrega un nuevo nodo al final de la lista
  void add(T d){
      Nodo<T>* n = new Nodo<T>(d);
      if(ptr == NULL){
          ptr =  n;
      }else{
          Nodo<T>* t = ptr;
          while(t->get_next() != NULL){
                t = t->get_next();
          }
          t->set_next(n);
      }
      size++;
  }

  //Función que imprime todos los nodos de la lista
  void print(){
      Nodo<T>* t = ptr;
      while(t != NULL){
          cout<<(*t)<<endl;
          t = t->get_next();
      }
  }
  
  //Función que inserta un nuevo nodo en una posición específica de la lista
  void insert(T dato, int pos){
      Nodo<T>* t = ptr;
      Nodo<T>* new_nodo = new Nodo<T>(dato);
      
      if(ptr == NULL || pos <= 0){
          new_nodo->set_next(ptr);
          ptr = new_nodo;
          size++;
      }else if(pos >= size){
          add(dato);
      }else{
          Nodo<T>* t = ptr;
          int i=0;
          while(i < pos-1){
                t = t->get_next();
                i++;
          }
          new_nodo->set_next(t->get_next());
          t->set_next(new_nodo);
          size++;
      }
      
  }
  
    
};