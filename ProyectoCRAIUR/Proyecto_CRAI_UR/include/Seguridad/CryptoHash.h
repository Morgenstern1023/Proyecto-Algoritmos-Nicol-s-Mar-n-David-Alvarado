/**
 * @file CryptoHash.h
 * @brief Wrapper de seguridad sobre OpenSSL (PBKDF2-HMAC-SHA256, CSPRNG y comparación tiempo constante).
 * @author Oscar David Alvarado Navarrete (DA)
 */

#ifndef CRYPTO_HASH_H
#define CRYPTO_HASH_H

#include <iomanip>
#include <iostream>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sstream>
#include <stdexcept>
#include <string>

/**
 * @brief Servicio criptográfico basado en OpenSSL (libcrypto) para credenciales administrativas y tokens.
 */
class CryptoHash {
private:
    static const int ITERACIONES_PBKDF2 = 100000;
    static const int BYTES_HASH = 32;

    static std::string bytesAHex(const unsigned char* datos, size_t len) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            oss << std::setw(2) << static_cast<int>(datos[i]);
        }
        return oss.str();
    }

    static std::string derivarPBKDF2(const std::string& clavePlana, const std::string& salt) {
        unsigned char salida[BYTES_HASH];
        int ok = PKCS5_PBKDF2_HMAC(
            clavePlana.c_str(),
            static_cast<int>(clavePlana.size()),
            reinterpret_cast<const unsigned char*>(salt.c_str()),
            static_cast<int>(salt.size()),
            ITERACIONES_PBKDF2,
            EVP_sha256(),
            BYTES_HASH,
            salida
        );
        if (ok != 1) {
            std::cerr << "[ERROR] [CryptoHash] Fallo en PKCS5_PBKDF2_HMAC de OpenSSL." << std::endl;
            throw std::runtime_error("[ERROR] Fallo en PKCS5_PBKDF2_HMAC de OpenSSL.");
        }
        return bytesAHex(salida, BYTES_HASH);
    }

public:
    static std::string generarAleatorioHex(int longitud = 16) {
        int n = (longitud > 0) ? longitud : 16;
        int numBytes = (n + 1) / 2;
        unsigned char* buffer = new unsigned char[numBytes];
        if (RAND_bytes(buffer, numBytes) != 1) {
            delete[] buffer;
            std::cerr << "[ERROR] [CryptoHash] Fallo al generar entropía con OpenSSL RAND_bytes." << std::endl;
            throw std::runtime_error("[ERROR] Fallo en RAND_bytes de OpenSSL.");
        }
        std::string hex = bytesAHex(buffer, static_cast<size_t>(numBytes));
        delete[] buffer;
        return hex.substr(0, static_cast<size_t>(n));
    }

    static std::string crearCredencial(const std::string& clavePlana) {
        std::string salt = generarAleatorioHex(16);
        std::string hash = derivarPBKDF2(clavePlana, salt);
        return salt + "$" + hash;
    }

    static bool verificarCredencial(const std::string& clavePlana, const std::string& credencialGuardada) {
        size_t posSep = credencialGuardada.find('$');
        if (posSep == std::string::npos || posSep == 0 || posSep + 1 >= credencialGuardada.size()) {
            std::cout << "[WARN] [CryptoHash] Formato de credencial almacenada inválido." << std::endl;
            return false;
        }
        std::string salt = credencialGuardada.substr(0, posSep);
        std::string hashEsperado = credencialGuardada.substr(posSep + 1);
        std::string hashCalculado = derivarPBKDF2(clavePlana, salt);

        if (hashCalculado.size() != hashEsperado.size()) {
            return false;
        }
        return CRYPTO_memcmp(hashCalculado.data(), hashEsperado.data(), hashCalculado.size()) == 0;
    }
};

#endif // CRYPTO_HASH_H
