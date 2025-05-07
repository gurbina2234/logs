#ifndef QUICKSORT_E_HPP
#define QUICKSORT_E_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void readBlock(const std::string &filename, size_t posicion, std::vector<int64_t> &buffer);
void writeBlock(const std::string &filename, size_t posicion, std::vector<int64_t> &buffer);
std::vector<int64_t> randomInterval(const std::string &filename, size_t N);
void readAllMemory(const std::string &filename,size_t startBlock, size_t numBlocks, std::vector<int64_t> &buffer);
void mergeFiles(const std::string &filenameSorted, size_t N, size_t depth);
void quicksortExternal(const std::string &filename, const std::string &filenameSorted, size_t N, size_t depth = 0);
void reiniciar_contador_IOs();

#endif