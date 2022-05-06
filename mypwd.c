#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_PATH_MAX 2048


typedef struct Data {
    char *path;  /* path that will be printed */
    int index;  /* index in path array */

    /* data for writePath(), this stuff is here so that it doesn't have to be
     * allocated on the stack each call */
    struct dirent *myDirent;
    struct stat childStat;
    struct stat myStat;
    ino_t rootIno;
    dev_t rootDev;
} Data;


Data *initializeData() {
    Data *data = (Data *) malloc(sizeof(Data));
    if (data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* max path length will be PATH_MAX, or 2048 if it is not defined.
     * index will start at the end of the path array */
    data->index = DEFAULT_PATH_MAX - 1;
    #ifdef PATH_MAX
    data->index = PATH_MAX - 1;
    #endif

    data->path = (char *) malloc(sizeof(char) * (data->index + 1));
    if (data->path == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    data->path[data->index] = '\0';  /* path will be null-terminated */
    data->index--;

    /* store root i-node and device number, writePath() will stop when it gets
     * to root */
    if (stat("/", &data->myStat) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    data->rootIno = data->myStat.st_ino;
    data->rootDev = data->myStat.st_dev;

    return data;
}


void addToPath(char *name, Data *data) {
    int i;
    int len = strlen(name);

    data->index -= len + 1;  /* room for the name with a slash in front */
    if (data->index < 0) {
        fprintf(stderr, "path too long\n");
        exit(EXIT_FAILURE);
    }

    /* copy name into array with '/' in front */
    data->path[data->index] = '/';
    for (i = 0; i < len; i++) {
        data->path[data->index + i + 1] = name[i];
    }
}


void writePath(Data *data, ino_t ino, dev_t dev) {
    DIR *dir;

    /* go one level up */
    if (chdir("..") == -1) {
        fprintf(stderr, "cannot get current directory\n");
        exit(EXIT_FAILURE);
    }
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    /* stop if we get to root */
    if (stat(".", &data->myStat) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    if (ino == data->rootIno && dev == data->rootDev) {
        return;
    }

    /* find name of previous dir and write it */
    while ( (data->myDirent = readdir(dir)) ) {
        stat(data->myDirent->d_name, &data->childStat);

        if (data->childStat.st_ino == ino && data->childStat.st_dev == dev) {
            addToPath(data->myDirent->d_name, data);
            writePath(data, data->myStat.st_ino, data->myStat.st_dev);
            closedir(dir);
            return;
        }
    }

    fprintf(stderr, "cannot get current directory\n");
    exit(EXIT_FAILURE);
}


int main(void) {
    Data *data = initializeData();

    if (stat(".", &data->myStat) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    writePath(data, data->myStat.st_ino, data->myStat.st_dev);

    printf("%s\n", data->path + data->index);

    return 0;
}

