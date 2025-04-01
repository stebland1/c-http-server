#ifndef STR_BUILDER_H
#define STR_BUILDER_H

#include <stddef.h>

struct string_builder {
  char *buffer;
  size_t size;
  size_t capacity;
};

int init_string_builder(struct string_builder *sb, size_t initial_capacity);
int append_string(struct string_builder *sb, const char *str);
int append_char(struct string_builder *sb, const char c);
const char *get_string(struct string_builder *sb);
void free_string_builder(struct string_builder *sb);

#endif
