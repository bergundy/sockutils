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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "sockutils.h"

int unblock( int sock, const char *error_str )
{
    int flags;

    if( ( flags = fcntl(sock, F_GETFL, NULL) ) < 0) { 
        sprintf(error_str, "Error fcntl(..., F_GETFL) (%s)", strerror(errno)); 
        return 0;
    } 
    if( fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) { 
        sprintf(error_str, "Error fcntl(..., F_SETFL) (%s)", strerror(errno)); 
        return 0;
    } 
    return 1;
}

struct addrinfo *prepare_addrinfo_tcp(const char *addr,
                                      const char *port,
                                      char *error_str)
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
        sprintf(error_str, "getaddrinfo() failed %s", gai_strerror(gai));
        return NULL;
    }

    return res;
}

int tcp_server(const char *bind_addr, const char *port, int flags, size_t backlog, const char *error_str)
{
    struct addrinfo *res;
    int sockfd, on;
    on = 1;

    if ( ( res = prepare_addrinfo_tcp(bind_addr, port, error_str) ) == NULL )
    {
        return SOCKERR;
    }

    /* socket creation */
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        sprintf(error_str, "failed to create socket");
        return SOCKERR;
    }

    /* reuse socket */
    if ( flags & REUSE )
        if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                  &on, sizeof(on)) != 0) {
            sprintf(error_str, "failed to set sockopt to reuse");
            close(sockfd);
            return SOCKERR; 
        }

    /* bind */
    if ( bind(sockfd, res->ai_addr, res->ai_addrlen) < 0 ) {
        sprintf(error_str, "failed to bind to port %s", port);
        close(sockfd);
        return SOCKERR; 
    }

    /* listen */
    if ( listen(sockfd, backlog) < 0 ) {
        close(sockfd);
        return SOCKERR; 
    }

    /* unblock socket */
    if ( flags & NONBLK )
        if ( !unblock(sockfd, error_str) ) {
            close(sockfd);
            return SOCKERR;
        }

    return sockfd;
}

int tcp_client(const char *server_addr, const char *port, int flags, const char *error_str)
{
    struct addrinfo *servinfo, *p;
    int sockfd;
    
    if ( ( servinfo = prepare_addrinfo_tcp(server_addr, port, error_str) ) == NULL )
    {
        return SOCKERR;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {

        /* socket creation */
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == SOCKERR) {
            sprintf(error_str, "client: socket :: %s", strerror(errno));
            continue;
        }

        /* connect */
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == SOCKERR) {
            close(sockfd);
            sprintf(error_str, "client: connect :: %s", strerror(errno));
            continue;
        }
        /* unblock */
        if ( flags & NONBLK )
            if ( !unblock(sockfd, error_str) ) {
                close(sockfd);
                continue;
            }

        break;
    }

    if (p == NULL) {
        sprintf(error_str, "client: failed to connect");
        sockfd = SOCKERR;
    }

    freeaddrinfo(servinfo); // all done with this structure
    return sockfd;
}
