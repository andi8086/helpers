#include "tcp_server.h"
#include <string.h>


int tcp_server_await_connection(net_api_t *n, net_api_t *cli)
{
        memcpy(cli, n, sizeof(net_api_t));
        socklen_t addr_len = sizeof(struct sockaddr_in);
        cli->client_addr.sock =
            accept(n->server_addr.sock,
                   (struct sockaddr *)&cli->client_addr.saddr, &addr_len);
        if (cli->client_addr.sock == SOCKET_INVAL) {
                return NET_CORE_INVALID_SOCKET;
        }

        return NET_CORE_OK;
}
