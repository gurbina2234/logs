#include "quicksort_e.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
#include <set>

// arreglo de numeros para trabajar en memoria principal -> buffer
void readBlock(const std::string &filename, size_t posicion, size_t B, std::vector<int64_t> &buffer) {
     std::ifstream file(filename, std::ios::binary);
     if (!file) {
          std::cerr << "Error opening file: " << filename << std::endl;
          return;
     }

     file.seekg(posicion * B, std::ios::beg);
     size_t maxSize = B / sizeof(int64_t);
     buffer.resize(maxSize); // reserva espacio para el bloque
     file.read(reinterpret_cast<char *>(buffer.data()), B);
     size_t bytesRead = file.gcount();
     if (bytesRead < B) {
          buffer.resize(bytesRead / sizeof(int64_t));
     }
     file.close();
}

void writeBlock(const std::string &filename, size_t posicion, size_t B, std::vector<int64_t> &buffer) {
     std::ofstream file(filename, std::ios::binary | std::ios::in |std::ios::out); // poner ios in y ios out sino se puede reescribir todo el archivo
     if (!file) {
          std::cerr << "Error opening file: " << filename << std::endl;
          return;
     }
  // puntero escritura
     file.seekp(posicion * B, std::ios::beg);
     size_t bytestoWrite = buffer.size() * sizeof(int64_t);
     file.write(reinterpret_cast<const char *>(buffer.data()), bytestoWrite);
     file.close();
}

std::vector<int64_t> randomInterval(const std::string &filename, size_t N, size_t B, size_t a) {
     std::vector<int64_t> buffer; // leer en memoria principal
     size_t blockSize = B / sizeof(int64_t);
     // agrega un bloque en caso que no sea divisible N por blockSize
     size_t numBlocks = (N + blockSize - 1) / blockSize;
     size_t numPivots = a - 1;
     // leer un bloque de filename aleatorio
     size_t randomBlock = rand() % numBlocks;
     // imprimir el bloque aleatorio elegido
     std::cout << "Bloque aleatorio: " << randomBlock << std::endl;
     // imprimir el contenido del bloque aleatorio elegido
     readBlock(filename, randomBlock, B, buffer);
     std::cout << "Numeros en el bloque:\n";
     for (int64_t num : buffer) {
          std::cout << num << " ";
     }
     std::cout << std::endl;
     // elegir a-1 elementos random del bloque para que sean pivotes
     size_t finalSize = buffer.size();
     if (finalSize < numPivots) {
          std::cerr << "Error: No hay suficientes elementos en el bloque para elegir " << numPivots << " pivotes.\n";
          return {};
     }
     std::set<int64_t> pivotsSet;
     while (pivotsSet.size() < numPivots && finalSize > 0) {
          size_t randomIndex = rand() % finalSize;
          pivotsSet.insert(buffer[randomIndex]);
     }
     // transformar el set en un vector
     std::vector<int64_t> pivots(pivotsSet.begin(), pivotsSet.end());
     // ordenar los pivotes
     std::sort(pivots.begin(), pivots.end());
     //imprimir pivotes
     std::cout << "Pivotes elegidos:\n";
     for (int64_t num : pivots) {
          std::cout << num << " ";
     }
     std::cout << std::endl;
     return pivots;
}

