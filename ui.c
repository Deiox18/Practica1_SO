#include "common.h"
#include <ctype.h>   // para tolower (si quieres hacer búsqueda insensible)

int main() {
    int req_fd = open(FIFO_REQ, O_WRONLY);
    int resp_fd = open(FIFO_RESP, O_RDONLY);
    if (req_fd < 0 || resp_fd < 0) {
        printf("No se pudo conectar con el servidor. ¿Está ejecutándose?\n");
        return 1;
    }

    char id[50] = "";
    char nombre[100] = "";
    char ciudad[100] = "";
    int option;
    int c;

    do {
        printf("\n=== MENÚ DE BÚSQUEDA ===\n");
        printf("1. Ingresar ID de persona (exacto)\n");
        printf("2. Ingresar nombre (parte del nombre, ej: 'Tim' o 'Sanchez')\n");
        printf("3. Ingresar ciudad (parte del nombre de ciudad)\n");
        printf("4. Realizar búsqueda (se aplican todos los criterios no vacíos)\n");
        printf("5. Salir\n");
        printf("Opción: ");

        if (scanf("%d", &option) != 1) {
            while ((c = getchar()) != '\n' && c != EOF);
            option = 0;
        } else {
            while ((c = getchar()) != '\n' && c != EOF);
        }

        switch (option) {
            case 1:
                printf("ID : ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = '\0';
                break;
            case 2:
                printf("Nombre (puede ser solo parte): ");
                fgets(nombre, sizeof(nombre), stdin);
                nombre[strcspn(nombre, "\n")] = '\0';
                break;
            case 3:
                printf("Ciudad (puede ser solo parte): ");
                fgets(ciudad, sizeof(ciudad), stdin);
                ciudad[strcspn(ciudad, "\n")] = '\0';
                break;
            case 4: {
                char request[MAX_LINE_LEN];
                snprintf(request, sizeof(request), "%s|%s|%s", id, nombre, ciudad);
                write(req_fd, request, strlen(request));

                printf("\nResultados:\n");
                char resp[MAX_LINE_LEN];
                while (1) {
                    int n = read(resp_fd, resp, sizeof(resp) - 1);
                    if (n <= 0) break;
                    resp[n] = '\0';
                    if (strncmp(resp, "END", 3) == 0) break;
                    printf("%s", resp);
                }
                printf("\n--- Presiona Enter para continuar ---");
                while ((c = getchar()) != '\n' && c != EOF);
                break;
            }
            case 5:
                printf("Saliendo...\n");
                break;
            default:
                printf("Opción no válida.\n");
                while ((c = getchar()) != '\n' && c != EOF);
        }
    } while (option != 5);

    close(req_fd);
    close(resp_fd);
    return 0;
}