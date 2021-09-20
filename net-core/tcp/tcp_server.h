#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "../net_api.h"


int tcp_server_await_connection(net_api_t *n, net_api_t *cli);


#endif
