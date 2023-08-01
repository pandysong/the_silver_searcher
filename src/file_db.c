#include <stddef.h>
#include "file_db.h"
#include "util.h"

static void alloc_enough_paths(char **paths[], char **base_paths[], size_t *paths_cap, size_t *paths_count)
{
#define BLOCK_SIZE 2048
    if (NULL == paths)
    {
        *paths = ag_malloc(sizeof(char*)*BLOCK_SIZE);
        *base_paths = ag_malloc(sizeof(char*)*BLOCK_SIZE);
        *paths_cap = BLOCK_SIZE; //include trailing '\0'
        return;
    }

    if (*paths_count <= *paths_cap -1)
    {
        *paths = ag_realloc(*paths, sizeof(char*)*(*paths_cap + BLOCK_SIZE));
        *base_paths = ag_realloc(*base_paths, sizeof(char*)*(*paths_cap + BLOCK_SIZE));
        *paths_cap += BLOCK_SIZE; //include trailing '\0'
    }
}
/* load the file pathes to `paths`*/
void load_list_of_files(char **paths[], char **base_paths[], const char *db_path, size_t *paths_count) {
    FILE *fp = NULL;
    size_t paths_cap = 0;
    *paths_count = 0;
    log_debug("database file %s.", db_path);
    fp = fopen(db_path, "r");
    if (fp == NULL) {
        log_debug("ignore file database %s: not readable", db_path);
        return;
    }
    log_debug("Loading database file %s.", db_path);

    char *line = NULL;
    ssize_t line_len = 0;
    size_t line_cap = 0;

    char *base_path = NULL;
#ifdef PATH_MAX
    char *tmp = NULL;
#endif

    // todo: is getline thread safe?
    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        if (line_len == 0 || line[0] == '\n' || line[0] == '#') {
            continue;
        }
        if (line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0'; /* kill the \n */
        }
        alloc_enough_paths(paths, base_paths, &paths_cap, paths_count);
        (*paths)[*paths_count] = ag_strdup(line);

#ifdef PATH_MAX
            tmp = ag_malloc(PATH_MAX);
            base_path = realpath(line, tmp);
#else
            base_path = realpath(line, NULL);
#endif
        (*base_paths)[*paths_count] = base_path;
        (*paths_count) ++;
        log_debug("adding path: %s, base_path %s\n", (*paths)[*paths_count], base_path);
    }
    (*paths)[*paths_count] = NULL;

    free(line);
    fclose(fp);
}


