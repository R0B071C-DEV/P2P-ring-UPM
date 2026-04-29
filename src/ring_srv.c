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

enum oper {GET_RPID,NEW_NODE,REM_SUC,SUCSUC};

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
        struct iovec iov[2];
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
            
            //printf("escribe succ port: %u\n",ntohs(port));
            port=htons(port);

            iov[0].iov_base=&ip;
            iov[0].iov_len=sizeof(ip);
            iov[1].iov_base=&port;
            iov[1].iov_len=sizeof(port);
            writev(soc,iov,2);
            
            //TODO guardar nuevo sucesor (IP y PORT)
            self.successor_ip=clnt_addr.sin_addr.s_addr;
            self.sucessor_port=s_port;
        break;

        case REM_SUC:
            //revisa cuál es su sucesor
            unsigned int suc_ip;
            unsigned short suc_port;
            ring_successor(&suc_ip,&suc_port);
            //le envía su suc_ip y suc_port
            suc_ip = htonl(suc_ip);
            suc_port = ntohs(suc_port);
            
            iov[0].iov_base = &suc_ip;
            iov[0].iov_len = sizeof(suc_ip);
            iov[1].iov_base = &suc_port;
            iov[1].iov_len = sizeof(suc_port);

            writev(soc,iov,2);

        break;

        case SUCSUC:
            unsigned int suc_suc_ip;
            unsigned short suc_suc_port;
            int s_temp, req;
            //hacer peticion de remote_suc
            req = htonl(REM_SUC);
            s_temp = create_socket_cln(self.successor_ip,self.sucessor_port);
            write(s_temp,&req,sizeof(req));

            recv(s_temp,&suc_suc_ip,sizeof(unsigned int),MSG_WAITALL);
            recv(s_temp,&suc_suc_port,sizeof(unsigned short),MSG_WAITALL);

            iov[0].iov_base = &suc_suc_ip;
            iov[0].iov_len = sizeof(suc_suc_ip);
            iov[1].iov_base = &suc_suc_port;
            iov[1].iov_len = sizeof(suc_suc_port);

            writev(soc,iov,2);
            
        break;
    }
    close(soc);
    printf("Conexión con cliente cerrada\n");
    return NULL;
}