void quicksortExternal(const std::string &filename, size_t N, size_t B, size_t M, size_t a, size_t depth = 0) {
     size_t blockSize = B / sizeof(int64_t); //cantidad de numeros en un bloque
     size_t numBlocks = (N + blockSize - 1) / blockSize; //cantidad de bloques B 
     //caso base primero
     std::ifstream file(filename, std::ios::binary | std::ios::in |std::ios::out);
     if (!file) {
          std::cerr << "Error opening file: " << filename << std::endl;
          return;
     }
     if (N <= M) {
          std::cout << "CASO BASE!! Ordenando " << N << " elementos en memoria principal.\n";
          std::vector<int64_t> uploadMemory;
          // Leer todos los bloques (completos) y concatenarlos
          for (size_t i = 0; i < numBlocks; ++i) {
               std::vector<int64_t> block;
               readBlock(filename, i, B, block);
               uploadMemory.insert(uploadMemory.end(), block.begin(), block.end());
          }     
          // Ordenar el arreglo completo en memoria principal
          std::sort(uploadMemory.begin(), uploadMemory.end());
     
          // Escribir de vuelta en bloques de tamaño B
          for (size_t i = 0; i < numBlocks; ++i) {
               size_t start = i * blockSize;
               size_t end = std::min(start + blockSize, N);
          
               std::vector<int64_t> block(uploadMemory.begin() + start, uploadMemory.begin() + end);
               writeBlock(filename, i, B, block);
          }
          return;
     }
     //caso recursivo 
     //leer un bloque de A aleatorio y elegir a- 1 elementos al azar y ordenarlos, esto lo hace la funcion randomInterval
     std::vector<int64_t> pivots = randomInterval(filename, N, B, a);
     //crear archivos binarios (buffers en disco) que representan los subarreglos
     std::vector<std::ofstream> subFiles(a);
     for (size_t i = 0; i < a; ++i) {
          std::string subFileName = "temp_" + std::to_string(depth) + "_p" + std::to_string(i) + ".bin";
          subFiles[i].open(subFileName, std::ios::binary);
     }
     //leer el archivo y clasificar en los subarreglos
     // leer cada bloque del archivo original
     for (size_t i = 0; i < numBlocks; ++i) {
          std::vector<int64_t> buffer;
          readBlock(filename, i, B, buffer);
          // leer elementos del bloque y clasificarlos
          for (size_t i = 0; i < buffer.size(); ++i) {
               int64_t num = buffer[i];
               size_t j = 0;
               while (j < pivots.size() && num > pivots[j]) {
                    ++j;
               }
               if (j >= a) {
                    j = a - 1;
               }
               std::cout << "Escribiendo " << num << " en temp_" << j << ".bin\n";
               subFiles[j].write(reinterpret_cast<const char *>(&num), sizeof(int64_t));
          }
     }
     // cerrar los archivos de subarreglos
     for (size_t i = 0; i < a; ++i) {
          subFiles[i].close();
     }
     //guarda el tamaño de los subarreglos 
     std::vector<size_t> numElements(a);
     //llamar recursivamente por cada subarreglo (subarchivo)
     for (size_t i = 0; i < a; ++i){
          //abre el archivo del subarreglo 
          std::string subFilename = "temp_" + std::to_string(depth) + "_p" + std::to_string(i) + ".bin";
          std::ifstream in(subFilename, std::ios::binary | std::ios::ate);
          if (!in) {
               std::cerr << "No se pudo abrir " << subFilename << " (posiblemente vacío).\n";
               continue; // salta a la siguiente iteración
          }
          size_t sizebytes = in.tellg();
          if (sizebytes == 0) {
               std::cerr << subFilename << " está vacío, se omite.\n";
               in.close();
               continue;
          }
          //calcular cantidad de numeros 
          numElements[i] = sizebytes / sizeof(int64_t);
          in.close();
          //llamar a la funcion recursiva
          quicksortExternal("temp_" + std::to_string(depth) + "_p" + std::to_string(i) + ".bin", numElements[i], B, M, a, depth + 1);
     }
     // fusionar los subarreglos ordenados
     std::ofstream out(filename, std::ios::binary);
     for (size_t i = 0; i < a; ++i) {
          //lee subarreglo ordenado 
          std::ifstream in("temp_" + std::to_string(depth) + "_p" + std::to_string(i) + ".bin", std::ios::binary);
          //devuelve un buffer con todo el contenido del archivo binario temporal y lo escribe en out 
          out << in.rdbuf();
          in.close();
          // borra archivo del disco
          std::remove(("temp_" + std::to_string(depth) + "_p" + std::to_string(i) + ".bin").c_str());
     }
     out.close();
     return;
}


int main() {
     std::string filename = "data.bin"; // nombre del archivo
     srand(static_cast<unsigned>(time(0)));
     {
     std::ofstream out(filename, std::ios::binary);
     for (int64_t i = 0; i < 50; ++i) {
          int64_t randomNum = rand() % 100; // numeros aleatorio entre 0 y 99
          out.write(reinterpret_cast<const char *>(&randomNum), sizeof(int64_t));
     }
     out.close();
     std::cout << "Archivo de prueba creado.\n";
     }
     // leer el archivo de prueba
     std::ifstream in(filename, std::ios::binary);
     std::cout << "Valores del archivo:\n";
     int64_t number;
     while (in.read(reinterpret_cast<char *>(&number), sizeof(int64_t))) {
          std::cout << number << " ";
     }
     std::cout << std::endl;
     in.close();
     size_t N = 50; // numero de elementos -> 20 elementos
     size_t B = 48; // tamaño del bloque -> 8*6 elementos
     size_t M = 6;  // tamaño de la memoria principal -> 10 elementos
     size_t a = 3;  // numero de pivotes -> 3 pivotes
     
     quicksortExternal(filename, N, B, M, a);
     // leer el archivo ordenado
     // 5. Mostrar archivo ordenado
     std::ifstream sortedIn(filename, std::ios::binary);
     std::cout << "Archivo ordenado:\n";
     while (sortedIn.read(reinterpret_cast<char*>(&number), sizeof(int64_t))) {
          std::cout << number << " ";
     }
     std::cout << std::endl;
     sortedIn.close();

     return 0;

}