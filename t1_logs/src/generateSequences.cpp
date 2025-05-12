#include "../include/generateSequences.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

void generateSequences(int64_t N, const std::string &filename) {
  // Definición de tamaño de buffer
  const size_t BufferMB = 5;
  const size_t BufferSize = (BufferMB * 1024 * 1024) / sizeof(int64_t);
  int64_t totalEnteros = N / sizeof(int64_t);
  int64_t porSecuencia = totalEnteros / 5;

  // Se crean 5 archivos temporales para almacenar secuencias y despues
  // escribirlas en los archivos reales
  std::vector<std::string> temp = {
      "temp1.bin", "temp2.bin", "temp3.bin", "temp4.bin", "temp5.bin"
  };

  // Generador de números aleatorios
  std::mt19937_64 gen(std::random_device{}());
  std::uniform_int_distribution<int64_t> dist(INT64_MIN, INT64_MAX);

  // Generar 5 secuencias de tamaño N cada una
  for (int i = 0; i < 5; ++i) {
    std::ofstream out(temp[i], std::ios::binary);
    std::vector<int64_t> buffer;
    buffer.reserve(BufferSize);

    size_t escritos = 0;
    while (escritos < porSecuencia) {
      buffer.clear();
      size_t length =
          std::min(BufferSize, static_cast<size_t>(porSecuencia - escritos));
      for (size_t j = 0; j < length; j++) {
        buffer.push_back(dist(gen));
      }

      out.write(
          reinterpret_cast<const char *>(buffer.data()),
          buffer.size() * sizeof(int64_t)
      );
      escritos += length;
    }

    out.close();
  }

  std::vector<std::ifstream> entradas;
  for (const auto &f : temp) {
    entradas.emplace_back(f, std::ios::binary);
  }

  std::ofstream salida(filename, std::ios::binary);

  const size_t chunkSizePerFile = BufferSize / 5; // ~1MB por archivo
  std::vector<int64_t> tempBuffer(chunkSizePerFile);
  std::vector<int64_t> mezclaBuffer;
  mezclaBuffer.reserve(BufferSize);

  size_t leidos = 0;
  while (leidos < totalEnteros) {
    mezclaBuffer.clear();

    for (auto &in : entradas) {
      size_t porLeer = std::min(
          chunkSizePerFile, static_cast<size_t>(totalEnteros - leidos)
      );
      in.read(
          reinterpret_cast<char *>(tempBuffer.data()), porLeer * sizeof(int64_t)
      );
      size_t leidoReal = in.gcount() / sizeof(int64_t);
      mezclaBuffer.insert(
          mezclaBuffer.end(), tempBuffer.begin(), tempBuffer.begin() + leidoReal
      );
      leidos += leidoReal;
    }

    std::shuffle(mezclaBuffer.begin(), mezclaBuffer.end(), gen);
    salida.write(
        reinterpret_cast<const char *>(mezclaBuffer.data()),
        mezclaBuffer.size() * sizeof(int64_t)
    );
  }

  salida.close();
  for (auto &in : entradas) {
    in.close();
  }

  for (const auto &f : temp) {
    std::remove(f.c_str());
  }

  std::cout << "Archivo generado: " << filename << " (" << totalEnteros
            << " enteros, " << N / (1024 * 1024) << " MB)" << std::endl;
}

// int main() {
//     const size_t MB = 50 * 1024 * 1024;

//     for (int i = 0; i < 5; i++) {
//       size_t total_bytes = 4 * MB;
//       //std::string nombre = "datos_" + std::to_string(60) + "M.bin";
//       char nombre[64];
//       snprintf(nombre, sizeof(nombre), "datos_4M_%d.bin", i);
//       generateSequences(total_bytes, nombre);
//     }
//     //for (int m = 4; m <= 60; m += 4) {
//     //}

//     return 0;
//   }