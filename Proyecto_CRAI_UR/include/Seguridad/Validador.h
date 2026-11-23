/**
 * @file Validador.h
 * @brief Sanitización de entradas, bloqueo de centinelas ("null"/"nullptr"), escape JSON y parseo seguro.
 * @author Oscar David Alvarado Navarrete (DA)
 */

#ifndef VALIDADOR_H
#define VALIDADOR_H

#include "../Estructuras/VectorDinamico.h"
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

class Validador {
public:
    static std::string normalizarTexto(const std::string& valor) {
        std::string s = valor;
        if (s.size() >= 3 &&
            static_cast<unsigned char>(s[0]) == 0xEF &&
            static_cast<unsigned char>(s[1]) == 0xBB &&
            static_cast<unsigned char>(s[2]) == 0xBF) {
            s.erase(0, 3);
        }
        size_t inicio = 0;
        while (inicio < s.size() &&
               (s[inicio] == ' ' || s[inicio] == '\t' || s[inicio] == '\r' || s[inicio] == '\n')) {
            inicio++;
        }
        size_t fin = s.size();
        while (fin > inicio &&
               (s[fin - 1] == ' ' || s[fin - 1] == '\t' || s[fin - 1] == '\r' || s[fin - 1] == '\n')) {
            fin--;
        }
        return s.substr(inicio, fin - inicio);
    }

    static std::string aMinusculas(const std::string& texto) {
        std::string res = texto;
        for (size_t i = 0; i < res.size(); ++i) {
            res[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(res[i])));
        }
        return res;
    }

    static bool esNuloOProhibido(const std::string& texto) {
        std::string limpio = aMinusculas(normalizarTexto(texto));
        return limpio.empty() ||
               limpio == "null" ||
               limpio == "nullptr" ||
               limpio == "none" ||
               limpio == "nil" ||
               limpio == "undefined" ||
               limpio == "nan";
    }

    static std::string sanitizarCampoCSV(const std::string& texto) {
        std::string base = normalizarTexto(texto);
        std::string limpio;
        limpio.reserve(base.size());
        for (size_t i = 0; i < base.size(); ++i) {
            char c = base[i];
            if (c == ',' || c == '\r' || c == '\n' || c == '\t') {
                limpio.push_back(' ');
            } else if (c == '"' || c == '\\') {
                continue;
            } else {
                limpio.push_back(c);
            }
        }
        return normalizarTexto(limpio);
    }

    static std::string sanitizarOpcional(const std::string& texto, const std::string& respaldo = "Sin especificar") {
        std::string limpio = sanitizarCampoCSV(texto);
        if (esNuloOProhibido(limpio)) return respaldo;
        return limpio;
    }

    static std::string escaparJSON(const std::string& texto) {
        std::string out;
        out.reserve(texto.size() + 8);
        for (size_t i = 0; i < texto.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(texto[i]);
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\b': out += "\\b";  break;
                case '\f': out += "\\f";  break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if (c >= 0x20) out.push_back(static_cast<char>(c));
                    break;
            }
        }
        return out;
    }

    static bool parsearEnteroSeguro(const std::string& texto, int& salida, int minVal, int maxVal) {
        std::string limpio = normalizarTexto(texto);
        if (esNuloOProhibido(limpio)) return false;
        char* endptr = nullptr;
        long valor = std::strtol(limpio.c_str(), &endptr, 10);
        if (endptr == limpio.c_str() || *endptr != '\0') return false;
        if (valor < minVal || valor > maxVal) return false;
        salida = static_cast<int>(valor);
        return true;
    }

    static std::string decodificarURL(const std::string& texto) {
        std::string resultado;
        resultado.reserve(texto.size());
        for (size_t i = 0; i < texto.size(); ++i) {
            if (texto[i] == '%' && i + 2 < texto.size()) {
                char hex1 = texto[i + 1];
                char hex2 = texto[i + 2];
                if (std::isxdigit(static_cast<unsigned char>(hex1)) &&
                    std::isxdigit(static_cast<unsigned char>(hex2))) {
                    char byte = 0;
                    if (hex1 >= '0' && hex1 <= '9') byte = (hex1 - '0') << 4;
                    else if (hex1 >= 'A' && hex1 <= 'F') byte = (hex1 - 'A' + 10) << 4;
                    else byte = (hex1 - 'a' + 10) << 4;
                    if (hex2 >= '0' && hex2 <= '9') byte |= (hex2 - '0');
                    else if (hex2 >= 'A' && hex2 <= 'F') byte |= (hex2 - 'A' + 10);
                    else byte |= (hex2 - 'a' + 10);
                    resultado.push_back(byte);
                    i += 2;
                } else {
                    resultado.push_back(texto[i]);
                }
            } else if (texto[i] == '+') {
                resultado.push_back(' ');
            } else {
                resultado.push_back(texto[i]);
            }
        }
        return resultado;
    }

    static bool parsearLongLongSeguro(const std::string& texto, long long& salida, long long minVal, long long maxVal) {
        std::string limpio = normalizarTexto(texto);
        if (esNuloOProhibido(limpio)) return false;
        try {
            size_t consumidos = 0;
            long long valor = std::stoll(limpio, &consumidos);
            if (consumidos != limpio.size()) return false;
            if (valor < minVal || valor > maxVal) return false;
            salida = valor;
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    static bool parsearBooleanoSeguro(const std::string& texto, bool& salida) {
        std::string limpio = aMinusculas(normalizarTexto(texto));
        if (limpio == "1" || limpio == "true" || limpio == "si") {
            salida = true;
            return true;
        }
        if (limpio == "0" || limpio == "false" || limpio == "no") {
            salida = false;
            return true;
        }
        return false;
    }

    static VectorDinamico<std::string> dividirCSV(const std::string& linea) {
        VectorDinamico<std::string> columnas(12);
        std::string actual;
        for (size_t i = 0; i < linea.size(); ++i) {
            if (linea[i] == ',') {
                columnas.push_back(normalizarTexto(actual));
                actual.clear();
            } else {
                actual.push_back(linea[i]);
            }
        }
        columnas.push_back(normalizarTexto(actual));
        return columnas;
    }

    static std::string obtenerTimestampActual() {
        std::time_t ahora = std::time(nullptr);
        std::tm tmLocal;
#ifdef _WIN32
        localtime_s(&tmLocal, &ahora);
#else
        localtime_r(&ahora, &tmLocal);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tmLocal, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

#endif // VALIDADOR_H
