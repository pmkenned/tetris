#include "serialize.h"

void
serial_buffer_reserve(serial_buffer_t * sb, size_t num_bytes)
{
    size_t needed = sb->index + num_bytes;
    if (needed > sb->cap_bytes) {
        while (needed > sb->cap_bytes) {
            sb->cap_bytes *= 2;
        }
        sb->buffer = realloc(sb->buffer, sb->cap_bytes);
        assert(sb->cap_bytes >= needed);
    }
}

serial_buffer_t *
serial_buffer_create()
{
    serial_buffer_t * sb = malloc(sizeof(*sb));
    sb->cap_bytes = INIT_BUFFER_SIZE;
    sb->buffer = malloc(INIT_BUFFER_SIZE);
    sb->index = 0;
    sb->len_bytes = 0;
    return sb;
}

serial_buffer_t *
serial_buffer_create_from_file(FILE * fp)
{
    serial_buffer_t * sb = serial_buffer_create();
    uint32_t magic;
    fread(&magic, sizeof(magic), 1, fp);
    if (magic != MAGIC32) {
        fprintf(stderr, "error: invalid file for deserialization\n");
        exit(EXIT_FAILURE);
    }
    while (1) {
        size_t bytes_read = fread((char *)sb->buffer + sb->index, 1, INIT_BUFFER_SIZE, fp);
        sb->index += bytes_read;

        if (bytes_read == INIT_BUFFER_SIZE) {
            serial_buffer_reserve(sb, INIT_BUFFER_SIZE);
        } else {
            assert(!ferror(fp));
            break;
        }

    }
    sb->len_bytes = sb->index;
    sb->index = 0;
    return sb;
}

void
serialize(serial_buffer_t * sb, const void * x, size_t size)
{
    serial_buffer_reserve(sb, size);
    memcpy((char *)sb->buffer + sb->index, x, size);
    sb->index += size;
    sb->len_bytes = sb->index;
}

void
deserialize(serial_buffer_t * sb, void * x, size_t size)
{
    assert(sb->index + size <= sb->len_bytes);
    memcpy(x, (char *)sb->buffer + sb->index, size);
    sb->index += size;
}

void
serial_buffer_write_to_file(serial_buffer_t * sb, FILE * fp)
{
    uint32_t magic = MAGIC32;
    assert(sb->index == sb->len_bytes);
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(sb->buffer, 1, sb->len_bytes, fp);
}
