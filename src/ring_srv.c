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

enum oper {GET_RPID,NEW_NODE,REM_SUC};

typedef struct ring_cln
{
    const char *myshrd_dir;
    unsigned int mylocal_ip;
    unsigned int successor_ip;
    unsigned short sucessor_port;
    unsigned short myalloc_port;
    int service_s;
}ring_cln;

extern ring_cln self;

void* request_hdlr(void* arg);

struct sockaddr_in clnt_addr;
// función para el thread que implementa la funcionalidad de servidor
// debe recibir como argumento el socket de servicio
void *server_thread(void *arg){
    int s =(int)arg;
    int s_conec;
    unsigned int addr_size;
    
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
    //printf("Recibido request con valor %d\n",req);

    switch(req){
        case GET_RPID:
            int pid = htonl(getpid());
            if(write(soc,&pid,sizeof(int))!=sizeof(int)){
                perror("error en el write");
                break;
            }        
        break;

        case NEW_NODE:
            unsigned int ip;
            unsigned short port,s_port;
            ring_successor(&ip,&port);
            //recibe puerto nuevo
            if(recv(soc,&s_port,sizeof(unsigned short),MSG_WAITALL)!=sizeof(unsigned short)){
                perror("err recv [NEW_NODE]");
                break;
            }
            //printf("Recibido service port: %u\n",ntohs(s_port));

            //printf("escribe succ ip: %s\n",inet_ntoa((struct in_addr){ip}));
            ip=htonl(ip);
            if(write(soc,&ip,sizeof(unsigned int))!=sizeof(unsigned int)){
                perror("err write succ ip");
                break;
            }
            
            //printf("escribe succ port: %u\n",ntohs(port));
            port=htons(port);
            if(write(soc,&port,sizeof(unsigned short))!=sizeof(unsigned short)){
                perror("err write succ port");
                break;
            }
            
            //TODO guardar nuevo sucesor (IP y PORT)
            self.successor_ip=clnt_addr.sin_addr.s_addr;
            self.sucessor_port=s_port;
        break;

        case REM_SUC:
            //revisa cuál es su sucesor
            unsigned int suc_ip;
            unsigned short suc_port;
            ring_successor(&suc_ip,&suc_port);
            //le envía su suc_ip
            suc_ip = htonl(suc_ip);
            if(write(soc,&suc_ip,sizeof(unsigned int))!=sizeof(unsigned int)){
                perror("error al escribir ip de sucesor");
                break;
            }
            //envia su suc_port
            suc_port = ntohs(suc_port);
            if(write(soc,&suc_port,sizeof(unsigned short))!=sizeof(unsigned short)){
                perror("error al escribir puerto de sucesor");
                break;
            }

        break;
    }
    close(soc);
    printf("Conexión con cliente cerrada\n");
    return NULL;
}


