#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <cstring>
#include <vector>
#include "mergesort_e.hpp"

/**
 * CONSTANTES DE CONFIGURACIÓN
 */
#define B_BYTES 4096 //tamaño bloque
#define M_BYTES (50 * 1024 * 1024) //tamaño RAM
#define RESERVA_RAM (48 * 1024 * 1024) //tamaño de RAM a reservar
#define NUMS_POR_BLOQUE (B_BYTES / sizeof(int64_t)) // = 512 | cant de numeros por bloque
#define BLOQUES_EN_RAM (RESERVA_RAM / B_BYTES) //numero de bloques posibles en RAM (con reserva)
#define NUMS_EN_RAM (BLOQUES_EN_RAM * NUMS_POR_BLOQUE) // = 6.291.456 numeros
#define ARIDAD 192 //aridad (numero de archivos a mergear)

// Contador global de operaciones de lectura/escritura para medir rendimiento
size_t lecturas_escrituras = 0;


/**
 * Función de comparación para qsort
 * Compara dos números de 64 bits para determinar su orden
 * @param a Puntero al primer número
 * @param b Puntero al segundo número
 * @return 1 si a>b, -1 si a<b, 0 si son iguales
 */
int comparar_64(const void *a, const void *b) {
    int64_t x = *(int64_t*)a;
    int64_t y = *(int64_t*)b;
    
    return (x > y) - (x < y);
}

/**
 * Primera fase del algoritmo: divide el archivo grande en archivos intermedios ordenados
 * Lee bloques del archivo original, los ordena en memoria y los escribe en archivos temporales
 * @param datos_desordenados Ruta al archivo con datos desordenados
 * @param cantidad_mid_files Puntero donde se almacenará la cantidad de archivos intermedios generados
 */
void generar_middle_files(char *datos_desordenados, size_t *cantidad_mid_files) {
    FILE *f_in = fopen(datos_desordenados, "rb");
    if (!f_in) {
        perror("abrir datos");
        exit(1);
    }
    int64_t *buffer = (int64_t*)malloc(RESERVA_RAM);
    if (!buffer) {
        perror("malloc");
        exit(1);
    }

    size_t mid_num = 0;
    
    while (1) {
        size_t total_leidos = 0;

        //traemos a RAM todos los bloques posibles
        while (total_leidos < NUMS_EN_RAM) { //mientras todavia quede espacio en ram
            size_t offset = total_leidos;
            size_t leidos = fread(buffer + offset, B_BYTES, 1, f_in); //leer 1 bloque

            if (leidos == 0) break; //llegamos al final del archivo

            total_leidos += NUMS_POR_BLOQUE; //leimos un bloque (512 elementos)
            lecturas_escrituras++; //+1 I/O
        }

        if (total_leidos == 0) break; //no se leyo ningun bloque

        qsort(buffer, total_leidos, sizeof(int64_t), comparar_64); //ordeno en ram

        char nombre_mid[64];
        snprintf(nombre_mid, sizeof(nombre_mid), "mid_%zu.bin", mid_num++);
        FILE *f_out = fopen(nombre_mid,"wb");
        if (!f_out) {
            perror("crear mid");
            exit(1);
        }

        size_t total_escritos = 0;

        //escribimos en disco el arreglo que acabamos de ordenar
        while (total_escritos < total_leidos) {
            size_t offset = total_escritos;
            size_t escritos = fwrite(buffer + offset, B_BYTES, 1, f_out); //un bloque a la vez

            total_escritos += NUMS_POR_BLOQUE; //se escribio un bloque (512 elementos)
            lecturas_escrituras++; //+1 I/O
        }

        fclose(f_out);
    }

    *cantidad_mid_files = mid_num;
    free(buffer);
    fclose(f_in);
}

/**
 * Actualiza el buffer de un archivo intermedio
 * Lee el siguiente bloque del archivo y actualiza los metadatos
 * @param mid Puntero a la estructura Mid que se actualizará
 */
void update_mf_buffer(Mid *mid) {
    if (mid->terminado) return;

    size_t leidos = fread(mid->buffer, B_BYTES, 1, mid->archivo);
    lecturas_escrituras++;

    if (leidos == 0) {
        mid->terminado = true;
        return;
    }

    mid->usados = NUMS_POR_BLOQUE;
    mid->pos = 0; //reiniciamos su posicion
}

/**
 * Selecciona el valor mínimo entre los primeros elementos de cada buffer
 * Parte fundamental del algoritmo de merge k-way
 * @param mids Array de estructuras Mid
 * @param cantidad_mids Cantidad de archivos intermedios
 * @return Índice del buffer que contiene el valor mínimo, o -1 si todos están vacíos
 */
