#ifndef COMMON_H
#define COMMON_H


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
// Tamaño máximo de una línea del dataset (suficiente para todos)
#define MAX_LINE_LEN 500
// Rutas de las tuberías para la comunicación entre proceso
#define FIFO_REQ "/tmp/search_req"
#define FIFO_RESP "/tmp/search_resp"
// Tamaño del bloque para el índice
#define BLOQUE_SIZE 1000  

// Estructura para los bloques
typedef struct {
    uint32_t person_id;
    uint32_t offset;
} BloqueInfo;

#endif
