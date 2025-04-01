#include "str_builder.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int init_string_builder(struct string_builder *sb, size_t initial_capacity) {
  sb->buffer = (char *)malloc(initial_capacity);
  if (sb->buffer == NULL) {
    perror("Failed to allocate memory");
    return -1;
  }

  sb->buffer[0] = '\0';
  sb->capacity = initial_capacity;
  sb->size = 0;

  return 0;
}

int append_string(struct string_builder *sb, const char *str) {
  size_t str_len = strlen(str);

  while (sb->size + str_len >= sb->capacity) {
    sb->capacity *= 2;
    sb->buffer = (char *)realloc(sb->buffer, sb->capacity);
    if (sb->buffer == NULL) {
      perror("Failed to reallocate memory");
      return -1;
    }
  }

  strcpy(sb->buffer + sb->size, str);
  sb->size += str_len;

  return 0;
}

int append_char(struct string_builder *sb, const char c) {
  if (sb->size + 1 >= sb->capacity) {
    sb->capacity *= 2;
    sb->buffer = (char *)realloc(sb->buffer, sb->capacity);
    if (sb->buffer == NULL) {
      perror("Failed to reallocate memory");
      return -1;
    }
  }

  sb->buffer[sb->size] = c;
  sb->size++;
  sb->buffer[sb->size] = '\0';

  return 0;
}

const char *get_string(struct string_builder *sb) { return sb->buffer; }

void free_string_builder(struct string_builder *sb) {
  free(sb->buffer);
  sb->buffer = NULL;
  sb->size = sb->capacity = 0;
}