int seleccionar_minimo(Mid *mids, size_t cantidad_mids) {
    int idx = -1;
    int64_t val_min = 0;

    //compara todos los primeros elementos de cada buffer
    for (int i = 0; i < cantidad_mids; i++) {
        if (mids[i].terminado || mids[i].pos >= mids[i].usados) continue;
        int64_t actual = mids[i].buffer[mids[i].pos];
        if (idx == -1 || actual < val_min) {
            val_min = actual;
            idx = i;
        }
    }

    return idx;
}

/**
 * Realiza el merge de una tanda de archivos intermedios
 * @param inicio Índice del primer archivo a procesar
 * @param cantidad Cantidad de archivos a mergear
 * @param nombre_salida Nombre del archivo de salida
 */
void merge_tanda(int inicio, int cantidad, const char* nombre_salida) {
    std::vector<Mid> mids(cantidad);
    int64_t buffer_a_disco[NUMS_POR_BLOQUE];
    size_t elementos_escritos = 0;

    //abrir archivos mid
    for (int i = 0; i < cantidad; i++) {
        int idx_archivo = inicio + i;
        char nombre_archivo[64];
        snprintf(nombre_archivo, sizeof(nombre_archivo), "mid_%d.bin", idx_archivo);

        mids[i].archivo = fopen(nombre_archivo, "rb");
        if (!mids[i].archivo) {
            perror("abrir mid file");
            exit(1);
        }

        mids[i].pos = 0;
        mids[i].usados = 0;
        mids[i].terminado = false;
        update_mf_buffer(&mids[i]);
    }

    //escribir superior mids
    FILE* f_out = fopen(nombre_salida, "wb");
    if (!f_out) {
        perror("crear archivo salida");
        exit(1);
    }

    while (1) {
        int idx = seleccionar_minimo(mids.data(), cantidad);
        if (idx == -1) break;

        buffer_a_disco[elementos_escritos++] = mids[idx].buffer[mids[idx].pos++];

        if (elementos_escritos == NUMS_POR_BLOQUE) {
            fwrite(buffer_a_disco, B_BYTES, 1, f_out);
            lecturas_escrituras++;
            elementos_escritos = 0;
        }

        if (mids[idx].pos >= mids[idx].usados) {
            update_mf_buffer(&mids[idx]);
        }
    }

    for (int i = 0; i < cantidad; i++) {
        fclose(mids[i].archivo);
    }

    fclose(f_out);
}

/**
 * Realiza el merge de múltiples archivos intermedios en grupos de ARIDAD
 * Implementa un merge jerárquico cuando hay demasiados archivos
 * @param cantidad Cantidad total de archivos intermedios
 * @param cuociente Número de grupos completos de ARIDAD
 * @param residuo Archivos restantes que no completan un grupo
 * @param cuantosM Tamaño del archivo en M (M = 50MB) (para nombrar archivos)
 * @param cantidadSupMid Puntero donde se almacenará la cantidad de archivos superiores
 * @param iteracion Número de iteración actual
 */
void merge_cant_sobre_a(size_t cantidad, int cuociente, int residuo, int cuantosM, int *cantidadSupMid, int iteracion) {
    for (int index = 0; index < cuociente; index++) {
        *cantidadSupMid = cuociente + (residuo > 0);
        int offset = index * ARIDAD;
        char nombre_salida[64];
        snprintf(nombre_salida, sizeof(nombre_salida), "superior_mid_%dM_%d.bin", cuantosM, index);

        merge_tanda(offset, ARIDAD, nombre_salida);
    }

    if (residuo > 0) {
        int offset = cuociente * ARIDAD;
        char nombre_salida[64];
        snprintf(nombre_salida, sizeof(nombre_salida), "superior_mid_%dM_%d.bin", cuantosM, cuociente);

        merge_tanda(offset, residuo, nombre_salida);
    }

    std::vector<Mid> sup_mids(cuociente + (residuo > 0));
    int64_t buffer_a_disco[NUMS_POR_BLOQUE];
    size_t elementos_escritos = 0;

    for (int i = 0; i < (cuociente + (residuo > 0)); i++) {
        char nombre_superior_mid[64];
        snprintf(nombre_superior_mid, sizeof(nombre_superior_mid), "superior_mid_%dM_%d.bin", cuantosM, i);
        sup_mids[i].archivo = fopen(nombre_superior_mid, "rb");
        if (!sup_mids[i].archivo) {
            perror("abrir superior mid");
            exit(1);
        }
        update_mf_buffer(&sup_mids[i]);
    }

    char nombre_archivo_ordenado[64];
    snprintf(nombre_archivo_ordenado, sizeof(nombre_archivo_ordenado), "orden_%dM_%d.bin", cuantosM, iteracion);    FILE *f_out = fopen(nombre_archivo_ordenado, "wb");
    if (!f_out) {
        perror("crear orden final");
        exit(1);
    }

    while (1) {
        int idx = seleccionar_minimo(sup_mids.data(), cuociente + (residuo > 0));
        if (idx == -1) break;

        buffer_a_disco[elementos_escritos++] = sup_mids[idx].buffer[sup_mids[idx].pos++];

        if (elementos_escritos == NUMS_POR_BLOQUE) {
            fwrite(buffer_a_disco, B_BYTES, 1, f_out);
            lecturas_escrituras++;
            elementos_escritos = 0;
        }

        if (sup_mids[idx].pos >= sup_mids[idx].usados) {
            update_mf_buffer(&sup_mids[idx]);
        }
    }

    //cerrar
    for (int i = 0; i < (cuociente + (residuo > 0)); i++) {
        fclose(sup_mids[i].archivo);
    }

    fclose(f_out);
}

