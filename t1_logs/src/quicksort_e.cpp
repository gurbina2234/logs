#include "quicksort_e.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>


size_t readnwrite = 0;

void readBlock(const std::string &filename, size_t posicion, size_t B, std::vector<int64_t> &buffer) {
     std::ifstream file(filename, std::ios::binary);
     if (!file) {
          std::cerr << "Error opening file: " << filename << std::endl;
          return;
     }

     file.seekg(posicion * B, std::ios::beg);
     size_t blockSize = B / sizeof(int64_t);
     buffer.resize(blockSize); 
     file.read(reinterpret_cast<char *>(buffer.data()), B);
     size_t bytesRead = file.gcount();
     if (bytesRead < B) {
          buffer.resize(bytesRead / sizeof(int64_t));
     }
     readnwrite ++;
     file.close();
}
 
 
void writeBlock(const std::string &filename, size_t posicion, size_t B, std::vector<int64_t> &buffer) {
     std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out); // poner ios in y ios out sino se puede reescribir todo el archivo
     if (!file.is_open()) {
          //crear achivo vacio
          std::ofstream createFile(filename, std::ios::binary);
          createFile.close();
          file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
          if (!file.is_open()) {
              std::cerr << "Error: no se pudo crear el archivo " << filename << std::endl;
              return;
          }
     }
     file.seekp(posicion * B, std::ios::beg);
     size_t bytestoWrite = buffer.size() * sizeof(int64_t);
     file.write(reinterpret_cast<const char *>(buffer.data()), bytestoWrite);
     readnwrite ++;
     file.close();
}
 
std::vector<int64_t> randomInterval(const std::string &filename, size_t N, size_t B, size_t a) {
     std::vector<int64_t> buffer;
     size_t blockSize = B / sizeof(int64_t);
     size_t numBlocks = (N + blockSize - 1) / blockSize;
     size_t numPivots = a - 1;
     
     // crear intentos para evitar que se tenga un set de pivotes incorrecto
     std::set<int64_t> candidates;
     size_t maxTries = 10;  
 
     for (size_t tries = 0; tries < maxTries && candidates.size() < numPivots; ++tries) {
         size_t randomBlock = rand() % numBlocks;
         std::vector<int64_t> tempBuffer;
         readBlock(filename, randomBlock, B, tempBuffer);
         for (auto val : tempBuffer) {
             candidates.insert(val);
             if (candidates.size() >= numPivots) break;
         }
     }
 
     if (candidates.size() < numPivots) {
         std::cerr << "No se pudieron obtener suficientes pivotes unicos!!.\n";
     }
 
     std::vector<int64_t> pivots(candidates.begin(), candidates.end());
     std::sort(pivots.begin(), pivots.end());
 
     std::cout << "Pivotes elegidos:\n";
     for (int64_t num : pivots) {
         std::cout << num << " ";
     }
     std::cout << std::endl;
 
     return pivots;
}

//funcion que lee un archivo binario de tamaño A en B bloques usando readBlock
void readAllMemory(const std::string &filename,size_t startBlock, size_t B, size_t numBlocks, std::vector<int64_t> &buffer) {
     size_t blockSize = B / sizeof(int64_t);
     for (size_t i = 0; i < numBlocks; ++i) {
          std::vector<int64_t> tempBuffer;
          readBlock(filename, startBlock + i, B, tempBuffer);
          buffer.insert(buffer.end(), tempBuffer.begin(), tempBuffer.end());
     }
}

