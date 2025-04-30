#ifndef QUICKSORT_E_HPP
#define QUICKSORT_E_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//crear funciones que lean bloques
//crear funciones que escriban bloques
//elegir a-1 pivotes aleatorios para separar el arreglo en a partes, quicksort

void readBlock(const std::string &filename, size_t posicion, size_t B, std::vector<int64_t>& buffer);
void writeBlock(const std::string &filename, size_t posicion, size_t B, const std::vector<int64_t>& buffer);
//leer un bloque de filename aleatorio y elegir a-1 subarreglos random que seran pivotes y ordenarlos
std::vector<int64_t> randomInterval(const std::string &filename, size_t N, size_t B, size_t a);

#endif