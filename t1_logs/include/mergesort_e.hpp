
typedef struct Mid;

int comparar_64(const void *a, const void *b);
void generar_middle_files(char *datos_desordenados, size_t *cantidad_mid_files);
void merge_middle_files(size_t cantidad, int cuantosM, int *cantidadSupMid, int iteracion);
void borrar_middle_files(size_t cantidad_mids, int cantidad_sup_mids, int cuantosM);
void reiniciar_contador_IOs();
void update_mf_buffer(Mid *mid);
void seleccionar_minimo(Mid *mids, int cantidad_mids, int *idx);
void merge_tanda(int offset, int cantidad, char *nombre_salida);
void merge_cant_sobre_a(size_t cantidad, int cuociente, int residuo, int cuantosM, int *cantidadSupMid);
void merge_middle_files(size_t cantidad, int cuantosM, int *cantidadSupMid, int iteracion);
void borrar_middle_files(size_t cantidad_mids, int cantidad_sup_mids, int cuantosM);
void reiniciar_contador_IOs();