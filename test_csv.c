#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaramos la función (o incluimos un .h)
int parse_csv_line(char *line, char *fields[], int max_fields);

int main() {
    FILE *f = fopen("C:\\Users\\dieve\\Documents\\Practica1_S0\\dataset.csv", "r");
    if (!f) {
        perror("No se pudo abrir el archivo");
        return 1;
    }

    char line[20000]; // Suficiente para una línea larga
    int line_num = 0;

    // Leer y mostrar las primeras 5 líneas (después de la cabecera)
    fgets(line, sizeof(line), f); // Saltar cabecera (si existe)

    while (fgets(line, sizeof(line), f) && line_num < 5) {
        line_num++;
        printf("\n--- Línea %d (original):\n%s", line_num, line);

        // Hacemos una copia porque parse_csv_line modifica la línea (aunque no debería,
        // pero por seguridad usamos una copia)
        char buffer[20000];
        strcpy(buffer, line);

        char *fields[10]; // Suponemos máximo 10 campos
        int n = parse_csv_line(buffer, fields, 10);

        printf("Campos encontrados: %d\n", n);
        for (int i = 0; i < n; i++) {
            printf("  Campo %d: '%s'\n", i, fields[i]);
            free(fields[i]); // Liberamos cada campo
        }
    }

    fclose(f);
    return 0;
}