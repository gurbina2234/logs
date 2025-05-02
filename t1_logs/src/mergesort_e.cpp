#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <cstring>

#define B_BYTES 4096 //tamaño bloque
#define M_BYTES (50 * 1024 * 1024) //tamaño RAM
#define RESERVA_RAM (48 * 1024 * 1024) //tamaño de RAM a reservar
#define NUMS_POR_BLOQUE (B_BYTES / sizeof(int64_t)) // = 512 | cant de numeros por bloque
#define BLOQUES_EN_RAM (RESERVA_RAM / B_BYTES) //numero de bloques posibles en RAM (con reserva)
#define NUMS_EN_RAM (BLOQUES_EN_RAM * NUMS_POR_BLOQUE) // = 6.291.456 numeros
#define ARIDAD 128 //aridad (numero de archivos a mergear)

size_t lecturas_escrituras = 0;

typedef struct {
    FILE* archivo; //mid_#.bin
    int64_t buffer[NUMS_POR_BLOQUE]; //buffer de 1 bloque
    size_t pos = 0; //posicion actual dentro del buffer
    size_t usados = 0;
    bool terminado = false;
} Mid;

int comparar_64(const void *a, const void *b) {
    int64_t x = *(int64_t*)a;
    int64_t y = *(int64_t*)b;
    
    return (a > b) - (a < b); //1 si a>b, -1 si a<b
}

void generar_middle_files(char *datos_desordenados) {
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
        sprintf(nombre_mid, "mid_%zu.bin", mid_num++);
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

    free(buffer);
    fclose(f_in);
}