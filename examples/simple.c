#include <stdio.h>
#include <stdlib.h>

#include "jsmn.h"

void start_arr(void) {
    printf("Array started\n");
}
void end_arr(void) {
    printf("Array ended\n");
}
void start_obj(void) {
    printf("Object started\n");
}
void end_obj(void) {
    printf("Object ended\n");
}
void obj_key(const char *key, size_t key_len) {
    printf("Object key: %s\n", key);
}
void str(const char *value, size_t len) {
    printf("String: %s\n", value);
}
void primitive(const char *value, size_t len) {
    printf("Primitive: %s\n", value);
}

jsmn_callbacks_t cbs = {
    start_arr,
    end_arr,
    start_obj,
    end_obj,
    obj_key,
    str,
    primitive
};
jsmn_parser parser;

int main(void) {
    FILE *infile = fopen("forecast.json", "r");

    jsmn_init(&parser, &cbs);

    size_t read_count;
    int ch;
    while ((ch = fgetc(infile)) != EOF) {
        jsmn_parse(&parser, (char)ch);
    }

    fclose(infile);
    return EXIT_SUCCESS;
}

