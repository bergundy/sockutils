/*
 * =====================================================================================
 *
 *       Filename:  sockutils.c
 *
 *    Description:  helper functions for sockets
 *
 *        Version:  1.0
 *        Created:  05/20/2010 04:30:25 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Roey Berman, royb@walla.net.il
 *        Company:  Walla!
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "sockutils.h"

int unblock( int sock, char **errorstr )
{
    int flags;

    if ( ( flags = fcntl(sock, F_GETFL, NULL) ) < 0) { 
        if (errorstr != NULL)
            *errorstr = strdup(strerror(errno));
        return 0;
    } 
    if ( fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) { 
        if (errorstr != NULL)
            *errorstr = strdup(strerror(errno));
        return 0;
    } 
    return 1;
}

struct addrinfo *prepare_addrinfo_tcp(const char *addr,
                                      const char *port,
                                      char **errorstr)
{
    struct addrinfo hints, *res;
    int gai;

    /*  init addrinfo */
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;     // fill in my IP for me

    if ( ( gai = getaddrinfo(addr, port, &hints, &res) ) != 0 ) {
        if (errorstr != NULL)
            *errorstr = strdup(gai_strerror(gai) );
        return NULL;
    }

    return res;
}

int tcp_server(const char *bind_addr, const char *port, int flags, size_t backlog, char **errorstr)
{
    struct addrinfo *res;
    int sockfd, on;
    static const char *bind_error = "failed to bind to port %s";
    on = 1;

    if ( ( res = prepare_addrinfo_tcp(bind_addr, port, errorstr) ) == NULL )
    {
        return SOCKERR;
    }

    /* socket creation */
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        if (errorstr != NULL)
            *errorstr = strdup("failed to create socket");
        return SOCKERR;
    }

    /* reuse socket */
    if ( flags & REUSE )
        if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                  &on, sizeof(on)) != 0) {
            if (errorstr != NULL)
                *errorstr = strdup("failed to set sockopt to reuse");
            close(sockfd);
            return SOCKERR; 
        }

    /* bind */
    if ( bind(sockfd, res->ai_addr, res->ai_addrlen) < 0 ) {
        if (errorstr != NULL)
            if ( ( *errorstr = (char *)malloc( sizeof(char) * ( sizeof(bind_error) - 2 + strlen(port) ) ) ) != NULL )
                sprintf(*errorstr, "failed to bind to port %s", port);
        close(sockfd);
        return SOCKERR; 
    }

    /* listen */
    if ( listen(sockfd, backlog) < 0 ) {
        if (errorstr != NULL)
            *errorstr = strdup("failed to listen to socket");
        close(sockfd);
        return SOCKERR; 
    }

    /* unblock socket */
    if ( flags & NONBLK )
        if ( !unblock(sockfd, errorstr) ) {
            close(sockfd);
            return SOCKERR;
        }

    return sockfd;
}

#define STRERROR2ERRORSTR do {                                  \
    if (errorstr != NULL) {                                     \
        if (error_initialized) {                                \
            free(*errorstr);                                    \
            --error_initialized;                                \
        }                                                       \
        if ( ( *errorstr = strdup(strerror(errno) ) ) != NULL ) \
            ++error_initialized;                                \
    }                                                           \
} while (0)

int tcp_client(char const *server_addr, char const *port, int flags, char **errorstr)
{
    struct addrinfo *servinfo, *p;
    int sockfd;
    int error_initialized = 0;
    
    if ( ( servinfo = prepare_addrinfo_tcp(server_addr, port, errorstr) ) == NULL )
        return SOCKERR;

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {

        /* socket creation */
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == SOCKERR) {
            STRERROR2ERRORSTR;
            continue;
        }

        /* connect */
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == SOCKERR) {
            close(sockfd);
            STRERROR2ERRORSTR;
            continue;
        }
        /* unblock */
        if ( flags & NONBLK )
            if ( !unblock(sockfd, errorstr) ) {
                close(sockfd);
                continue;
            }

        break;
    }

    if (p == NULL) {
        if (errorstr != NULL && !error_initialized)
                *errorstr = strdup("failed to connect");
        sockfd = SOCKERR;
    }

    freeaddrinfo(servinfo); // all done with this structure
    return sockfd;
}
