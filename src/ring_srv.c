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

// función para el thread que implementa la funcionalidad de servidor
// debe recibir como argumento el socket de servicio
void *server_thread(void *arg){
    int s =(int)arg;
    //printf("AAAAAAAAA s recibido:%d\n",s);
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
        
    }
    close(s);
    return NULL;
}
