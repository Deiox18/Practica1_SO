#include "common.h"
#include <ctype.h>  // opcional

#define MAX_NODOS 400000   // Límite de nodos (ya no se usa porque usamos índice de bloques, pero lo dejamos por si acaso)
#define BLOQUE_SIZE 1000   // Tamaño de bloque para el índice



BloqueInfo *bloques = NULL;
int num_bloques = 0;
FILE *csv_file = NULL;
char *csv_filename = "personas.csv";
char *idx_filename = "bloques.idx";

// Declaraciones
int parse_csv_line(char *line, char *fields[], int max_fields);
void build_index();
int buscar_bloque(uint32_t id);
int cumple_filtros(char *fields[], int n, const char *nombre, const char *ciudad);
int search_by_id(uint32_t person_id, const char *nombre, const char *ciudad, char *result);
void search_sequential(const char *nombre, const char *ciudad, int resp_fd);
void cleanup(int sig);

void cleanup(int sig) {
    unlink(FIFO_REQ);
    unlink(FIFO_RESP);
    if (bloques) free(bloques);
    if (csv_file) fclose(csv_file);
    exit(0);
}

// Construye el índice de bloques y lo guarda en disco, además lo carga en memoria
void build_index() {
    FILE *csv = fopen(csv_filename, "r");
    if (!csv) {
        perror("No se pudo abrir el CSV");
        exit(1);
    }

    FILE *idx = fopen(idx_filename, "wb");
    if (!idx) {
        perror("No se pudo crear el archivo de índice");
        fclose(csv);
        exit(1);
    }

    char line[MAX_LINE_LEN];
    long offset;
    long line_count = 0;
    BloqueInfo bi;

    // Saltar cabecera
    if (!fgets(line, sizeof(line), csv)) {
        perror("Error leyendo cabecera");
        fclose(csv);
        fclose(idx);
        exit(1);
    }

    while (1) {
        offset = ftell(csv);
        if (!fgets(line, sizeof(line), csv)) break;

        if (line_count % BLOQUE_SIZE == 0) {
            char line_copy[MAX_LINE_LEN];
            strcpy(line_copy, line);
            char *fields[15];
            int n = parse_csv_line(line_copy, fields, 15);
            if (n >= 1) {
                bi.person_id = atoi(fields[0]);
                bi.offset = (uint32_t)offset;
                fwrite(&bi, sizeof(BloqueInfo), 1, idx);
            }
        }
        line_count++;
        if (line_count % 100000 == 0) {
            printf("Procesadas %ld líneas...\n", line_count);
            fflush(stdout);
        }
    }

    fclose(csv);
    fclose(idx);
    printf("Índice de bloques creado con %ld puntos de control.\n", line_count / BLOQUE_SIZE + 1);

    // Cargar el índice en memoria
    idx = fopen(idx_filename, "rb");
    if (!idx) {
        perror("No se pudo abrir el índice");
        exit(1);
    }
    fseek(idx, 0, SEEK_END);
    long sz = ftell(idx);
    num_bloques = sz / sizeof(BloqueInfo);
    bloques = (BloqueInfo*)malloc(sz);
    if (!bloques) {
        perror("malloc");
        fclose(idx);
        exit(1);
    }
    fseek(idx, 0, SEEK_SET);
    fread(bloques, sizeof(BloqueInfo), num_bloques, idx);
    fclose(idx);
    printf("Índice cargado en memoria: %d bloques, %.2f KB\n", num_bloques, sz/1024.0);
}

// Búsqueda binaria en el índice de bloques
int buscar_bloque(uint32_t id) {
    int izq = 0, der = num_bloques - 1;
    while (izq <= der) {
        int mid = (izq + der) / 2;
        if (bloques[mid].person_id == id) {
            return mid;
        } else if (bloques[mid].person_id < id) {
            izq = mid + 1;
        } else {
            der = mid - 1;
        }
    }
    return der; // puede ser -1 si es menor que el primero
}