void quicksortExternal(const std::string &filename, size_t N, size_t B, size_t M, size_t a, size_t depth = 0) {
     size_t blockSize = B / sizeof(int64_t); 
     size_t numBlocks = (N + blockSize - 1) / blockSize;
     size_t blocksMemory = (M + blockSize - 1) / blockSize; // cantidad de bloques que caben en memoria principal
     if (N <= M) {
          std::cout << "CASO BASE!! Ordenando " << N << " elementos en memoria principal.\n";
          std::vector<int64_t> uploadMemory;
          //leer de a bloques el archivo binario 
          for (size_t i = 0; i < numBlocks; ++i) {
               std::vector<int64_t> valuesBlock;
               readBlock(filename, i, B, valuesBlock);
               uploadMemory.insert(uploadMemory.end(), valuesBlock.begin(), valuesBlock.end());
          }
          //ordenar el arreglo completo en memoria principal
          std::sort(uploadMemory.begin(), uploadMemory.end());
          //escribir de a bloques de tamaño B
          for(size_t i = 0; i < numBlocks; ++i) {
               size_t start = i * blockSize;
               size_t end = std::min(start + blockSize, N);
               std::vector<int64_t> valuesBlock(uploadMemory.begin() + start, uploadMemory.begin() + end);
               writeBlock(filename, i, B, valuesBlock);
          }
          return;
     }
     std::cout << "CASO RECURSIVO!! Ordenando " << N << " elementos en disco.\n";
     std::vector<int64_t> pivots = randomInterval(filename, N, B, a);
     std::vector<std::vector<int64_t>> subArrays(a);
     std::vector<size_t> blockCounters(a, 0);
     for (size_t i = 0; i < numBlocks; i += blocksMemory) {
          std::vector<int64_t> memoryBuffer;
          size_t blockstoRead = std::min(blocksMemory, numBlocks - i);
          //leer bloques en memoria principal
          readAllMemory(filename, i, B, blockstoRead, memoryBuffer);
          //clasificar los elementos leidos en los subarreglos
          for (size_t k = 0; k < memoryBuffer.size(); ++k) {
               int64_t num = memoryBuffer[k];
               size_t j = 0;
               while (j < pivots.size() && num > pivots[j]) {
                    ++j;
               }
               if (j >= a) j = a - 1;
               subArrays[j].push_back(num);
               if (subArrays[j].size() == blockSize) {
                    std::string subFileName = "temp_" + std::to_string(depth) + "_p" + std::to_string(j) + ".bin";
                    writeBlock(subFileName, blockCounters[j], B, subArrays[j]);
                    subArrays[j].clear();
                    blockCounters[j]++;
               }
          }

     }
     //escribir los bloques restantes incompletos
     for (size_t j = 0; j < a; ++j) {
          if (!subArrays[j].empty()) {
              std::string subFile = "temp_" + std::to_string(depth) + "_p" + std::to_string(j) + ".bin";
              writeBlock(subFile, blockCounters[j], B, subArrays[j]);
              blockCounters[j]++;
          }
     }
     //llamar recursivamente por cada subarreglo (subarchivo)
     //guarda el tamaño de los subarreglos
     std::vector<size_t> numElements(a);
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
          numElements[i] = sizebytes / sizeof(int64_t);
          in.close();
          quicksortExternal("temp_" + std::to_string(depth) + "_p" + std::to_string(i) + ".bin", numElements[i], B, M, a, depth + 1);
     }
     //fusionar los subarreglos
     std::ofstream out(filename, std::ios::binary);
     std::vector<int64_t> accumulatedBuffer; // buffer acumulado
     accumulatedBuffer.reserve(blockSize);  // reserva espacio para evitar realocaciones
     size_t outCurrentBlock = 0; // inicializar conteo de bloques

     for (size_t i = 0; i < a; ++i) {
          std::string tempFile = "temp_" + std::to_string(depth) + "_p" + std::to_string(i) + ".bin";
          std::ifstream in(tempFile, std::ios::binary | std::ios::ate);
          if (!in) {
               std::cerr << "Error opening temp file: " << tempFile << std::endl;
               continue;
          }
          size_t fileSize = in.tellg() / sizeof(int64_t);
          size_t totalBlocks = (fileSize + blockSize - 1) / blockSize;
          in.close();
          
          for (size_t j = 0; j < totalBlocks; ++j) {
               std::vector<int64_t> buffer;
               readBlock(tempFile, j, B, buffer);
          
               // acumular los valores en el buffer global
               accumulatedBuffer.insert(accumulatedBuffer.end(), buffer.begin(), buffer.end());
          
               // si se alcanza el tamaño de un bloque escribirlo al archivo
               while (accumulatedBuffer.size() >= blockSize) {
                    std::vector<int64_t> blockToWrite(accumulatedBuffer.begin(), accumulatedBuffer.begin() + blockSize);
                    writeBlock(filename, outCurrentBlock++, B, blockToWrite);
                    accumulatedBuffer.erase(accumulatedBuffer.begin(), accumulatedBuffer.begin() + blockSize);
               }
          }
          std::remove(tempFile.c_str());
     }
      
      //escribir elementos faltantes en buffer acumulado 
     if (!accumulatedBuffer.empty()) {
          writeBlock(filename, outCurrentBlock++, B, accumulatedBuffer);
     }
     out.close();
     return;
} 

int main() {
     std::string filename = "datos_60M.bin"; 
     srand(static_cast<unsigned>(time(0)));
     size_t memoryLimitBytes = 50000000; 

     size_t N = 3145728000 / sizeof(int64_t); 
     size_t M = memoryLimitBytes / sizeof(int64_t);
     size_t B = 4096; 
     size_t a = 32;  
     
     quicksortExternal(filename, N, B, M, a);
     std::cout << "Cantidad de Read y Writes: " << readnwrite << std::endl;

     return 0;

}