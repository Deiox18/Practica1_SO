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

#define MAX_LINE_LEN 500
#define FIFO_REQ "/tmp/search_req"
#define FIFO_RESP "/tmp/search_resp"
#define BLOQUE_SIZE 1000  // cada 1000 registros guardamos un punto de control

// Estructura para el índice de bloques (en memoria)
typedef struct {
    uint32_t person_id;
    uint32_t offset;
} BloqueInfo;

#endif