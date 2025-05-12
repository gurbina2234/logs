# Tarea 1 - Diseño y Análisis de Algoritmos
## Introducción

Este repositorio contiene implementaciones de algoritmos de ordenamiento externo diseñados para manejar grandes volúmenes de datos que no caben completamente en memoria RAM. Los algoritmos implementados son:

- **QuickSort Externo**: Una adaptación del algoritmo QuickSort clásico para trabajar con archivos en disco.
- **MergeSort Externo**: Una implementación del algoritmo MergeSort adaptada para trabajar con archivos en disco.

## Estructura del Proyecto

```
.
├── CMakeLists.txt
├── Makefile
├── README.md
├── estructura.txt
├── include
│   ├── IOs.hpp
│   ├── generateSequences.hpp
│   ├── mergesort_e.hpp
│   └── quicksort_e.hpp
└── src
    ├── experimento.cpp
    ├── generateSequences.cpp
    ├── mergesort_e.cpp
    ├── programa
    └── quicksort_e.cpp

3 directories, 13 files

```
## Componentes Principales

### QuickSort Externo

El algoritmo QuickSort Externo divide el problema en subproblemas más pequeños utilizando pivotes, similar al QuickSort tradicional, pero adaptado para trabajar con datos en disco.

#### Componentes clave:

- **Selección de pivotes**: Utiliza la función `randomInterval` para seleccionar pivotes aleatorios que dividen el conjunto de datos.
- **División en subarreglos**: Clasifica los elementos en subarreglos según los pivotes seleccionados.
- **Ordenamiento recursivo**: Ordena cada subarreglo de forma recursiva.
- **Fusión de resultados**: Combina los subarreglos ordenados mediante la función `mergeFiles`.

#### Parámetros de configuración:
- Tamaño de bloque (B): 4096 bytes
- Memoria disponible (M): ~48MB
- Aridad (a): 192 (número de subarreglos)

### MergeSort Externo

El algoritmo MergeSort Externo divide el archivo original en archivos intermedios ordenados y luego los fusiona de manera eficiente.

#### Componentes clave:

- **Generación de archivos intermedios**: La función `generar_middle_files` divide el archivo original en archivos intermedios ordenados.
- **Actualización de buffers**: La función `update_mf_buffer` gestiona la lectura de bloques desde los archivos intermedios.
- **Selección de mínimos**: La función `seleccionar_minimo` implementa la lógica para el merge k-way.
- **Fusión jerárquica**: Las funciones `merge_tanda` y `merge_cant_sobre_a` implementan la fusión de múltiples archivos.

#### Parámetros de configuración:
- Tamaño de bloque (B_BYTES): 4096 bytes
- Memoria disponible (M_BYTES): 50MB
- Memoria reservada (RESERVA_RAM): 48MB
- Aridad (ARIDAD): 192

## Funciones de Utilidad

### Operaciones de I/O

Ambos algoritmos utilizan funciones específicas para optimizar las operaciones de entrada/salida:

- **readBlock/writeBlock**: Leen/escriben bloques de datos desde/hacia archivos binarios.
- **readAllMemory**: Lee múltiples bloques consecutivos en memoria.
- **Contador de I/O**: Se mantiene un contador de operaciones de lectura/escritura para análisis de rendimiento.

### Visualización de Progreso

- **printProgressBar**: Muestra una barra de progreso en la consola para visualizar el avance del procesamiento.

## Estructura de Datos

### MergeSort Externo

- **Estructura Mid**: Representa un archivo intermedio con su buffer y metadatos asociados.

```c
typedef struct {
    FILE* archivo;                  // Puntero al archivo mid_#.bin
    int64_t buffer[NUMS_POR_BLOQUE]; // Buffer de 1 bloque (512 números de 64 bits)
    size_t pos;                     // Posición actual dentro del buffer
    size_t usados;                  // Cantidad de elementos válidos en el buffer
    bool terminado;                 // Indica si ya se procesó todo el archivo
} Mid;
```

## Flujo de Ejecución

