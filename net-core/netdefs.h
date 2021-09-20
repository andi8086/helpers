#ifndef __NET_DEFS_H__
#define __NET_DEFS_H__


#if defined(_WIN32) || defined(_WIN64)
#        include <Winsock2.h>
#        include <Ws2tcpip.h> /* InetPton */
#        pragma comment(lib, "ws2_32.lib")

#        define inet_pton(a, b, c) InetPton(a, b, c)
#elif !defined(SOC_AM65XX)
#        include <sys/types.h>
#        include <sys/socket.h>
#        include <netinet/tcp.h>
#        include <arpa/inet.h>
#        include <unistd.h>
#        ifndef SOCKET
typedef int SOCKET;
#        endif
#endif

#if defined(SOC_AM65XX)
#        include <ti/ndk/inc/netmain.h>
#        include <ti/ndk/inc/stkmain.h>

#endif

#ifndef ssize_t
#        define ssize_t int
#endif

#ifndef socklen_t
#        define socklen_t int
#endif

/* Window's SD_SEND maps to 'nix's SHUT_WR */
#ifndef SHUT_WR
#        define SHUT_WR SD_SEND
#endif

/* Widows' closesocket maps to 'nix's close */
#if defined(_WIN32) || defined(SOC_AM65XX)
#        define net_close_socket(X) closesocket(X)
#else
#        define net_close_socket(X) close(X)
#endif


#ifdef SOC_AM65XX
#        include <ti/ndk/inc/socket.h>
#        define fd_set      NDK_fd_set
#        define FD_ZERO     NDK_FD_ZERO
#        define FD_SET      NDK_FD_SET
#        define FD_ISSET    NDK_FD_ISSET
#        define select      fdSelect
#        define htonl       NDK_htonl
#        define htons       NDK_htons
#        define closesocket fdClose
#endif

#ifdef SOC_AM65XX
#        define SOCKET_INVAL INVALID_SOCKET
#else
#        define SOCKET_INVAL -1
#endif


#define NET_TCP_SOCKET_BLOCKING    0
#define NET_TCP_SOCKET_NONBLOCKING 1

#if !defined(_WIN32)
/* needed for setsockopt */
#        ifndef SOCKET_ERROR
#                define SOCKET_ERROR -1
#        endif
#endif


int set_socket_blockmode(SOCKET s, int mode);

/*
 * Please also note the prototype difference in Windows WSAAPI and UNIX!
 *
 * WSAAPI:
 *      int WSAAPI send(SOCKET s, const char *buf, int len, int flags);
 *      int recv (SOCKET s, char *buf, int len, int flags);
 *
 * UNIX:
 *      ssize_t send(int sockfd, const void *buf, size_t len, int flags);
 *      ssize_t recv(int sockfd, void *buf, size_t len, int flags);
 *
 *      The difference especially is the size of 'len' and return values, since
 *      'int' on 64 bit machines probably is 32 bits depending on the compiler.
 *
 *      Thus netdefs.h defines ssize_t to be int in case it is undefined.
 *      On _WIN32 platform, the 'len' parameter gets casted to 'int'.
 *
 *      On _WIN32, the first parameter of select gets ignored, whereas on UNIX,
 *      it must be set to max_fd + 1. The two macros below help with this
 *      inconsistency.
 */

#ifdef _WIN32
#        define WSOCK_INT(x)    ((int)(x))
#        define SELECT_FIRST(x) 0
#else
#        define WSOCK_INT(x) (x)
#        ifdef SOC_AM65XX
#                define SELECT_FIRST(x) 0
#        else
#                define SELECT_FIRST(x) (x)
#        endif
#endif

#endif
