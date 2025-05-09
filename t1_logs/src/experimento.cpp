#include "generateSequences.hpp"
#include "mergesort_e.hpp"
#include "quicksort_e.hpp"
#include <IOs.hpp>
#include <chrono>

size_t lecturas_escrituras = 0;
int main()
{
    
    
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
            auto inicio_m = std::chrono::high_resolution_clock::now();

            generar_middle_files(nombre, &cantidad_mids);
            merge_middle_files(cantidad_mids, m, &cantidadSupMids, i);

            auto fin_m = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duracion_m = fin_m - inicio_m;
            char nombreOrdenado[64];
            snprintf(nombreOrdenado, sizeof(nombreOrdenado),"orden_%dM_%d.bin", m, i);
            printf("\nArchivo ordenado: %s\n", nombreOrdenado);
            printf("Tiempo total: %.2f segundos\n", duracion_m.count());
            printf("Total I/Os (lecturas + escrituras de bloques): %zu\n", lecturas_escrituras);
            
            borrar_middle_files(cantidad_mids, cantidadSupMids, m);
            std::remove(nombreOrdenado);
            std::cout << "----------------------------------------\n" << std::endl;
            std::cout << "QUICKSORT" << std::endl;
          
            lecturas_escrituras = 0;
            std::string filename = nombre;
            std::cout << "Comienzo con el archivo: " << filename << std::endl;
            std::string filenameSorted = "SORTED_"+std::to_string(m)+"M_" + std::to_string(i) + "_" + std::to_string(192) + ".bin";
            size_t MB = 50 * 1024 * 1024;
            size_t Nbytes = m * MB;
            size_t N = Nbytes / sizeof(int64_t);  
            auto inicio_q = std::chrono::high_resolution_clock::now();

            quicksortExternal(filename, filenameSorted, N);

            auto fin_q = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duracion_q = fin_q - inicio_q;
            std::cout << "Cantidad de Read y Writes: " << lecturas_escrituras << std::endl;
            printf("Tiempo total: %.2f segundos\n", duracion_q.count());
            std::cout << "Proxima iteracion: " << i+1 << std::endl;
            std::cout << "----------------------------------------\n" << std::endl;
            
            std::cout << "Borrando archivos" << std::endl;
            std::remove(nombre);
            std::remove(filenameSorted.c_str());
            i--;
            
            }
           
    }
    return 0;
}
