#ifndef DIR_LIST_H
#define DIR_LIST_H

#include <stddef.h>

struct dir_list {
  char **files;
  size_t count;
};

void init_dir_list(struct dir_list *dl);
void free_dir_list(struct dir_list *dl);
int read_dir(char *path, struct dir_list *dl);

#endif
