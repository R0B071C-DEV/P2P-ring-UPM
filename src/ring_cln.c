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

enum oper {GET_RPID,NEW_NODE,REM_SUC};

static int is_initialized(void);
static int initialize(void);

typedef struct ring_cln
{
    const char *myshrd_dir;
    unsigned int mylocal_ip;
    unsigned int successor_ip;
    unsigned short successor_port;
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
    self.myalloc_port=*alloc_port;
    self.service_s=s;

    create_thread(server_thread,(void*)s);

    if(remote_ip!=0){//Si se pide unir un nodo
        //envía petición
        int soc,req; 
        soc = create_socket_cln(remote_ip,remote_port);
        req = htonl(NEW_NODE);
        if(write(soc,&req,sizeof(int))!=sizeof(int)){
            perror("error en write [write de req NEW_NODE]");
        }
        //envía su service_port
        unsigned short s_port= *alloc_port;
        if(write(soc,&s_port,sizeof(unsigned short))!=sizeof(unsigned short)){
            perror("error en write [write de srv_sockt]");
        }
        printf("enviado service port: %d\n",ntohs(s_port));

        //espera respuesta
        unsigned int nip;
        unsigned short nport; 
        if(recv(soc,&nip,sizeof(unsigned int),MSG_WAITALL)!=sizeof(unsigned int)){
            perror("error esperando ip sucesor");
        }
        nip=ntohl(nip);

        if(recv(soc,&nport,sizeof(unsigned short),MSG_WAITALL)!=sizeof(unsigned short)){
            perror("error esperando port sucesor");
        }
        nport=ntohs(nport);

        self.successor_ip=nip;
        self.successor_port=nport;
    }
    else{//de lo contrario, es él mismo
        self.successor_ip=local_ip;
        self.successor_port=*alloc_port;
    }
    
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
    return resp;
}
// función local que devuelve la IP y el puerto del nodo sucesor;
// retorna 0 si OK y -1 si error
int ring_successor(unsigned int *ip, unsigned short *port) {
    if (!is_initialized()) return -1; // no está inicializada
    *ip=self.successor_ip;
    *port=self.successor_port;
    return 0;
}
// devuelve la IP y el puerto del nodo sucesor del especificado;
// retorna 0 si OK y -1 si error
int ring_remote_successor(unsigned int remote_ip, unsigned short remote_port, unsigned int *suc_ip, unsigned short *suc_port) {
    if (!is_initialized()) return -1; // no está inicializada
    int s;
    s = create_socket_cln(remote_ip,remote_port);
    //envia peticion para revisar el nodo sucesor de un nodo remoto
    int req = htonl(REM_SUC);
    if(write(s,&req,sizeof(unsigned int))!=sizeof(unsigned int)){
        perror("error al mandar pet. REM_SUC");
        return -1;
    }

    if(recv(s,suc_ip,sizeof(unsigned int),MSG_WAITALL)!=sizeof(unsigned int)){
        perror("error al recibir remote_ip");
        return -1;
    }
    *suc_ip = ntohl(*suc_ip);

    if(recv(s,suc_port,sizeof(unsigned short),MSG_WAITALL)!=sizeof(unsigned short)){
        perror("error al recibir remote_port");
        return -1;
    }
    *suc_port = ntohs(*suc_port);

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
