#include <stdio.h>
#include <string.h>

#include "netdefs.h"
#include "net_core.h"


uint64_t net_core_tcp_disconnect(net_api_t *n)
{
        net_close_socket(n->client_addr.sock);
        n->client_addr.sock = SOCKET_INVAL;
        return NET_CORE_OK;
}


uint64_t net_core_connect(net_api_t *n)
{
        switch (n->sock_type) {
        case SOCK_TYPE_TCP: return n->connect(n);
        default: return NET_CORE_INVALID_PROTOCOL;
        }
}


uint64_t net_core_disconnect(net_api_t *n)
{
        switch (n->sock_type) {
        case SOCK_TYPE_TCP: return net_core_tcp_disconnect(n);
        default: return NET_CORE_INVALID_PROTOCOL;
        }
}
