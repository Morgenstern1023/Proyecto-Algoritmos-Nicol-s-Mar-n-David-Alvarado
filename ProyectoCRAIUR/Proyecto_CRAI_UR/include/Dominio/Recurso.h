/**
 * @file Recurso.h
 * @brief Jerarquía POO polimórfica de recursos del CRAI (Recurso -> Libro, Computador, Sala).
 * @author Nicolás Marín Quintero (NM)
 */

#ifndef RECURSO_H
#define RECURSO_H

#include "../Estructuras/Cola.h"
#include "../Estructuras/ListaDoble.h"
#include "../Estructuras/VectorDinamico.h"
#include "../Seguridad/Validador.h"
#include <sstream>
#include <string>

struct RegistroHistorial {
    std::string fechaHora;
    std::string tipoEvento;
    std::string codigoRecurso;
    std::string documentoUsuario;
    std::string detalle;

    RegistroHistorial(
        const std::string& fecha = "",
        const std::string& tipo = "",
        const std::string& codRec = "",
        const std::string& docUsu = "",
        const std::string& det = ""
    ) : fechaHora(fecha), tipoEvento(tipo), codigoRecurso(codRec),
        documentoUsuario(docUsu), detalle(det) {}

    std::string aJSON() const {
        std::ostringstream oss;
        oss << "{\"fechaHora\":\"" << Validador::escaparJSON(fechaHora)
            << "\",\"tipoEvento\":\"" << Validador::escaparJSON(tipoEvento)
            << "\",\"codigoRecurso\":\"" << Validador::escaparJSON(codigoRecurso)
            << "\",\"documentoUsuario\":\"" << Validador::escaparJSON(documentoUsuario)
            << "\",\"detalle\":\"" << Validador::escaparJSON(detalle) << "\"}";
        return oss.str();
    }
};

class Recurso {
protected:
    std::string codigo;
    std::string nombre;
    std::string categoria;
    std::string subcategoria;
    std::string estado;
    bool disponible;
    int sedeId;
    int anioAdquisicion;
    int vecesPrestado;
    Cola<std::string> colaEspera;
    ListaDoble<RegistroHistorial> historial;

    std::string baseJSON() const {
        std::ostringstream oss;
        oss << "\"codigo\":\"" << Validador::escaparJSON(codigo)
            << "\",\"nombre\":\"" << Validador::escaparJSON(nombre)
            << "\",\"categoria\":\"" << Validador::escaparJSON(categoria)
            << "\",\"subcategoria\":\"" << Validador::escaparJSON(subcategoria)
            << "\",\"estado\":\"" << Validador::escaparJSON(estado)
            << "\",\"disponible\":" << (disponible ? "true" : "false")
            << ",\"sedeId\":" << sedeId
            << ",\"anioAdquisicion\":" << anioAdquisicion
            << ",\"vecesPrestado\":" << vecesPrestado
            << ",\"enEspera\":" << colaEspera.getTamano();
        return oss.str();
    }

public:
    Recurso(
        const std::string& cod,
        const std::string& nom,
        const std::string& cat,
        const std::string& subcat,
        const std::string& est,
        bool disp,
        int sede,
        int anio,
        int prestamos
    ) : codigo(cod), nombre(nom), categoria(cat), subcategoria(subcat),
        estado(est), disponible(disp), sedeId(sede),
        anioAdquisicion(anio), vecesPrestado(prestamos),
        colaEspera(), historial() {}

    virtual ~Recurso() {}

    virtual std::string getTipoEspecifico() const = 0;
    virtual std::string getDetalleExtra() const = 0;
    virtual void setDetalleExtra(const std::string& detalle) = 0;
    virtual std::string aJSON() const = 0;

    std::string getCodigo() const { return codigo; }
    std::string getNombre() const { return nombre; }
    std::string getCategoria() const { return categoria; }
    std::string getSubcategoria() const { return subcategoria; }
    std::string getEstado() const { return estado; }
    bool isDisponible() const { return disponible; }
    int getSedeId() const { return sedeId; }
    int getAnioAdquisicion() const { return anioAdquisicion; }
    int getVecesPrestado() const { return vecesPrestado; }

    void setNombre(const std::string& nom) { nombre = nom; }
    void setSubcategoria(const std::string& subcat) { subcategoria = subcat; }
    void setEstado(const std::string& est) { estado = est; }
    void setDisponible(bool disp) { disponible = disp; }
    void setSedeId(int sede) { sedeId = sede; }
    void setAnioAdquisicion(int anio) { anioAdquisicion = anio; }
    void setVecesPrestado(int v) { vecesPrestado = (v >= 0) ? v : 0; }
    void incrementarPrestamos() { vecesPrestado++; }
    void decrementarPrestamos() { if (vecesPrestado > 0) vecesPrestado--; }

    bool estaEnColaEspera(const std::string& docUsuario) const {
        Cola<std::string> copia(colaEspera);
        std::string actual;
        while (copia.desencolar(actual)) {
            if (actual == docUsuario) return true;
        }
        return false;
    }

