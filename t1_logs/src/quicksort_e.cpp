#include "quicksort_e.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
#include <set>

size_t readnwrite = 0;
size_t B = 4096; 
size_t blockSize = B / sizeof(int64_t);
size_t M = (50 * 1024 * 1024) / sizeof(int64_t);
size_t blocksMemory = (M + blockSize - 1) / blockSize;
size_t a = 256;

void readBlock(const std::string &filename, size_t posicion, std::vector<int64_t> &buffer) {
     std::ifstream file(filename, std::ios::binary);
     if (!file) {
          std::cerr << "Error opening file: " << filename << std::endl;
          return;
     }
     file.seekg(posicion * B, std::ios::beg);
     buffer.resize(blockSize); 
     file.read(reinterpret_cast<char *>(buffer.data()), B);
     size_t bytesRead = file.gcount();
     if (bytesRead < B) { //caso bloque incompleto
          buffer.resize(bytesRead / sizeof(int64_t));
     }
     readnwrite ++;
     file.close();
}
 
 
void writeBlock(const std::string &filename, size_t posicion, std::vector<int64_t> &buffer) {
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
     if (buffer.size() < blockSize) {
          size_t bytesIncomplete = buffer.size() * sizeof(int64_t);
          file.write(reinterpret_cast<const char *>(buffer.data()), bytesIncomplete);
     }
     else{
          size_t bytesTotal = blockSize * sizeof(int64_t);
          file.write(reinterpret_cast<const char *>(buffer.data()), bytesTotal);
     }
     readnwrite ++;
     file.close();
     return;
}
 
std::vector<int64_t> randomInterval(const std::string &filename, size_t N) {
     std::vector<int64_t> buffer;
     size_t numBlocks = (N + blockSize - 1) / blockSize;
     size_t numPivots = a - 1;
     // crear intentos para evitar que se tenga un set de pivotes incorrecto
     std::set<int64_t> candidates;
     size_t randomBlock = rand() % numBlocks;
     std::vector<int64_t> tempBuffer;
     readBlock(filename, randomBlock, tempBuffer);
     std::shuffle(tempBuffer.begin(), tempBuffer.end(), std::mt19937{std::random_device{}()});
     for (auto val : tempBuffer) {
          candidates.insert(val);
          if (candidates.size() >= numPivots) break;
     }
     if (candidates.size() < numPivots) {
         std::cerr << "No se pudieron obtener suficientes pivotes unicos!!.\n";
     }
     std::vector<int64_t> pivots(candidates.begin(), candidates.end());
     std::sort(pivots.begin(), pivots.end());
     return pivots;
}

//funcion que lee un archivo binario de tamaño A en B bloques usando readBlock
void readAllMemory(const std::string &filename,size_t startBlock, size_t numBlocks, std::vector<int64_t> &buffer) {
     for (size_t i = 0; i < numBlocks; ++i) {
          std::vector<int64_t> tempBuffer;
          readBlock(filename, startBlock + i, tempBuffer);
          buffer.insert(buffer.end(), tempBuffer.begin(), tempBuffer.end());
     }
}

void mergeFiles(const std::string &filenameSorted, size_t N, size_t depth) {
     std::fstream file(filenameSorted, std::ios::binary | std::ios::in | std::ios::out);
     std::vector<int64_t> accumulatedBuffer;
     accumulatedBuffer.reserve(blockSize); 
     size_t outCurrentBlock = 0; // inicializar conteo de bloques en nuevo archivo binario
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
               readBlock(tempFile, j, buffer);
               accumulatedBuffer.insert(accumulatedBuffer.end(), buffer.begin(), buffer.end());
               while (accumulatedBuffer.size() >= blockSize) {
                    std::vector<int64_t> blockToWrite(accumulatedBuffer.begin(), accumulatedBuffer.begin() + blockSize);
                    writeBlock(filenameSorted, outCurrentBlock++, blockToWrite);
                    accumulatedBuffer.erase(accumulatedBuffer.begin(), accumulatedBuffer.begin() + blockSize);
               }
          }
          std::remove(tempFile.c_str());
     }
     //escribir elementos que faltan en el buffer
     if (!accumulatedBuffer.empty()) {
          writeBlock(filenameSorted, outCurrentBlock++, accumulatedBuffer);
     }
     file.close();
     return;
}

void quicksortExternal(const std::string &filename, const std::string &filenameSorted, size_t N, size_t depth) {
     size_t numBlocks = (N + blockSize - 1) / blockSize;
     if (N <= M) {
          std::vector<int64_t> uploadMemory;
          readAllMemory(filename, 0, numBlocks, uploadMemory);
          std::sort(uploadMemory.begin(), uploadMemory.end());
          for(size_t i = 0; i < numBlocks; ++i) {
               size_t start = i * blockSize;
               size_t end = std::min(start + blockSize, N);
               std::vector<int64_t> valuesBlock(uploadMemory.begin() + start, uploadMemory.begin() + end);
               writeBlock(filenameSorted, i, valuesBlock);
          }
          return;
     }
     std::vector<int64_t> pivots = randomInterval(filename, N);
     std::vector<std::vector<int64_t>> subArrays(a);
     std::vector<size_t> blockCounters(a, 0);
     for (size_t i = 0; i < numBlocks; i += blocksMemory) {
          std::vector<int64_t> memoryBuffer;
          size_t blockstoRead = std::min(blocksMemory, numBlocks - i);
          //leer bloques en memoria principal
          readAllMemory(filename, i, blockstoRead, memoryBuffer);
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
                    writeBlock(subFileName, blockCounters[j], subArrays[j]);
                    subArrays[j].clear();
                    blockCounters[j]++;
               }
          }

     }
     //escribir los bloques restantes incompletos
     for (size_t j = 0; j < a; ++j) {
          if (!subArrays[j].empty()) {
              std::string subFile = "temp_" + std::to_string(depth) + "_p" + std::to_string(j) + ".bin";
              writeBlock(subFile, blockCounters[j], subArrays[j]);
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
               continue; 
          }
          size_t sizeBytes = in.tellg();
          if (sizeBytes == 0) {
               std::cerr << subFilename << " está vacío, se omite.\n";
               in.close();
               continue;
          } 
          numElements[i] = sizeBytes / sizeof(int64_t);
          in.close();
          quicksortExternal(subFilename, subFilename, numElements[i], depth + 1);
     }
     mergeFiles(filenameSorted, N, depth);
     return;
} 

void reiniciar_contador_IOs() {
     readnwrite = 0;
}

int main() {
     srand(static_cast<unsigned>(time(0)));
     for (size_t i = 0; i < 5; ++i) {
          reiniciar_contador_IOs();
          std::string filename = "datos_60M_" + std::to_string(i) + ".bin";
          std::cout << "Comienzo con el archivo: " << filename << std::endl;
          std::string filenameSorted = "SORTED_60M_" + std::to_string(i) + "_" + std::to_string(a) + ".bin";
          size_t MB = 50 * 1024 * 1024;
          size_t Nbytes = 60 * MB;
          size_t N = Nbytes / sizeof(int64_t);  
          clock_t inicio = clock();
          quicksortExternal(filename, filenameSorted, N);
          clock_t fin = clock();
          double segundos = (double)(fin - inicio) / CLOCKS_PER_SEC;
          std::cout << "Cantidad de Read y Writes: " << readnwrite << std::endl;
          std::cout << "Tiempo total: " << segundos << " segundos." << std::endl;
          std::cout << "Proxima iteracion: " << i+1 << std::endl;
          std::cout << "----------------------------------------\n" << std::endl;
     }
     return 0;
}