// EJEMPLO DE USO DE SENDFILE PARA COPIAR UN FICHERO
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

int main(int argc, char *argv[]) {
    if (argc!=3) {
        fprintf(stderr, "Uso: %s fich_orig fich_copia\n", argv[0]);
        return -1;
    }
    int fd_orig, fd_copia;
    
    if ((fd_orig = open(argv[1], O_RDONLY)) < 0) {
       perror("open"); return -1;
    }
    struct stat st;
    if (fstat(fd_orig, &st) < 0) { 
       perror("stat"); close(fd_orig); return -1;
    }
    if ((fd_copia = creat(argv[2], 0666)) < 0) {
       perror("creat"); close(fd_orig); return -1;
    }
    sendfile(fd_copia, fd_orig, NULL, st.st_size);
    close(fd_orig);
    close(fd_copia);
    return 0;
}
