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
#define ARIDAD 256 //aridad (numero de archivos a mergear)

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
    
    return (x > y) - (x < y); //1 si a>b, -1 si a<b
}

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

    *cantidad_mid_files = mid_num;
    free(buffer);
    fclose(f_in);
}

//llena y/o actualiza el buffer del middle file
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

void merge_tanda(int inicio, int cantidad, const char* nombre_salida) {
    Mid mids[cantidad];
    int64_t buffer_a_disco[NUMS_POR_BLOQUE];
    size_t elementos_escritos = 0;

    //abrir archivos mid
    for (int i = 0; i < cantidad; i++) {
        int idx_archivo = inicio + i;
        char nombre_archivo[64];
        sprintf(nombre_archivo, "mid_%d.bin", idx_archivo);

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
        int idx = seleccionar_minimo(mids, cantidad);
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


void merge_cant_sobre_a(size_t cantidad, int cuociente, int residuo, int cuantosM, int *cantidadSupMid) {
    for (int index = 0; index < cuociente; index++) {
        *cantidadSupMid = cuociente + (residuo > 0);
        int offset = index * ARIDAD;
        char nombre_salida[64];
        sprintf(nombre_salida, "superior_mid_%dM_%d.bin", cuantosM, index);

        merge_tanda(offset, ARIDAD, nombre_salida);
    }

    if (residuo > 0) {
        int offset = cuociente * ARIDAD;
        char nombre_salida[64];
        sprintf(nombre_salida, "superior_mid_%dM_%d.bin", cuantosM, cuociente);

        merge_tanda(offset, residuo, nombre_salida);
    }

    Mid sup_mids[cuociente + (residuo > 0)];
    int64_t buffer_a_disco[NUMS_POR_BLOQUE];
    size_t elementos_escritos = 0;

    for (int i = 0; i < (cuociente + (residuo > 0)); i++) {
        char nombre_superior_mid[64];
        sprintf(nombre_superior_mid, "superior_mid_%dM_%d.bin", cuantosM,i);
        sup_mids[i].archivo = fopen(nombre_superior_mid, "rb");
        if (!sup_mids[i].archivo) {
            perror("abrir superior mid");
            exit(1);
        }
        update_mf_buffer(&sup_mids[i]);
    }

    char nombre_archivo_ordenado[64];
    sprintf(nombre_archivo_ordenado, "orden_%dM.bin", cuantosM);
    FILE *f_out = fopen(nombre_archivo_ordenado, "wb");
    if (!f_out) {
        perror("crear orden final");
        exit(1);
    }

    while (1) {
        int idx = seleccionar_minimo(sup_mids, cuociente + (residuo > 0));
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

void merge_middle_files(size_t cantidad, int cuantosM, int *cantidadSupMid, int iteracion) {
    if (cantidad > ARIDAD) {
        printf("cantidad: %zu | aridad: %d", cantidad, ARIDAD);
        int cuociente = cantidad / ARIDAD;
        int residuo = cantidad % ARIDAD;
        merge_cant_sobre_a(cantidad, cuociente, residuo, cuantosM, cantidadSupMid);
        return;
    }

    Mid mids[ARIDAD];
    int64_t buffer_a_disco[NUMS_POR_BLOQUE]; //se escribe 1 bloque a la vez
    size_t elementos_escritos = 0;

    for (int i = 0; i < cantidad; i++) {
        char nombre_mid[64];
        sprintf(nombre_mid, "mid_%d.bin", i);
        mids[i].archivo = fopen(nombre_mid, "rb");
        if (!mids[i].archivo) {
            perror("abrir mid file");
            exit(1);
        }
        update_mf_buffer(&mids[i]);
    }

    char nombre_archivo_ordenado[64];
    sprintf(nombre_archivo_ordenado, "orden_%dM_%d.bin", cuantosM, iteracion);
    FILE *f_out = fopen(nombre_archivo_ordenado, "wb");
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

void borrar_middle_files(size_t cantidad_mids, int cantidad_sup_mids, int cuantosM) {
    for (int i = 0; i < cantidad_mids; i++) {
        char nombre_mid[64];
        sprintf(nombre_mid, "mid_%d.bin", i);
        if (remove(nombre_mid) != 0) {
            perror("borrar mid file");
            exit(1);
        }
    }
    for (int i = 0; i < cantidad_sup_mids; i++) {
        char nombre_sup_mid[64];
        sprintf(nombre_sup_mid, "superior_mid_%dM_%d.bin", cuantosM, i);
        if (remove(nombre_sup_mid) != 0) {
            perror("borrar superior mid file");
            exit(1);
        }
    }
}

void reiniciar_contador_IOs() {
    lecturas_escrituras = 0;
}

int main() {
    int cuantosM = 60;

    for (int i = 0; i < 5; i++) {
        reiniciar_contador_IOs();

        size_t cantidad_mids = 0;
        int cantidadSupMids = 0;
        char nombre_archivo[64];
        sprintf(nombre_archivo, "datos_%dM_%d.bin", cuantosM, i);
        
        clock_t inicio = clock();

        generar_middle_files(nombre_archivo, &cantidad_mids);
        merge_middle_files(cantidad_mids, 60, &cantidadSupMids, i);

        clock_t fin = clock();
        double segundos = (double)(fin - inicio) / CLOCKS_PER_SEC;
        
        printf("\nArchivo ordenado: orden_%dM_%d.bin\n", 60, i);
        printf("Tiempo total: %.2f segundos\n", segundos);
        printf("Total I/Os (lecturas + escrituras de bloques): %zu\n", lecturas_escrituras);
        
        printf("Borrando middle files antes de comenzar denuevo...");
        borrar_middle_files(cantidad_mids, cantidadSupMids, 60);
        printf("Comenzando siguiente iteracion.");
    }

    return 0;
}