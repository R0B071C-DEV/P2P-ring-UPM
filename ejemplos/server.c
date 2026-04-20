// EJEMPLO DE SERVIDOR MULTITHREAD CUYOS CLIENTES ENVÍAN UNA PETICIÓN POR CONEXIÓN.
// RECIBE UN ENTERO Y UN STRING PRECEDIDO POR SU LONGITUD Y ENVÍA COMO
// RESPUESTA UN ENTERO.
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <pthread.h>
#include "common.h"

// función del thread: solo recibe una petición por conexión
void *request_handler(void *arg){
    int soc = (long)arg; // argumento recibido por valor

    int entero;
    int longitud;
    char *string;

    // recibe el entero
    if (recv(soc, &entero, sizeof(int), MSG_WAITALL)!=sizeof(int)) {
        perror("error en recv");
        return NULL;
    }
    entero = ntohl(entero);
    printf("Recibido entero: %d\n", entero);

    // recibe la longitud del string
    if (recv(soc, &longitud, sizeof(int), MSG_WAITALL)!=sizeof(int)) {
        perror("error en recv");
        return NULL;
    }
    longitud = ntohl(longitud);
    string = malloc(longitud+1); // +1 para el carácter nulo
				 
    // recibe el string
    if (recv(soc, string, longitud, MSG_WAITALL)!=longitud) {
        perror("error en recv");
        return NULL;
    }
    string[longitud]='\0';      // añadimos el carácter nulo
    printf("Recibido string: %s\n", string);

    // procesado sin sentido de la petición
    int respuesta = entero + strlen(string);
    printf("Enviado: %d\n", respuesta);
    respuesta = htonl(respuesta); // a formato de red
    if (write(soc, &respuesta, sizeof(int))!=sizeof(int)) {
        perror("error en write");
    }
    close(soc);
    printf("conexión del cliente cerrada\n");
    return NULL;
}
int main(int argc, char *argv[]) {
    int s, s_conec;
    unsigned int addr_size;
    unsigned short port;
    struct sockaddr_in clnt_addr;

    // inicializa el socket y lo prepara para aceptar conexiones
    if ((s=create_socket_srv(&port)) < 0) return 1;
    printf("Reservado el puerto %d\n", ntohs(port));

    while(1) {
        addr_size=sizeof(clnt_addr);
        // acepta la conexión
        if ((s_conec=accept(s, (struct sockaddr *)&clnt_addr, &addr_size))<0){
            perror("error en accept"); close(s); return 0;
        }
        printf("conectado cliente con ip %s y puerto %u\n",
                inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
        // crea el thread de servicio pasándole el argumento por valor
        create_thread(request_handler, (void *)(long)s_conec);
    }
    close(s); // cierra el socket general
    return 0;
}
