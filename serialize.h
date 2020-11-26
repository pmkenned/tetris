#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    INIT_BUFFER_SIZE = 1024,
    MAGIC32 = 0x73c6eeb7
};

typedef struct {
    void * buffer;
    size_t index;
    size_t len_bytes;
    size_t cap_bytes;
} serial_buffer_t;

void serialize(serial_buffer_t * sb, const void * x, size_t size);
void deserialize(serial_buffer_t * sb, void * x, size_t size);
void serial_buffer_reserve(serial_buffer_t * sb, size_t num_bytes);
serial_buffer_t * serial_buffer_create();
serial_buffer_t * serial_buffer_create_from_file(FILE * fp);
void serial_buffer_write_to_file(serial_buffer_t * sb, FILE * fp);

#endif /* SERIALIZE_H */