    bool encolarUsuarioEspera(const std::string& docUsuario) {
        if (Validador::esNuloOProhibido(docUsuario) || estaEnColaEspera(docUsuario)) {
            return false;
        }
        colaEspera.encolar(docUsuario);
        return true;
    }

    bool desencolarSiguienteEspera(std::string& docSiguiente) {
        return colaEspera.desencolar(docSiguiente);
    }

    void reencolarAlFrenteEspera(const std::string& docUsuario) {
        Cola<std::string> nuevaCola;
        nuevaCola.encolar(docUsuario);
        std::string tmp;
        while (colaEspera.desencolar(tmp)) {
            nuevaCola.encolar(tmp);
        }
        colaEspera = nuevaCola;
    }

    int getTamanoColaEspera() const { return colaEspera.getTamano(); }
    bool tieneColaEspera() const { return !colaEspera.estaVacia(); }

    VectorDinamico<std::string> obtenerColaEsperaOrdenada() const {
        VectorDinamico<std::string> res(colaEspera.getTamano() > 0 ? colaEspera.getTamano() : 4);
        Cola<std::string> copia(colaEspera);
        std::string doc;
        while (copia.desencolar(doc)) res.push_back(doc);
        return res;
    }

    void agregarAlHistorial(const RegistroHistorial& reg) { historial.insertarFinal(reg); }
    VectorDinamico<RegistroHistorial> obtenerHistorialAdelante() const { return historial.obtenerAdelante(); }
    VectorDinamico<RegistroHistorial> obtenerHistorialAtras() const { return historial.obtenerAtras(); }
};

class Libro : public Recurso {
private:
    std::string autor;

public:
    Libro(
        const std::string& cod, const std::string& nom, const std::string& subcat,
        const std::string& est, bool disp, int sede, int anio, int prestamos,
        const std::string& aut = "Autor institucional"
    ) : Recurso(cod, nom, "libro", subcat, est, disp, sede, anio, prestamos),
        autor(Validador::sanitizarOpcional(aut, "Autor institucional")) {}

    std::string getTipoEspecifico() const override { return "Libro"; }
    std::string getDetalleExtra() const override { return autor; }
    void setDetalleExtra(const std::string& detalle) override {
        autor = Validador::sanitizarOpcional(detalle, "Autor institucional");
    }

    std::string aJSON() const override {
        std::ostringstream oss;
        oss << "{" << baseJSON()
            << ",\"tipoEspecifico\":\"Libro\""
            << ",\"detalleExtra\":\"" << Validador::escaparJSON(autor) << "\"}";
        return oss.str();
    }
};

class Computador : public Recurso {
private:
    std::string sistemaOperativo;

public:
    Computador(
        const std::string& cod, const std::string& nom, const std::string& subcat,
        const std::string& est, bool disp, int sede, int anio, int prestamos,
        const std::string& so = "Linux Arch/Ubuntu"
    ) : Recurso(cod, nom, "computador", subcat, est, disp, sede, anio, prestamos),
        sistemaOperativo(Validador::sanitizarOpcional(so, "Linux Arch/Ubuntu")) {}

    std::string getTipoEspecifico() const override { return "Computador"; }
    std::string getDetalleExtra() const override { return sistemaOperativo; }
    void setDetalleExtra(const std::string& detalle) override {
        sistemaOperativo = Validador::sanitizarOpcional(detalle, "Linux Arch/Ubuntu");
    }

    std::string aJSON() const override {
        std::ostringstream oss;
        oss << "{" << baseJSON()
            << ",\"tipoEspecifico\":\"Computador\""
            << ",\"detalleExtra\":\"" << Validador::escaparJSON(sistemaOperativo) << "\"}";
        return oss.str();
    }
};

class Sala : public Recurso {
private:
    int capacidadPersonas;

public:
    Sala(
        const std::string& cod, const std::string& nom, const std::string& subcat,
        const std::string& est, bool disp, int sede, int anio, int prestamos,
        int capacidad = 6
    ) : Recurso(cod, nom, "sala", subcat, est, disp, sede, anio, prestamos),
        capacidadPersonas(capacidad > 0 ? capacidad : 6) {}

    std::string getTipoEspecifico() const override { return "Sala"; }
    std::string getDetalleExtra() const override { return std::to_string(capacidadPersonas); }
    void setDetalleExtra(const std::string& detalle) override {
        int cap = 6;
        if (Validador::parsearEnteroSeguro(detalle, cap, 1, 200)) capacidadPersonas = cap;
    }

    int getCapacidadPersonas() const { return capacidadPersonas; }

    std::string aJSON() const override {
        std::ostringstream oss;
        oss << "{" << baseJSON()
            << ",\"tipoEspecifico\":\"Sala\""
            << ",\"detalleExtra\":\"" << capacidadPersonas << "\"}";
        return oss.str();
    }
};

#endif // RECURSO_H
