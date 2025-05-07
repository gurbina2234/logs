#ifndef QUICKSORT_E_HPP
#define QUICKSORT_E_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Lee un bloque de datos desde un archivo binario
 *
 * Esta función lee un bloque de tamaño fijo desde una posición específica
 * en un archivo binario y lo almacena en un buffer.
 *
 * @param filename Nombre del archivo binario
 * @param posicion Índice del bloque a leer
 * @param buffer Vector donde se almacenarán los datos leídos
 */
void readBlock(const std::string &filename, size_t posicion, std::vector<int64_t> &buffer);

/**
 * @brief Escribe un bloque de datos en un archivo binario
 *
 * Esta función escribe un bloque de datos en una posición específica
 * de un archivo binario. Si el archivo no existe, lo crea.
 *
 * @param filename Nombre del archivo binario
 * @param posicion Índice del bloque donde se escribirá
 * @param buffer Vector que contiene los datos a escribir
 */
void writeBlock(const std::string &filename, size_t posicion, std::vector<int64_t> &buffer);

/**
 * @brief Selecciona un conjunto de pivotes aleatorios en un bloque aleatorio de un archivo binario
 *
 * Esta función selecciona valores aleatorios desde un archivo binario
 * para usarlos como pivotes en el algoritmo de QuickSort externo.
 *
 * @param filename Nombre del archivo binario
 * @param N Número total de elementos en el archivo
 * @return Vector con los valores seleccionados ordenados como pivotes
 */
std::vector<int64_t> randomInterval(const std::string &filename, size_t N);

/**
 * @brief Lee múltiples bloques de datos en memoria
 *
 * Esta función lee varios bloques consecutivos desde un archivo binario
 * y los almacena en un buffer.
 *
 * @param filename Nombre del archivo binario
 * @param startBlock Índice del primer bloque a leer
 * @param numBlocks Número de bloques a leer
 * @param buffer Vector donde se almacenarán los datos leídos
 */
void readAllMemory(const std::string &filename,size_t startBlock, size_t numBlocks, std::vector<int64_t> &buffer);

/**
 * @brief Fusiona archivos temporales en un archivo ordenado
 *
 * Esta función combina los datos de varios archivos temporales en un único
 * archivo binario ordenado.
 *
 * @param filenameSorted Nombre del archivo de salida ordenado
 * @param N Número total de elementos a fusionar
 * @param depth Nivel de profundidad en la recursión
 */
void mergeFiles(const std::string &filenameSorted, size_t N, size_t depth);

/**
 * @brief Implementa el algoritmo de QuickSort externo
 *
 * Esta función ordena un archivo binario utilizando el algoritmo de QuickSort
 * externo, dividiendo los datos en subarreglos y fusionándolos recursivamente.
 *
 * @param filename Nombre del archivo binario de entrada
 * @param filenameSorted Nombre del archivo binario de salida ordenado
 * @param N Número total de elementos en el archivo
 * @param depth Nivel de profundidad en la recursión (por defecto 0)
 */
void quicksortExternal(const std::string &filename, const std::string &filenameSorted, size_t N, size_t depth = 0);

/**
 * @brief Reinicia el contador global de lecturas/escrituras
 *
 * Esta función reinicia el contador global que rastrea el número de I/Os
 * realizadas durante la ejecución del programa.
 */
void reiniciar_contador_IOs();

#endif