// Función que verifica si una línea cumple los filtros de nombre y ciudad
int cumple_filtros(char *fields[], int n, const char *nombre, const char *ciudad) {
    if (n < 10) return 0;

    // Verificar nombre (en firstname o lastname)
    if (nombre[0] != '\0') {
        char *firstname = fields[1];
        char *lastname = fields[2];
        if (strstr(firstname, nombre) == NULL && strstr(lastname, nombre) == NULL) {
            return 0;
        }
    }

    // Verificar ciudad
    if (ciudad[0] != '\0') {
        if (strstr(fields[9], ciudad) == NULL) {
            return 0;
        }
    }

    return 1;
}

// Búsqueda por ID (rápida, usando índice de bloques)
int search_by_id(uint32_t person_id, const char *nombre, const char *ciudad, char *result) {
    int bloque_idx = buscar_bloque(person_id);
    if (bloque_idx < 0) bloque_idx = 0;

    fseek(csv_file, bloques[bloque_idx].offset, SEEK_SET);

    char line[MAX_LINE_LEN];
    int lineas_leidas = 0;
    while (lineas_leidas < BLOQUE_SIZE && fgets(line, sizeof(line), csv_file)) {
        lineas_leidas++;
        char line_copy[MAX_LINE_LEN];
        strcpy(line_copy, line);
        char *fields[15];
        int n = parse_csv_line(line_copy, fields, 15);
        if (n >= 10) {
            uint32_t current_id = atoi(fields[0]);
            if (current_id == person_id) {
                if (cumple_filtros(fields, n, nombre, ciudad)) {
                    strcpy(result, line);
                    return 1;
                }
            } else if (current_id > person_id) {
                break;
            }
        }
    }
    return 0;
}

// Búsqueda secuencial (para cuando no hay ID)
void search_sequential(const char *nombre, const char *ciudad, int resp_fd) {
    char line[MAX_LINE_LEN];
    long current_pos = ftell(csv_file); // guardar posición actual

    // Volver al inicio de los datos (después de la cabecera)
    fseek(csv_file, 0, SEEK_SET);
    fgets(line, sizeof(line), csv_file); // saltar cabecera

    int encontrados = 0;
    while (fgets(line, sizeof(line), csv_file)) {
        char line_copy[MAX_LINE_LEN];
        strcpy(line_copy, line);
        char *fields[15];
        int n = parse_csv_line(line_copy, fields, 15);
        if (n >= 10) {
            if (cumple_filtros(fields, n, nombre, ciudad)) {
                write(resp_fd, line, strlen(line));
                encontrados++;
            }
        }
    }

    // Restaurar posición original
    fseek(csv_file, current_pos, SEEK_SET);

    if (encontrados == 0) {
        write(resp_fd, "NA\n", 3);
    }
}

int main() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    // Abrir el archivo CSV (se mantendrá abierto)
    csv_file = fopen(csv_filename, "r");
    if (!csv_file) {
        perror("No se pudo abrir el CSV");
        exit(1);
    }

    // Construir o cargar el índice
    build_index();

    // Crear FIFOs
    mkfifo(FIFO_REQ, 0666);
    mkfifo(FIFO_RESP, 0666);

    int req_fd = open(FIFO_REQ, O_RDONLY);
    int resp_fd = open(FIFO_RESP, O_WRONLY);
    if (req_fd < 0 || resp_fd < 0) {
        perror("Error abriendo FIFOs");
        exit(1);
    }

    char buffer[MAX_LINE_LEN];
    while (1) {
        int n = read(req_fd, buffer, sizeof(buffer)-1);
        if (n <= 0) continue;
        buffer[n] = '\0';

        char *id = strtok(buffer, "|");
        char *nombre = strtok(NULL, "|");
        char *ciudad = strtok(NULL, "|");

        if (!nombre) nombre = "";
        if (!ciudad) ciudad = "";

        if (id && id[0] != '\0') {
            // Búsqueda por ID
            uint32_t person_id = atoi(id);
            char result[MAX_LINE_LEN];
            if (search_by_id(person_id, nombre, ciudad, result)) {
                write(resp_fd, result, strlen(result));
                write(resp_fd, "\n", 1);
            } else {
                write(resp_fd, "NA\n", 3);
            }
        } else {
            // Búsqueda sin ID (secuencial)
            search_sequential(nombre, ciudad, resp_fd);
        }
        write(resp_fd, "END\n", 4);
    }

    close(req_fd);
    close(resp_fd);
    cleanup(0);
    return 0;
}