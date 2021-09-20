#ifndef NET_CORE_H
#define NET_CORE_H

#include <stdint.h>

#include "netdefs.h"


#define NET_CORE_OK                0
#define NET_CORE_INVALID_PROTOCOL  1
#define NET_CORE_TIMEOUT           3
#define NET_CORE_INET_PTON_FAILED  4
#define NET_CORE_CONNECT_FAILED    5
#define NET_CORE_LISTEN_FAILED     6
#define NET_CORE_BIND_FAILED       7
#define NET_CORE_SETSOCKOPT_FAILED 8

#define NET_CORE_WRONG_SOCK_TYPE -1
#define NET_CORE_WRONG_OBJ_TYPE  -2
#define NET_CORE_NOT_IMPLEMENTED -3
#define NET_CORE_INVALID_SOCKET  -4
#define NET_CORE_STREAM_TIMEOUT  -5
#define NET_CORE_ZSTREAM_ERROR   -6
#define NET_CORE_ZLIB_ERROR      -7
#define NET_CORE_ENOMEM          -8

#define DEFAULT_SEND_TIMEOUT_US 100000
#define DEFAULT_RECV_TIMEOUT_US 100000
#define DEFAULT_NET_RETRIES     3

#define UDP_MAX 65507

#define NET_CORE_ZSTREAM_DEFLATE_BLOCK 262144

#pragma pack(push, 1)
typedef struct net_header {
        uint32_t seq;
        uint32_t retry;
} net_header_t;

#define NET_CORE_PACKET_SIZE (UDP_MAX - sizeof(net_header_t))


struct net_addr {
        SOCKET sock;
        struct sockaddr_in saddr;
        uint32_t ipaddr_v4;
        uint16_t port;
};
#pragma pack(pop)


#include "net_api.h"

uint64_t net_core_connect(net_api_t *n);
uint64_t net_core_disconnect(net_api_t *n);

#endif
