#ifndef MERGESORT_E_HPP
#define MERGESORT_E_HPP

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

/**
 * CONSTANTES DE CONFIGURACIÓN
 */
#define B_BYTES 4096               // Tamaño de bloque en bytes
#define M_BYTES (50 * 1024 * 1024) // Tamaño total de RAM disponible
#define RESERVA_RAM                                                            \
  (48 * 1024 * 1024) // Tamaño de RAM a reservar para el buffer principal
#define NUMS_POR_BLOQUE                                                        \
  (B_BYTES / sizeof(int64_t)) // Cantidad de números por bloque (512)
#define BLOQUES_EN_RAM                                                         \
  (RESERVA_RAM / B_BYTES) // Número de bloques que caben en RAM
#define NUMS_EN_RAM                                                            \
  (BLOQUES_EN_RAM * NUMS_POR_BLOQUE) // Números totales que caben en RAM
#define ARIDAD 192 // Aridad del merge (archivos a fusionar simultáneamente)

/**
 * Estructura que representa un archivo intermedio (mid file)
 * Contiene un buffer para almacenar un bloque de datos y metadatos para su
 * procesamiento
 */
typedef struct {
  FILE *archivo; // Puntero al archivo mid_#.bin
  int64_t
      buffer[NUMS_POR_BLOQUE]; // Buffer de 1 bloque (512 números de 64 bits)
  size_t pos;                  // Posición actual dentro del buffer
  size_t usados;               // Cantidad de elementos válidos en el buffer
  bool terminado;              // Indica si ya se procesó todo el archivo
} Mid;

/**
 * Función de comparación para qsort
 * Compara dos números de 64 bits para determinar su orden
 * @param a Puntero al primer número
 * @param b Puntero al segundo número
 * @return 1 si a>b, -1 si a<b, 0 si son iguales
 */
int comparar_64(const void *a, const void *b);

/**
 * Primera fase del algoritmo: divide el archivo grande en archivos intermedios
 * ordenados Lee bloques del archivo original, los ordena en memoria y los
 * escribe en archivos temporales
 * @param datos_desordenados Ruta al archivo con datos desordenados
 * @param cantidad_mid_files Puntero donde se almacenará la cantidad de archivos
 * intermedios generados
 */
void generar_middle_files(char *datos_desordenados, size_t *cantidad_mid_files);

/**
 * Actualiza el buffer de un archivo intermedio
 * Lee el siguiente bloque del archivo y actualiza los metadatos
 * @param mid Puntero a la estructura Mid que se actualizará
 */
void update_mf_buffer(Mid *mid);

/**
 * Selecciona el valor mínimo entre los primeros elementos de cada buffer
 * Parte fundamental del algoritmo de merge k-way
 * @param mids Array de estructuras Mid
 * @param cantidad_mids Cantidad de archivos intermedios
 * @return Índice del buffer que contiene el valor mínimo, o -1 si todos están
 * vacíos
 */
int seleccionar_minimo(Mid *mids, size_t cantidad_mids);

/**
 * Realiza el merge de una tanda de archivos intermedios
 * @param inicio Índice del primer archivo a procesar
 * @param cantidad Cantidad de archivos a mergear
 * @param nombre_salida Nombre del archivo de salida
 */
void merge_tanda(int inicio, int cantidad, const char *nombre_salida);

/**
 * Realiza el merge de múltiples archivos intermedios en grupos de ARIDAD
 * Implementa un merge jerárquico cuando hay demasiados archivos
 * @param cantidad Cantidad total de archivos intermedios
 * @param cuociente Número de grupos completos de ARIDAD
 * @param residuo Archivos restantes que no completan un grupo
 * @param cuantosM Tamaño del archivo en M (M = 50MB) (para nombrar archivos)
 * @param cantidadSupMid Puntero donde se almacenará la cantidad de archivos
 * superiores
 * @param iteracion Número de iteración actual
 */
void merge_cant_sobre_a(
    size_t cantidad,
    int cuociente,
    int residuo,
    int cuantosM,
    int *cantidadSupMid,
    int iteracion
);

/**
 * Función principal de merge que decide la estrategia según la cantidad de
 * archivos
 * @param cantidad Cantidad de archivos intermedios
 * @param cuantosM Tamaño del archivo en M (M = 50MB)
 * @param cantidadSupMid Puntero donde se almacenará la cantidad de archivos
 * superiores
 * @param iteracion Número de iteración actual
 */
void merge_middle_files(
    size_t cantidad,
    int cuantosM,
    int *cantidadSupMid,
    int iteracion
);

/**
 * Elimina los archivos temporales generados durante el proceso
 * @param cantidad_mids Cantidad de archivos intermedios
 * @param cantidad_sup_mids Cantidad de archivos superiores
 * @param cuantosM Tamaño del archivo en M (M = 50MB)
 */
void borrar_middle_files(
    size_t cantidad_mids,
    int cantidad_sup_mids,
    int cuantosM
);

#endif // MERGESORT_E_HPP