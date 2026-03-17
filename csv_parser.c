#include <stdio.h>
#include <string.h>

int parse_csv_line(char *line, char *fields[], int max_fields) {
    int field_count = 0;
    char *token = strtok(line, ",");
    while (token && field_count < max_fields) {
        fields[field_count++] = token;
        token = strtok(NULL, ",");
    }
    return field_count;
}