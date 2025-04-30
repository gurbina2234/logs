#include "quicksort_e.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
#include <set>
#include <vector>

// arreglo de numeros para trabajar en memoria principal -> buffer
void readBlock(const std::string &filename, size_t posicion, size_t B, std::vector<int64_t> &buffer) {
     std::ifstream file(filename, std::ios::binary);
     if (!file) {
          std::cerr << "Error opening file: " << filename << std::endl;
          return;
     }
     // mueve puntero lectura a posicion * B
     file.seekg(posicion * B, std::ios::beg);
     size_t maxSize = B / sizeof(int64_t);
     buffer.resize(maxSize); // resize el buffer para que almacene
     file.read(reinterpret_cast<char *>(buffer.data()), B);
     size_t bytesRead = file.gcount();
     size_t elementsRead = bytesRead / sizeof(int64_t);
     buffer.resize(elementsRead); 
     file.close();
}

void writeBlock(const std::string &filename, size_t posicion, size_t B, const std::vector<int64_t> &buffer) {
     std::ofstream file(filename, std::ios::binary | std::ios::in |std::ios::out); // poner ios in y ios out sino se puede
     if (!file) {
          std::cerr << "Error opening file: " << filename << std::endl;
          return;
     }
  // puntero escritura
     file.seekp(posicion * B, std::ios::beg);
     file.write(reinterpret_cast<const char *>(buffer.data()), B);
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
     std::set<int64_t> pivotsSet;
     for (size_t i = 0; i < numPivots; ++i) {
          size_t randomIndex = rand() % finalSize;
          pivotsSet.insert(buffer[randomIndex]);
     }
     // transformar el set en un vector
     std::vector<int64_t> pivots(pivotsSet.begin(), pivotsSet.end());
     // ordenar los pivotes
     std::sort(pivots.begin(), pivots.end());
     return pivots;
}

int main() {
     std::string filename = "data.bin"; // nombre del archivo
     srand(static_cast<unsigned>(time(0)));
     {
     std::ofstream out(filename, std::ios::binary);
     for (int64_t i = 0; i < 20; ++i) {
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
     size_t N = 20; // numero de elementos -> 20 elementos
     size_t B = 48; // tamaño del bloque -> 8*6 elementos
     size_t a = 3;  // numero de pivotes -> 3 pivotes
     std::vector<int64_t> pivotes = randomInterval(filename, N, B, a);
     std::cout << "Pivotes random:\n";
     for (int64_t pivot : pivotes) {
          std::cout << pivot << " ";
     }
     std::cout << std::endl;
     return 0;
}