### QuickSort Externo

1. **Verificación de tamaño**: Si el tamaño del archivo es menor que la memoria disponible (M), se carga completamente en memoria, se ordena y se escribe de vuelta al disco.

2. **Selección de pivotes**: Si el archivo es grande, se seleccionan pivotes aleatorios (a-1 pivotes) para dividir el conjunto de datos en a subarreglos.

3. **Clasificación en subarreglos**:
   - Se leen bloques del archivo original en memoria.
   - Cada elemento se clasifica en uno de los a subarreglos según los pivotes.
   - Los subarreglos se escriben en archivos temporales cuando alcanzan el tamaño de un bloque.

4. **Ordenamiento recursivo**:
   - Para cada subarreglo, se llama recursivamente a quicksortExternal.
   - La recursión termina cuando un subarreglo es lo suficientemente pequeño para caber en memoria.

5. **Fusión de resultados**:
   - Una vez ordenados todos los subarreglos, se fusionan en un único archivo ordenado.
   - La fusión se realiza leyendo secuencialmente cada archivo temporal y combinando los resultados.

### MergeSort Externo

1. **Generación de archivos intermedios**:
   - Se lee el archivo original en bloques del tamaño máximo que cabe en memoria.
   - Cada bloque se ordena en memoria usando std::sort.
   - Los bloques ordenados se escriben en archivos intermedios (mid_#.bin).

2. **Estrategia de fusión**:
   - Si hay más archivos intermedios que la aridad máxima (ARIDAD), se implementa una fusión jerárquica.
   - De lo contrario, se fusionan directamente todos los archivos.

3. **Fusión k-way**:
   - Se mantiene un buffer para cada archivo intermedio.
   - En cada paso, se selecciona el valor mínimo entre los primeros elementos de cada buffer.
   - El valor seleccionado se escribe en el archivo de salida.
   - Cuando un buffer se vacía, se recarga desde su archivo correspondiente.

4. **Limpieza**: Al finalizar, se eliminan todos los archivos temporales generados.

## Optimizaciones

### QuickSort Externo

1. **Manejo eficiente de memoria**:
   - Se utiliza la memoria disponible para procesar la mayor cantidad posible de datos en cada paso.
   - Si todo el archivo cabe en memoria, se evita la recursión y se ordena directamente.

2. **Selección estratégica de pivotes**:
   - Los pivotes se seleccionan de un bloque aleatorio para garantizar una buena distribución.
   - Se asegura que los pivotes sean valores únicos para evitar subarreglos desequilibrados.

3. **Procesamiento por lotes**:
   - Los datos se procesan en lotes del tamaño máximo que cabe en memoria para reducir las operaciones de I/O.
   - Se utiliza una barra de progreso para visualizar el avance del procesamiento.

4. **Manejo de casos especiales**:
   - Se detectan y manejan correctamente los subarreglos vacíos.
   - Se gestionan adecuadamente los bloques incompletos al final de los archivos.

### MergeSort Externo

1. **Fusión k-way optimizada**:
   - Se implementa una fusión k-way que permite combinar múltiples archivos simultáneamente.
   - Se utiliza una estructura de datos eficiente para seleccionar el valor mínimo en cada paso.

2. **Fusión jerárquica**:
   - Cuando hay demasiados archivos intermedios, se implementa una estrategia de fusión por niveles.
   - Esto reduce la cantidad de archivos abiertos simultáneamente y mejora el rendimiento.

3. **Buffers de tamaño fijo**:
   - Se utilizan buffers de tamaño fijo para minimizar las operaciones de I/O.
   - Cada buffer tiene el tamaño exacto de un bloque de disco.

4. **Detección temprana de finalización**:
   - Se detecta cuando un archivo intermedio se ha procesado completamente para evitar operaciones innecesarias.

## Consideraciones de Rendimiento

- **Operaciones de I/O**: El rendimiento de estos algoritmos está principalmente limitado por las operaciones de entrada/salida. Se mantiene un contador para analizar la eficiencia.

- **Parámetros configurables**: La elección de los parámetros (tamaño de bloque, memoria disponible, aridad) afecta significativamente el rendimiento:
  - Un tamaño de bloque mayor reduce las operaciones de I/O pero aumenta el tiempo de procesamiento por bloque.
  - Una aridad mayor permite una mejor distribución de los datos pero aumenta la complejidad de la fusión.

- **Equilibrio de subarreglos**: En QuickSort Externo, la calidad de los pivotes seleccionados afecta directamente el equilibrio de los subarreglos y, por tanto, el rendimiento global.

- **Fusión jerárquica**: En MergeSort Externo, la estrategia de fusión jerárquica es crucial cuando hay muchos archivos intermedios, ya que reduce la cantidad de archivos abiertos simultáneamente.

## Uso

### QuickSort Externo

```cpp
#include "quicksort_e.hpp"
#include <iostream>

int main() {
    // Nombre del archivo de entrada
    std::string inputFile = "datos.bin";
    
    // Nombre del archivo de salida ordenado
    std::string outputFile = "datos_ordenados.bin";
    
    // Tamaño del archivo en número de elementos (int64_t)
    size_t fileSize = 1000000; // Ejemplo: 1 millón de elementos
    
    // Inicializar el generador de números aleatorios (para selección de pivotes)
    std::srand(std::time(nullptr));
    
    // Ejecutar QuickSort Externo
    std::cout << "Iniciando QuickSort Externo..." << std::endl;
    quicksortExternal(inputFile, outputFile, fileSize);
    std::cout << "Ordenamiento completado." << std::endl;
    
    return 0;
}
```

### MergeSort Externo

```cpp
#include "mergesort_e.hpp"
#include "IOs.hpp"
#include <iostream>

int main() {
    // Nombre del archivo de entrada
    char inputFile[] = "datos.bin";
    
    // Tamaño del archivo en MB (para nombrar archivos)
    int fileSizeMB = 100; // Ejemplo: archivo de 100MB
    
    // Variables para almacenar información sobre archivos intermedios
    size_t midFilesCount = 0;
    int supMidFilesCount = 0;
    
    // Reiniciar contador de operaciones I/O
    reiniciar_contador_IOs();
    
    // Fase 1: Generar archivos intermedios
    std::cout << "Generando archivos intermedios..." << std::endl;
    generar_middle_files(inputFile, &midFilesCount);
    
    // Fase 2: Fusionar archivos intermedios
    std::cout << "Fusionando archivos intermedios..." << std::endl;
    merge_middle_files(midFilesCount, fileSizeMB, &supMidFilesCount, 0);
    
    // Fase 3: Limpiar archivos temporales
    std::cout << "Limpiando archivos temporales..." << std::endl;
    borrar_middle_files(midFilesCount, supMidFilesCount, fileSizeMB);
    
    // Mostrar estadísticas
    std::cout << "Ordenamiento completado." << std::endl;
    std::cout << "Total de operaciones I/O: " << lecturas_escrituras << std::endl;
    
    return 0;
}
```
## Compilación
### Compilar el proyecto
Se debe compilar en un contenedor de Docker en `t1_logs` usando un límite de memoria 500MB.

```
docker run --rm -it -m 500m -v "$PWD":/workspace pabloskewes/cc4102-cpp-env bash
```
Luego dentro del contenedor, se debe realizar lo siguiente para crear el ejecutable `./ExpSort`.

````
mkdir build
cd build
cmake ..
cmake --build .
````
En caso de que `cmake`no esté instalado dentro del contenedor, se debe instalar lo siguiente:

````
apt update
apt install -y cmake
````
### Ejecutar el experimento
Se debe ejecutar en un contenedor de Docker  en `t1_logs` usando un límite de memoria 50MB.

```
docker run --rm -it -m 500m -v "$PWD":/workspace pabloskewes/cc4102-cpp-env bash
```

Luego dentro del contenedor, se debe realizar lo siguiente para ejecutar los experimentos.

````
cd build 
./ExpSort
````
