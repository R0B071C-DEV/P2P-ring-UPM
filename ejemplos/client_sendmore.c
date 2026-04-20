// EJEMPLO DE CLIENTE QUE ENVÍA UN ENTERO Y UN STRING.
// RECIBIENDO EN UN ENTERO COMO RESPUESTA.
// PARA ENVIAR EL STRING, AL SER DE TAMAÑO VARIABLE, TRANSMITE ANTES SU TAMAÑO.
// USA MSG_MORE PARA EVITAR LA FRAGMENTACIÓN DE LOS PAQUETES DE RED.
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

int main(int argc, char *argv[]) {
    int s;
    if (argc!=5) {
        fprintf(stderr, "Usage: %s server_host server_port entero string\n", argv[0]);
        return -1;
    }
    // inicializa el socket y se conecta al servidor
    if ((s=create_socket_cln(argv[1], argv[2]))<0) return -1;

    // datos a enviar: un entero y un string
    int entero = atoi(argv[3]);
    char *string = argv[4];

    // envío del entero convertido a formato de red
    // indicando que se van a enviar más mensajes (MSG_MORE)
    int entero_net = htonl(entero);
    if (send(s, &entero_net, sizeof(int), MSG_MORE)<0) {
        perror("error en send"); close(s); return -1;
    }

    // envío de la longitud del string
    // indicando que se van a enviar más mensajes (MSG_MORE)
    int longitud_str = strlen(string); // no incluye el carácter nulo
    int longitud_str_net = htonl(longitud_str);
    if (send(s, &longitud_str_net, sizeof(int), MSG_MORE)<0) {
        perror("error en send"); close(s); return -1;
    }
    // envío del string sin ndicar que se van a enviar más mensajes
    if (send(s, string, longitud_str, 0)<0) {
        perror("error en send"); close(s); return -1;
    }

    // recepción de un entero
    if (recv(s, &entero, sizeof(int), MSG_WAITALL) != sizeof(int)) {
        perror("error en recv"); close(s); return -1;
    }
    printf("Recibido %d\n", ntohl(entero)); // a formato de host
    close(s);
    return 0;
}
