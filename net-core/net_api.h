#ifndef NET_API_H
#define NET_API_H


#include "netdefs.h"
#include "net_sock_type.h"


typedef struct net_api net_api_t;
typedef struct net_addr net_addr_t;

typedef int (*server_await_cli_t)(net_api_t *n, net_api_t *cli);
typedef int (*client_connect_t)(net_api_t *n);

typedef enum { NET_SERVER = 1, NET_CLIENT } net_obj_type_t;

#include "net_core.h"

#include <stdarg.h>

typedef struct net_state {
        uint32_t max_retries;
        struct timeval recv_timeout;
        struct timeval send_timeout;
        uint32_t retry_no;
        uint32_t seq_no;
} net_state_t;


struct net_api {
        net_obj_type_t obj_type;
        sock_type_t sock_type;
        net_state_t state;

        net_addr_t server_addr;
        net_addr_t client_addr;

        server_await_cli_t await_cli;
        client_connect_t connect;
        double compression_ratio;
};


typedef void (*log_func_t)(char *msg, va_list args);

int net_core_init(void);
int net_server_init(net_api_t *n, sock_type_t sock_type, uint32_t ipv4,
                    uint16_t port);
int net_client_init(net_api_t *n, sock_type_t sock_type, uint32_t ipv4,
                    uint16_t port);
void net_server_disconnect_client(net_api_t *n);
void net_server_shutdown(net_api_t *n);
void net_core_shutdown(void);
ssize_t net_recv_single(net_api_t *n, net_addr_t *a, char *dst,
                        size_t num_bytes);
void net_set_dest_addr(net_api_t *n, net_addr_t *a);
ssize_t net_send_single(net_api_t *n, const char *src, size_t num_bytes);
int net_send_packetized(net_api_t *n, const char *src, size_t num_bytes);
int net_recv_packetized(net_api_t *n, net_addr_t *from, char *dst,
                        size_t num_bytes);
ssize_t net_recv_stream(net_api_t *n, net_addr_t *from, char *dst,
                        size_t num_bytes);
ssize_t net_send_stream(net_api_t *n, const char *src, size_t num_bytes);
ssize_t net_recv_stream_inflate(net_api_t *n, net_addr_t *from, char *dst,
                                size_t num_bytes);
ssize_t net_send_stream_deflate(net_api_t *n, const char *src,
                                size_t num_bytes);
void net_set_log_err_func(log_func_t log_func);
void net_set_log_warn_func(log_func_t log_func);
void net_set_log_info_func(log_func_t log_func);
void net_log_err(char *msg, ...);
void net_log_warn(char *msg, ...);
void net_log_info(char *msg, ...);


#endif
