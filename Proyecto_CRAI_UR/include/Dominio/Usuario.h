/**
 * @file Usuario.h
 * @brief Modelo POO de Usuario (Escenario A: sin clave) y Administrador (autenticado con cooldown anti fuerza bruta).
 * @author Nicolás Marín Quintero (NM) & Oscar David Alvarado Navarrete (DA)
 */

#ifndef USUARIO_H
#define USUARIO_H

#include "Recurso.h"
#include "../Estructuras/ListaDoble.h"
#include "../Seguridad/Validador.h"
#include <ctime>
#include <sstream>
#include <string>

class Usuario {
protected:
    std::string documento;
    std::string nombre;
    std::string apellido;
    bool sancionado;
    ListaDoble<RegistroHistorial> historial;

public:
    Usuario(
        const std::string& doc,
        const std::string& nom,
        const std::string& ape,
        bool sanc = false
    ) : documento(doc), nombre(nom), apellido(ape), sancionado(sanc), historial() {}

    virtual ~Usuario() {}
    virtual bool esAdmin() const { return false; }

    std::string getDocumento() const { return documento; }
    std::string getNombre() const { return nombre; }
    std::string getApellido() const { return apellido; }
    bool isSancionado() const { return sancionado; }

    void setNombre(const std::string& nom) { nombre = nom; }
    void setApellido(const std::string& ape) { apellido = ape; }
    void setSancionado(bool sanc) { sancionado = sanc; }

    void agregarAlHistorial(const RegistroHistorial& reg) { historial.insertarFinal(reg); }
    VectorDinamico<RegistroHistorial> obtenerHistorialAdelante() const { return historial.obtenerAdelante(); }
    VectorDinamico<RegistroHistorial> obtenerHistorialAtras() const { return historial.obtenerAtras(); }

    virtual std::string aJSON() const {
        std::ostringstream oss;
        oss << "{\"documento\":\"" << Validador::escaparJSON(documento)
            << "\",\"nombre\":\"" << Validador::escaparJSON(nombre)
            << "\",\"apellido\":\"" << Validador::escaparJSON(apellido)
            << "\",\"sancionado\":" << (sancionado ? "true" : "false")
            << ",\"esAdmin\":false}";
        return oss.str();
    }
};

/**
 * @brief Recepcionista / Administrador del CRAI (RQF 020).
 *
 * Implementa bloqueo temporal (cooldown) tras 3 intentos fallidos consecutivos
 * mediante marca de tiempo epoch `bloqueadoHasta`, y permite desbloqueo de emergencia
 * exclusivamente por terminal local (`--unlock`).
 */
class Administrador : public Usuario {
private:
    static const int MAX_INTENTOS_FALLIDOS = 3;

    std::string credencialHash; ///< Formato "salt$pbkdf2_sha256".
    int intentosFallidos;
    long long bloqueadoHasta;   ///< Timestamp UNIX (segundos) hasta el que dura el cooldown (0 = libre).
    int totalOperaciones;

public:
    Administrador(
        const std::string& doc,
        const std::string& nom,
        const std::string& ape,
        const std::string& hashGuardado,
        int intentos = 0,
        long long bloqHasta = 0,
        int ops = 0
    ) : Usuario(doc, nom, ape, false),
        credencialHash(hashGuardado),
        intentosFallidos(intentos >= 0 ? intentos : 0),
        bloqueadoHasta(bloqHasta >= 0 ? bloqHasta : 0),
        totalOperaciones(ops >= 0 ? ops : 0) {}

    bool esAdmin() const override { return true; }

    std::string getCredencialHash() const { return credencialHash; }
    void setCredencialHash(const std::string& nuevoHash) { credencialHash = nuevoHash; }

    int getIntentosFallidos() const { return intentosFallidos; }
    long long getBloqueadoHasta() const { return bloqueadoHasta; }
    int getTotalOperaciones() const { return totalOperaciones; }

    bool estaEnCooldown(std::time_t ahora) const {
        return bloqueadoHasta > static_cast<long long>(ahora);
    }

    long long segundosRestantesCooldown(std::time_t ahora) const {
        long long diff = bloqueadoHasta - static_cast<long long>(ahora);
        return (diff > 0) ? diff : 0;
    }

    /**
     * @brief Registra un fallo de autenticación y activa cooldown al llegar al 3er intento fallido.
     * @return true si el cooldown fue activado en este intento.
     */
    bool registrarIntentoFallido(std::time_t ahora, long long duracionCooldownSeg) {
        intentosFallidos++;
        if (intentosFallidos >= MAX_INTENTOS_FALLIDOS) {
            bloqueadoHasta = static_cast<long long>(ahora) + (duracionCooldownSeg > 0 ? duracionCooldownSeg : 60);
            intentosFallidos = 0;
            return true;
        }
        return false;
    }

    void reiniciarIntentos() {
        intentosFallidos = 0;
        bloqueadoHasta = 0;
    }

    void registrarOperacion() { totalOperaciones++; }

    std::string aJSON() const override {
        std::time_t ahora = std::time(nullptr);
        std::ostringstream oss;
        oss << "{\"documento\":\"" << Validador::escaparJSON(documento)
            << "\",\"nombre\":\"" << Validador::escaparJSON(nombre)
            << "\",\"apellido\":\"" << Validador::escaparJSON(apellido)
            << "\",\"sancionado\":" << (sancionado ? "true" : "false")
            << ",\"esAdmin\":true"
            << ",\"enCooldown\":" << (estaEnCooldown(ahora) ? "true" : "false")
            << ",\"segundosRestantesCooldown\":" << segundosRestantesCooldown(ahora)
            << ",\"intentosFallidos\":" << intentosFallidos
            << ",\"totalOperaciones\":" << totalOperaciones << "}";
        return oss.str();
    }
};

#endif // USUARIO_H
