#include "dir_list.h"
#include "utils.h"
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void init_dir_list(struct dir_list *dl) {
  dl->files = NULL;
  dl->count = 0;
}

int read_dir(char *path, struct dir_list *dl) {
  if (path[0] == '/') {
    path++;
  }

  if (path[0] == '\0') {
    path = ".";
  }

  DIR *fd = opendir(path);
  if (fd == NULL) {
    perror("Failed to open dir");
    return -1;
  }

  struct dirent *entry;
  size_t capacity = 10;
  dl->files = malloc(capacity * sizeof(char *));
  if (dl->files == NULL) {
    perror("Failed to allocate memory for dir list");
    closedir(fd);
    return -1;
  }

  while ((entry = readdir(fd)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    if (dl->count == capacity) {
      capacity *= 2;
      dl->files = realloc(dl->files, capacity * sizeof(char *));
      if (dl->files == NULL) {
        perror("realloc");
        closedir(fd);
        return -1;
      }
    }

    dl->files[dl->count] = strdup(entry->d_name);
    if (dl->files[dl->count] == NULL) {
      perror("strdup");
      closedir(fd);
      return -1;
    }

    dl->count++;
  }

  closedir(fd);
  return 0;
}

void free_dir_list(struct dir_list *dl) {
  if (dl->files != NULL) {
    for (int i = 0; i < dl->count; i++) {
      free(dl->files[i]);
    }

    free(dl->files);

    dl->files = NULL;
    dl->count = 0;
  }
}
