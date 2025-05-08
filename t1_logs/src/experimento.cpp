#include "generateSequences.hpp"
#include "mergesort_e.hpp"
#include "quicksort_e.hpp"


int main()
{
    size_t lecturas_escrituras = 0;
    
    const size_t MB = 50 * 1024 * 1024;

    for (int m = 4; m <= 60; m += 4) {
        for (int i = 0; i < 5; i++) {
            i++;
            size_t total_bytes = m * MB;
            char nombre[64];
            snprintf(nombre, sizeof(nombre),"datos_%dM_%d.bin", m, i);
            generateSequences(total_bytes, nombre);
            lecturas_escrituras = 0;

            size_t cantidad_mids = 0;
            int cantidadSupMids = 0;
            printf("Comienzo con el archivo: %s\n", nombre);
            std::cout << "MERGESORT" << std::endl;
            clock_t inicio = clock();

            generar_middle_files(nombre, &cantidad_mids);
            merge_middle_files(cantidad_mids, 60, &cantidadSupMids, i);

            clock_t fin = clock();
            double segundos = (double)(fin - inicio) / CLOCKS_PER_SEC;
            
            printf("\nArchivo ordenado: orden_%dM_%d.bin\n", 60, i);
            printf("Tiempo total: %.2f segundos\n", segundos);
            printf("Total I/Os (lecturas + escrituras de bloques): %zu\n", lecturas_escrituras);
            
            printf("Borrando middle files antes de comenzar denuevo...");
            borrar_middle_files(cantidad_mids, cantidadSupMids, 60);
            printf("Comenzando siguiente iteracion.");

            std::cout << "----------------------------------------\n" << std::endl;
            std::cout << "QUICKSORT" << std::endl;
          
            lecturas_escrituras = 0;
            std::string filename = nombre;
            std::cout << "Comienzo con el archivo: " << filename << std::endl;
            std::string filenameSorted = "SORTED_60M_" + std::to_string(i) + "_" + std::to_string(192) + ".bin";
            size_t MB = 50 * 1024 * 1024;
            size_t Nbytes = 60 * MB;
            size_t N = Nbytes / sizeof(int64_t);  
            clock_t inicio_q = clock();
            quicksortExternal(filename, filenameSorted, N);
            clock_t fin_q = clock();
            double segundosTotal = (double)(fin - inicio) / CLOCKS_PER_SEC;
            std::cout << "Cantidad de Read y Writes: " << lecturas_escrituras << std::endl;
            printf("Tiempo total: %.2f segundos\n", segundosTotal);
            std::cout << "Proxima iteracion: " << i+1 << std::endl;
            std::cout << "----------------------------------------\n" << std::endl;
            i--; 
            }
           
    }
    return 0;
}
