#include "tcp_client.h"


int tcp_client_connect(net_api_t *n)
{
        if (connect(n->client_addr.sock,
                    (const struct sockaddr *)&n->server_addr.saddr,
                    sizeof(struct sockaddr_in)) < 0) {
                return NET_CORE_CONNECT_FAILED;
        }
        return NET_CORE_OK;
}
