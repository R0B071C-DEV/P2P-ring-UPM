// DECLARACIÓN DE LA FUNCIONALIDAD COMÚN PARA CLIENTES Y SERVIDORES
#ifndef _COMMON_H
#define _COMMON_H        1

// crea un thread detached
int create_thread(void *(*func)(void *), void *arg);

// Inicializa el socket, le asigna un puerto seleccionado por el SO
// y lo prepara para aceptar conexiones. Recibe un parámetro de salida
// donde devuelve el puerto asignado en formato de red.
int create_socket_srv(unsigned short *port);

// crea socket y se conecta al servidor traduciendo previamente su nombre
int create_socket_cln(const char *host, const char *port);

#endif // _COMMON_H
