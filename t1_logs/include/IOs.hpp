#ifndef CONTADOR_IO_HPP
#define CONTADOR_IO_HPP

/**
 * Contador de operaciones I/O
 * Se utiliza para llevar un registro de la cantidad de lecturas y escrituras realizadas
 */
extern size_t lecturas_escrituras;

/**
 * Reinicia el contador de operaciones I/O
 */
void reiniciar_contador_IOs();

#endif
