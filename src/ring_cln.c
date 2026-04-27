// FUNCIONALIDAD DE LA PARTE CLIENTE
#include <sys/mman.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include "ring.h"
#include "common.h"

enum oper {GET_RPID};

static int is_initialized(void);
static int initialize(void);

typedef struct ring_cln
{
    const char *myshrd_dir;
    unsigned int mylocal_ip;
    unsigned int myremote_ip;
    unsigned short myremote_port;
    unsigned short myalloc_port;
    int service_s;
}ring_cln;

ring_cln self;

// inicia el nodo añadiéndolo a la red P2P si ya está creada;
// los puertos e IPs deben estar en formato de red;
// debe devolver en el último parámetro el puerto reservado en formato red;
// retorna 0 si OK y -1 si error
int ring_init(const char *shrd_dir, unsigned int local_ip, unsigned int remote_ip, unsigned short remote_port, unsigned short *alloc_port) {
    int s;
    if (initialize()) return -1; // ya está inicializada
    if((s=create_socket_srv(alloc_port)) < 0) return -1;
    self.myshrd_dir=shrd_dir;
    self.mylocal_ip=local_ip;
    self.myremote_ip=remote_ip;
    self.myremote_port=remote_port;
    self.myalloc_port=*alloc_port;
    self.service_s=s;
    //printf("Iniciando server thread con s:%d\n",s);
    create_thread(server_thread,(void*)s);
    return 0;
}
// función local que devuelve la IP y el puerto del nodo;
// retorna 0 si OK y -1 si error
int ring_self(unsigned int *ip, unsigned short *port) {
    if (!is_initialized()) return -1; // no está inicializada
    *ip=self.mylocal_ip;
    *port=self.myalloc_port;
    return 0;
}
// devuelve el PID del nodo remoto especificado o -1 si error
int ring_remote_pid(unsigned int remote_ip, unsigned short remote_port) {
    if (!is_initialized()) return -1; // no está inicializada
    
    int s, req, resp;
    s=create_socket_cln(remote_ip,remote_port);
    req = htonl(GET_RPID);
    printf("Enviada petición ring_remote_pid\nEsperando respuesta...\n");
    if(write(s,&req,sizeof(int))!=sizeof(int)){
        perror("error en write [client]");
    }
    if(recv(s,&resp,sizeof(int),MSG_WAITALL)!=sizeof(int)){
        perror("error en recv");
        return -1;
    }
    resp=ntohl(resp);
    printf("PID: %d\n",resp);
    return 0;
}
// función local que devuelve la IP y el puerto del nodo sucesor;
// retorna 0 si OK y -1 si error
int ring_successor(unsigned int *ip, unsigned short *port) {
    if (!is_initialized()) return -1; // no está inicializada
    return 0;
}
// devuelve la IP y el puerto del nodo sucesor del especificado;
// retorna 0 si OK y -1 si error
int ring_remote_successor(unsigned int remote_ip, unsigned short remote_port, unsigned int *suc_ip, unsigned short *suc_port) {
    if (!is_initialized()) return -1; // no está inicializada
    return 0;
}
// devuelve la IP y el puerto del nodo sucesor del sucesor del especificado;
// retorna 0 si OK y -1 si error
int ring_remote_successor_successor(unsigned int remote_ip, unsigned short remote_port, unsigned int *suc_suc_ip, unsigned short *suc_suc_port) {
    if (!is_initialized()) return -1; // no está inicializada
    return 0;
}
// descarga el fichero del nodo especificado;
// retorna el tamaño del fichero si OK y -1 en caso de error
int ring_download(unsigned int remote_ip, unsigned short remote_port, const char *filename) {
    if (!is_initialized()) return -1; // no está inicializada
    return 0;
}
// busca el fichero en el anillo dando un número máximo de saltos y devolviendo
// la IP y el puerto del nodo que lo contiene;
// retorna 0 si OK y -1 si error
int ring_lookup(const char *filename, int hops, unsigned int *ip, unsigned short *port) {
    if (!is_initialized()) return -1; // no está inicializada
    return 0;
}
// busca y descarga el fichero del nodo encontrado en el anillo que lo contiene;
// retorna el tamaño del fichero si OK y -1 en caso de error;
// ESTA FUNCIÓN YA ESTA COMPLETADA
int ring_get_file(const char *filename, int hops) {
    if (!is_initialized()) return -1; // no está inicializada
    unsigned int ip, ip_local;
    unsigned short port, port_local;
    int res = ring_lookup(filename, hops, &ip, &port);
    ring_self(&ip_local, &port_local);
    // realiza la descarga si encontrado en un nodo que no es el local
    if ((res!=-1) && ((ip!=ip_local) || (port!=port_local)))
        res = ring_download(ip, port, filename);
    return res;
}

// funciones auxiliares
static int initialized;

static int initialize(void) {
    return initialized?1:(initialized=1,0);
}
static int is_initialized(void) {
    return initialized;
}
