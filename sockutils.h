/*
 * =====================================================================================
 *
 *       Filename:  sockutils.h
 *
 *    Description:  header file for helper functions for sockets
 *
 *        Version:  1.0
 *        Created:  05/20/2010 04:46:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Roey Berman, royb@walla.net.il
 *        Company:  Walla!
 *
 * =====================================================================================
 */
#ifndef SOCKUTILS_H
#define SOCKUTILS_H 

#include <sys/types.h>

#define SOCKERR -1
#define REUSE    1
#define NONBLK   2

int unblock(int , char *);
struct addrinfo *prepare_addrinfo_tcp(const char *, const char *, char *);
int tcp_server(const char *, const char *, int, size_t, const char *);
int tcp_client(const char *, const char *, int, const char *);

#endif /* SOCKUTILS_H */
