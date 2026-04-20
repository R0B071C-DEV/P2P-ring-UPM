// EJEMPLO DE USO DE READ Y MMAP PARA COPIAR UN FICHERO
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int  argc, char *argv[]) {
    if (argc!=3) {
        fprintf(stderr, "Uso: %s fich_orig fich_copia\n", argv[0]);
        return -1;
    }
    int fd_orig, fd_copia;
    size_t tam;
    char *p;

    if ((fd_orig = open(argv[1], O_RDONLY)) < 0) {
       perror("open"); return -1;
    }
    struct stat st;
    if (fstat(fd_orig, &st) < 0) {
       perror("stat"); close(fd_orig); return -1;
    }

    fd_copia = open(argv[2], O_CREAT|O_TRUNC|O_RDWR, 0666);
    if (fd_copia < 0) {
       perror("destino"); close(fd_orig); return -1;
    }
    tam=st.st_size;
    /* Establece la longitud del fichero destino */
    if (ftruncate(fd_copia, tam))
    {
        perror("ftruncate"); close(fd_orig); close(fd_copia); return -1;
    }
    /* Se proyecta el fichero destino */
    if ((p=mmap(0, tam, PROT_WRITE, MAP_SHARED, fd_copia, 0))==MAP_FAILED){
        perror("mmap"); close(fd_orig); close(fd_copia); return -1;
    }
    close(fd_copia);
    /* Se lee directamente sobre la región proyectada */
    if (read(fd_orig, p, tam) != tam) {
        perror("read"); close(fd_orig); return -1;
    }

    close(fd_orig);
    munmap(p, tam);
    return(0);
}
