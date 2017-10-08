
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

#include "pdns-logger.h"
#include "dnsmessage.pb-c.h"

#define MAX_BUFFER_SIZE 1024

static void *socket_thread_exec(void *data) {
    int flags;
    int *psocket = (int*) data;
    int socket = *psocket;
    char buffer[MAX_BUFFER_SIZE];

    /* Set socket non-blocking */
    flags = fcntl (socket, F_GETFL, 0);
    if ( flags == -1 ) {
        return PDNS_NO;
    }

    flags |= !O_NONBLOCK;
    if ( fcntl (socket, F_SETFL, flags) == -1 ) {
        return PDNS_NO;
    }

    while ( 1 ) {
        int nbytes;

        nbytes = read (socket, buffer, sizeof(buffer));
        if ( nbytes < 0 ) {
            /* Error */
            break;
        }
        else if ( nbytes == 0 ) {
            /* EOF */
            break;
        }
    }

    fprintf(stderr, "Disconnecting client\n");

    return NULL;
}

static pdns_status_t socket_start_thread(int socket) {
    pthread_t thread;

    pthread_create(&thread, NULL, &socket_thread_exec, &socket);

    return PDNS_OK;
}

static pdns_status_t socket_loop(globals_t *conf) {
    int i;
    struct sockaddr_in client_sa;
    fd_set active_fd_set, read_fd_set;

    FD_ZERO (&active_fd_set);
    FD_SET (conf->socket, &active_fd_set);

    while ( conf->running ) {
        read_fd_set = active_fd_set;

        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            return PDNS_NO;
        }

        usleep(1000000 / 10); /* Let's slow things down ... */

        for (i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET (i, &read_fd_set)) {
                if (i == conf->socket) {
                    /* Connection request on original socket. */
                    int new;
                    size_t size;

                    size = sizeof (client_sa);
                    new = accept(conf->socket, (struct sockaddr *) &client_sa, (socklen_t *) &size);

                    if ( new < 0 ) {
                        // TODO ERROR;
                    }
                    else {
                        fprintf (stderr, "Connection from host %s, port %d.\n", inet_ntoa (client_sa.sin_addr), ntohs(client_sa.sin_port));
                        socket_start_thread(new);
                    }
                }
                else {
                    /* Should never happen */
                    assert(0);
                }
            }
        }
    }

    return PDNS_OK;
}

static pdns_status_t socket_start(globals_t *conf) {
    int flags;
    struct sockaddr_in sa;

    conf->socket = socket(AF_INET , SOCK_STREAM , 0);
    if ( conf->socket == -1 ) {
        return PDNS_NO;
    }

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons( conf->bind_port );

    flags = 1;
    if (setsockopt(conf->socket, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags)) < 0) {
        fprintf(stderr, "Cannot set SO_REUSEADDR\n");
    }

    if( bind(conf->socket,(struct sockaddr *)&sa , sizeof(sa)) < 0) {
        fprintf(stderr, "Bind failed. Error\n");
        return PDNS_NO;
    }

    flags = fcntl (conf->socket, F_GETFL, 0);
    if ( flags == -1 ) {
        return PDNS_NO;
    }

    flags |= O_NONBLOCK;
    if ( fcntl (conf->socket, F_SETFL, flags) == -1 ) {
        return PDNS_NO;
    }

    if ( listen(conf->socket, SOMAXCONN) < 0 ) {
        return PDNS_NO;
    }

    return PDNS_OK;
}

static pdns_status_t socket_close(globals_t *conf) {
    close(conf->socket);
    return PDNS_OK;
}

pdns_status_t pdns_socket_run(globals_t *globals) {
    if ( socket_start(globals) == PDNS_OK ) {
        socket_loop(globals);
        return socket_close(globals);
    }

    return PDNS_NO;
}
