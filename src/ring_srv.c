// FUNCIONALIDAD DE LA PARTE SERVIDORA
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <pthread.h>
#include "ring.h"
#include "common.h"

enum oper {GET_RPID};

void* request_hdlr(void* arg);

// función para el thread que implementa la funcionalidad de servidor
// debe recibir como argumento el socket de servicio
void *server_thread(void *arg){
    int s =(int)arg;
    int s_conec;
    unsigned int addr_size;
    struct sockaddr_in clnt_addr;
    while(1){
        addr_size=sizeof(clnt_addr);
        if((s_conec=accept(s,(struct sockaddr*)&clnt_addr, &addr_size))<0){
            perror("Error al aceptar conexión\n");
            close(s);
            return 0;
        }
        printf("conectado cliente con ip %s y puerto %u\n",
        inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));
        create_thread(request_hdlr,(void *)(long)s_conec);
        
    }
    close(s);
    return NULL;
}

void* request_hdlr(void* arg){
    int soc = (long)arg;
    int req;

    if(recv(soc,&req,sizeof(int),MSG_WAITALL)!=sizeof(int)){
        perror("error en recv");
        return NULL;
    }
    req=ntohl(req);
    printf("Recibido request con valor %d\n",req);

    switch(req){
        case GET_RPID:
            int pid = htonl(getpid());
            if(write(soc,&pid,sizeof(int))!=sizeof(int)){
                perror("error en el write");
            }        
        break;
    }
    close(soc);
    printf("Conexión con cliente cerrada\n");
    return NULL;
}


