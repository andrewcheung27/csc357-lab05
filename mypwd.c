#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_PATH_MAX 2048


typedef struct Path {
    char *arr;
    int index;

    /* data for writePath() */
    struct dirent *myDirent;
    struct stat childStat;
    struct stat myStat;
    ino_t rootIno;
    dev_t rootDev;
} Path;


Path *createPath() {
    Path *path = (Path *) malloc(sizeof(Path));
    if (path == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    path->index = DEFAULT_PATH_MAX - 1;
    #ifdef PATH_MAX
    path->index = PATH_MAX - 1;
    #endif

    path->arr = (char *) malloc(sizeof(char) * (path->index + 1));
    if (path->arr == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    path->arr[path->index] = '\0';
    path->index--;

    stat("/", &path->myStat);
    path->rootIno = path->myStat.st_ino;
    path->rootDev = path->myStat.st_dev;

    return path;
}


void addToPath(char *name, Path *path) {
    int i;
    int len = strlen(name);

    path->index -= len + 1;  /* room for the name with a slash in front */
    if (path->index < 0) {
        fprintf(stderr, "path too long\n");
        exit(EXIT_FAILURE);
    }

    (path->arr)[path->index] = '/';
    for (i = 0; i < len; i++) {
        (path->arr)[path->index + i + 1] = name[i];
    }
}


void writePath(Path *path, ino_t ino, dev_t dev) {
    DIR *dir;

    /* go one level up */
    chdir("..");
    dir = opendir(".");

    stat(".", &path->myStat);
    if (ino == path->rootIno && dev == path->rootDev) {
        return;
    }

    /* find name of previous dir and write it */
    while ( (path->myDirent = readdir(dir)) ) {
        stat(path->myDirent->d_name, &path->childStat);
        if (path->childStat.st_ino == ino && path->childStat.st_dev == dev) {
            addToPath(path->myDirent->d_name, path);
            writePath(path, path->myStat.st_ino, path->myStat.st_dev);
            closedir(dir);
            return;
        }
    }

    fprintf(stderr, "cannot get current directory");
    exit(EXIT_FAILURE);
}


int main(void) {
    Path *path = createPath();

    stat(".", &path->myStat);
    writePath(path, path->myStat.st_ino, path->myStat.st_dev);

    printf("%s\n", path->arr + path->index);
}