/**
 * Función principal de merge que decide la estrategia según la cantidad de archivos
 * @param cantidad Cantidad de archivos intermedios
 * @param cuantosM Tamaño del archivo en MB
 * @param cantidadSupMid Puntero donde se almacenará la cantidad de archivos superiores
 * @param iteracion Número de iteración actual
 */
void merge_middle_files(size_t cantidad, int cuantosM, int *cantidadSupMid, int iteracion) {
    if (cantidad > ARIDAD) {
        printf("cantidad: %zu | aridad: %d", cantidad, ARIDAD);
        int cuociente = cantidad / ARIDAD;
        int residuo = cantidad % ARIDAD;
        merge_cant_sobre_a(cantidad, cuociente, residuo, cuantosM, cantidadSupMid, iteracion);
        return;
    }

    Mid mids[ARIDAD];
    int64_t buffer_a_disco[NUMS_POR_BLOQUE]; //se escribe 1 bloque a la vez
    size_t elementos_escritos = 0;

    for (int i = 0; i < cantidad; i++) {
        char nombre_mid[64];
        snprintf(nombre_mid, sizeof(nombre_mid), "mid_%d.bin", i);
        mids[i].archivo = fopen(nombre_mid, "rb");
        if (!mids[i].archivo) {
            perror("abrir mid file");
            exit(1);
        }
        update_mf_buffer(&mids[i]);
    }

    char nombre_archivo_ordenado[64];
    snprintf(nombre_archivo_ordenado, sizeof(nombre_archivo_ordenado), "orden_%dM_%d.bin", cuantosM, iteracion);    FILE *f_out = fopen(nombre_archivo_ordenado, "wb");
    if (!f_out) {
        perror("crear orden final");
        exit(1);
    }

    while (1) {
        int idx = seleccionar_minimo(mids, cantidad);
        if (idx == -1) break; //todos estan vacios

        buffer_a_disco[elementos_escritos++] = mids[idx].buffer[mids[idx].pos++]; //copio el menor primer elemento y muevo la posicion de su puntero

        if (elementos_escritos == NUMS_POR_BLOQUE) {
            fwrite(buffer_a_disco, B_BYTES, 1, f_out);
            lecturas_escrituras++;
            elementos_escritos = 0;
        }

        if (mids[idx].pos >= mids[idx].usados) { //agotamos los elementos del buffer
            update_mf_buffer(&mids[idx]); //actualizamos los valores de su buffer
        }
    }

    //cerramos los archivos
    for (int i = 0; i < cantidad; i++) {
        fclose(mids[i].archivo);
    }

    fclose(f_out);
}

/**
 * Elimina los archivos temporales generados durante el proceso
 * @param cantidad_mids Cantidad de archivos intermedios
 * @param cantidad_sup_mids Cantidad de archivos superiores
 * @param cuantosM Tamaño del archivo en M (M = 50MB)
 */
void borrar_middle_files(size_t cantidad_mids, int cantidad_sup_mids, int cuantosM) {
    for (int i = 0; i < cantidad_mids; i++) {
        char nombre_mid[64];
        snprintf(nombre_mid, sizeof(nombre_mid), "mid_%d.bin", i);
        if (remove(nombre_mid) != 0) {
            perror("borrar mid file");
            exit(1);
        }
    }
    for (int i = 0; i < cantidad_sup_mids; i++) {
        char nombre_sup_mid[64];
        snprintf(nombre_sup_mid, sizeof(nombre_sup_mid), "superior_mid_%dM_%d.bin", cuantosM, i);        if (remove(nombre_sup_mid) != 0) {
            perror("borrar superior mid file");
            exit(1);
        }
    }
}

/**
 * Reinicia el contador de operaciones I/O
 */
void reiniciar_contador_IOs() {
    lecturas_escrituras = 0;
}

