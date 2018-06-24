#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
// Memomia compartida y cola de mensajes
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
// Sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "server.h"
int timeout = (3 * 60 * 1000);


int main(int argc, char **argv)
{
    if(argc<2)
    {
        printf("%s [puerto]\n",argv[0]);
        return 1;
    }
    int len ;

    int desc_ready, end_server = 0, compress_array = 0;
    int close_conn;
    int puerto, process_pid[4], rc, on = 1;
    int listen_sd = -1, new_sd = -1;
    int current_size = 0, i, j;
    pid_t process;
    socklen_t longCliente;
    struct sockaddr_in servidor, cliente;
    char buffer[200];
    puerto = atoi(argv[1]);

    key_t key = ftok(".", 444);
    int id_mem;
    void *pto_mem;
    shmem_data *slave;

    /*
    * Memoria compartida
    */
    if ((id_mem = shmget(key, sizeof(shmem_data), IPC_CREAT|0666)) < 0)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    if ((pto_mem = (void *) shmat(id_mem, NULL, 0)) == (int *) -1)
    {
        perror("shmmat");
        exit(EXIT_FAILURE);
    }

    slave = (shmem_data *) pto_mem;

    /*
    * Asignacion -> socket
    */
    if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() failed");
        exit(-1);
    }

    /*
    * Socket descriptor reutilizable
    */
    if ((rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                                    (char *)&on, sizeof(on))) < 0)
    {
        perror("setsockopt() failed");
        close(listen_sd);
        exit(-1);
    }

    /*
    * Socket no bloqueante
    */
    if((rc = ioctl(listen_sd, FIONBIO, (char *)&on)) < 0)
    {
        perror("ioctl() failed");
        close(listen_sd);
        exit(-1);
    }

    /*
    * Asignacion -> socket a puerto
    */
    bzero((char *)&servidor, sizeof(servidor));
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(puerto);
    servidor.sin_addr.s_addr = INADDR_ANY; //Macro -> Propia Direccion


    if(bind(listen_sd, (struct sockaddr *)&servidor,
                    sizeof(servidor)) < 0)
    {
        perror("bind() failed");
        close(listen_sd);
        exit(-1);
    }

    /*
    * Eschuchar
    */
    if ((rc = listen(listen_sd, 30)) < 0)
    {
        perror("listen() failed");
        close(listen_sd);
        exit(-1);
    }

    /*
    * Procesos encargados de: escuchar slave,
    *escuchar al cliente, proceso encargado de
    *los hijos que terminan
    */
    /*
    * Inicializar estructura pollfd
    */

    slave->nfds = 1;
    //slave->nfds_dsp = 0;
    for (i = 0; i<200 ; i++)
    {
        slave->dsp[i] = -1;
        slave->fds[i].fd = -1;
    }

    // memset(slave->fds, 0 , sizeof(slave->fds));
    slave->fds[0].fd = listen_sd; //servidor
    slave->fds[0].events = POLLIN;


    /*
    *  Poll conexion de esclavos
    */

    process = fork();

    if(!process){
        // hijo

    }else{
        // padre
        close(listen_sd);
    }

    do
    {
        if(!process)
        {
            if (((rc = poll(slave->fds, slave->nfds, timeout)) < 0))
            {
                perror("    poll() failed");
                continue;
            }
            if (rc == 0)
            {
                continue;
            }
            /*
            * Registrando esclavos
            */
            current_size = slave->nfds;
            for (i = 0; i < current_size; i++)
            {
                if(slave->fds[i].revents == 0)
                    continue;

                if(slave->fds[i].revents != POLLIN)
                {
                    printf("Error revents  hijo= %d\n", slave->fds[i].revents);
                    break;
                }

                if (slave->fds[i].fd == listen_sd)
                {
                    printf("nuevo!\n");
                    do
                    {
                        if((new_sd = accept(listen_sd, NULL, NULL)) < 0)
                        {
                            if (errno != EWOULDBLOCK)
                                perror("    accept() failed");
                            break;
                        }
                        slave->fds[slave->nfds].fd = new_sd; //tener cuidado cuando se desconecta un esclavo
                        slave->fds[slave->nfds].events = POLLIN;
                        slave->nfds++;




                        sleep(2);
                        int fd = slave->nfds - 1;
                        rc = recv(slave->fds[fd].fd, buffer, sizeof(buffer), 0);

                        if (rc < 0)
                        {
                            if (errno != EWOULDBLOCK)
                            {
                                perror("    recv() failed");
                                close_conn = 1;
                            }
                            break;
                        }

                        if (rc == 0)
                        {
                            printf("    Connection closed\n");
                            close_conn = 1;
                            break;
                        }

                        len = rc;
                        printf("    %d bytes received\n", len);
                        printf(" Buffer:    %s\n", buffer);

                        rc = send(slave->fds[fd].fd, "hola", 16, 0);
                        if (rc < 0)
                        {
                            perror("    send() failed");
                            close_conn = 1;
                            break;
                        }











                    }while (new_sd != -1);







                }
            }
        }else
        {

            int fd;
            while(1)
            {

                if(slave->nfds >= 1)
                {
                    poll(slave->fds, slave->nfds, timeout);

                    close_conn = 0;

                    printf("Select a file descriptor: \n");
                    fflush(stdin);
                    scanf("%d", &fd);
                    printf("    fd: %d Descriptor selected:  %d revents: %d\n", fd, slave->fds[fd].fd, slave->fds[fd].revents);

                    if(slave->fds[fd].fd == -1){ // fd >= nfds
                        printf("fd is not in use\n");
                        continue;
                    }

                    if(fd == 0){
                        printf("fd is the listener\n");
                        continue;
                    }

                    if(slave->fds[fd].revents == 0)
                    {
                        printf("Nothing to read. Augus y Sara.\n");
                        continue;
                    }
                    if(slave->fds[fd].revents != POLLIN)
                    {
                        printf("Error revents  padre= %d\n", slave->fds[fd].revents);
                        close_conn = 1;
                    }else{
                        // we know that fds[fd].revents == POLLIN

                        printf("    Descriptor %d is readable\n", slave->fds[fd].fd);

                        rc = recv(slave->fds[fd].fd, buffer, sizeof(buffer), 0);

                        if (rc < 0)
                        {
                            if (errno != EWOULDBLOCK)
                            {
                                perror("    recv() failed");
                                close_conn = 1;
                            }
                            break;
                        }

                        if (rc == 0)
                        {
                            printf("    Connection closed\n");
                            close_conn = 1;
                            break;
                        }

                        len = rc;
                        printf("    %d bytes received\n", len);
                        printf(" Buffer:    %s\n", buffer);

                        rc = send(slave->fds[fd].fd, "hola", 16, 0);
                        if (rc < 0)
                        {
                            perror("    send() failed");
                            close_conn = 1;
                            break;
                        }
                    }





                    if (close_conn)
                    {
                        close(slave->fds[fd].fd);
                        //slave->fds[fd].fd = -1;
                        compress_array = 1;
                    }
                    if (compress_array)
                    {
                        compress_array = 0;
                        for (i = 0; i < slave->nfds; i++)
                        {
                            if (slave->fds[i].fd == -1)
                            {
                                for(j = i; j < slave->nfds; j++)
                                {
                                    slave->fds[j].fd = slave->fds[j+1].fd;
                                }
                                i--;
                                slave->nfds--;
                                slave->fds[slave->nfds].fd = -1;
                            }
                        }
                    }
                }
            }
        }
    }while(end_server == 0); /* End of serving running.      */

    for (i = 0; i < slave->nfds; i++)
    {
        if(slave->fds[i].fd >= 0)
            close(slave->fds[i].fd);
    }

    return 0;
}