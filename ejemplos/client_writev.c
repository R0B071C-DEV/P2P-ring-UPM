// EJEMPLO DE CLIENTE QUE ENVÍA UN ENTERO Y UN STRING
// RECIBIENDO EN UN ENTERO COMO RESPUESTA.
// PARA ENVIAR EL STRING, AL SER DE TAMAÑO VARIABLE, TRANSMITE ANTES SU TAMAÑO.
// USA writev PARA ENVIARLO TODO CON UNA SOLA OPERACIÓN EVITANDO
// LA FRAGMENTACIÓN DE LOS PAQUETES DE RED.
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include "common.h"

int main(int argc, char *argv[]) {
    int s;
    if (argc!=5) {
        fprintf(stderr, "Usage: %s server_host server_port entero string\n", argv[0]);
        return -1;
    }
    // inicializa el socket y se conecta al servidor
    if ((s=create_socket_cln(argv[1], argv[2]))<0) return -1;

    // datos a enviar
    int entero = atoi(argv[3]);
    char *string = argv[4];

    struct iovec iov[3]; // hay que enviar 3 elementos
    int nelem = 0, tot = 0;

    // preparo el envío del entero convertido a formato de red
    int entero_net = htonl(entero);
    iov[nelem].iov_base=&entero_net;
    tot+=iov[nelem++].iov_len=sizeof(int);

    // preparo el envío de la longitud del string
    int longitud_str = strlen(string); // no incluye el carácter nulo
    int longitud_str_net = htonl(longitud_str);
    iov[nelem].iov_base=&longitud_str_net;
    tot+=iov[nelem++].iov_len=sizeof(int);

    // preparo el envío del string
    iov[nelem].iov_base=string; // no se usa & porque ya es un puntero
    tot+=iov[nelem++].iov_len=longitud_str;

    if (writev(s, iov, nelem)!=tot) {
        perror("error en writev"); close(s); return -1;
    }
 
    // recepción de un entero
    if (recv(s, &entero, sizeof(int), MSG_WAITALL) != sizeof(int)) {
        perror("error en recv"); close(s); return -1;
    }
    printf("Recibido %d\n", ntohl(entero)); // a formato de host
    close(s);
    close(s); // cierra el socket
    return 0;
}
