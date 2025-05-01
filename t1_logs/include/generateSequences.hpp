#ifdef GENERATE_SEQUENCES_HPP
#define GENERATE_SEQUENCES_HPP

#include <string>
#include <cstdint>

/// @brief Genera 5 secuencias de números al azar y las almacena en un archivos binarios.
/// @param N Tamaño del archivo en bytes
/// @param filename Archivo de salida.
void generateSequences(int64_t N, const std::string &filename);

#endif