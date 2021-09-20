#include "net_api.h"

#include "tcp/tcp_server.h"
#include "tcp/tcp_client.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <zlib.h>

#ifdef _WIN32
#        include <winsock2.h>
#        include <errorhandlingapi.h>
#endif

#define error(...)   net_log_err(__VA_ARGS__)
#define warning(...) net_log_warn(__VA_ARGS__)
#define info(...)    net_log_info(__VA_ARGS__)


int net_core_init(void)
{
#ifdef _WIN32
        WSADATA wsaData;
        int iResult;

        iResult = WSAStartup(0x0202, &wsaData);
        if (iResult != 0) {
                return GetLastError();
        }
#endif
        return NET_CORE_OK;
}


static int init_socket(SOCKET *s, sock_type_t sock_type)
{
        switch (sock_type) {
        case SOCK_TYPE_TCP: *s = socket(AF_INET, SOCK_STREAM, 0); break;
        case SOCK_TYPE_UDP:
                *s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                break;
        default: *s = SOCKET_INVAL; break;
        }
        if (*s == SOCKET_INVAL) {
                return NET_CORE_INVALID_SOCKET;
        }

        return NET_CORE_OK;
}


static void init_state(net_state_t *s)
{
        memset(s, 0, sizeof(net_state_t));

        uint32_t recv_to = DEFAULT_RECV_TIMEOUT_US;
        uint32_t send_to = DEFAULT_SEND_TIMEOUT_US;

        s->recv_timeout.tv_sec  = recv_to / 1000000;
        s->recv_timeout.tv_usec = recv_to % 1000000;

        s->send_timeout.tv_sec  = send_to / 1000000;
        s->send_timeout.tv_usec = send_to % 1000000;

        s->max_retries = DEFAULT_NET_RETRIES;

        s->retry_no = 0;
        s->seq_no   = 0;
}


static int net_common_init(net_api_t *n, net_obj_type_t obj_type,
                           sock_type_t sock_type)
{
        if (!n) {
                return -1;
        }

        memset(n, 0, sizeof(net_api_t));

        n->obj_type = obj_type;

        n->sock_type = sock_type;

        init_state(&n->state);

        switch (obj_type) {
        case NET_SERVER: return init_socket(&n->server_addr.sock, sock_type);
        case NET_CLIENT: return init_socket(&n->client_addr.sock, sock_type);
        default: return NET_CORE_INVALID_SOCKET;
        }

        return NET_CORE_INVALID_SOCKET;
}


int net_server_init(net_api_t *n, sock_type_t sock_type, uint32_t ipv4,
                    uint16_t port)
{
        int res = net_common_init(n, NET_SERVER, sock_type);

        if (res != NET_CORE_OK) {
                return res;
        }

        memset(&n->server_addr.saddr, 0, sizeof(struct sockaddr_in));

        n->server_addr.saddr.sin_family      = AF_INET;
        n->server_addr.ipaddr_v4             = ipv4;
        n->server_addr.saddr.sin_addr.s_addr = htonl(ipv4);
        n->server_addr.port                  = port;
        n->server_addr.saddr.sin_port        = htons(port);

        int flag = 1;
        setsockopt(n->server_addr.sock, SOL_SOCKET, SO_REUSEADDR, (char *)&flag,
                   sizeof(flag));

        if (bind(n->server_addr.sock, (struct sockaddr *)&n->server_addr.saddr,
                 sizeof(struct sockaddr_in)) == -1) {
                return NET_CORE_BIND_FAILED;
        }

        if (sock_type == SOCK_TYPE_TCP) {
                if (listen(n->server_addr.sock, 1) == -1) {
                        return NET_CORE_LISTEN_FAILED;
                }
                n->await_cli = tcp_server_await_connection;
        }


        return NET_CORE_OK;
}


int net_client_init(net_api_t *n, sock_type_t sock_type, uint32_t ipv4,
                    uint16_t port)
{
        int res = net_common_init(n, NET_CLIENT, sock_type);

        if (res != NET_CORE_OK) {
                return res;
        }

        memset(&n->client_addr.saddr, 0, sizeof(struct sockaddr_in));

        /* Only TCP and UDP */
        n->server_addr.saddr.sin_family      = AF_INET;
        n->server_addr.ipaddr_v4             = ipv4;
        n->server_addr.saddr.sin_addr.s_addr = htonl(ipv4);
        n->server_addr.port                  = port;
        n->server_addr.saddr.sin_port        = htons(port);

        if (sock_type == SOCK_TYPE_UDP) {

                int bc_enable = 1;
                int res =
                    setsockopt(n->client_addr.sock, SOL_SOCKET, SO_BROADCAST,
                               (char *)&bc_enable, sizeof(bc_enable));
                if (res == SOCKET_ERROR) {
                        net_close_socket(n->client_addr.sock);
                        return NET_CORE_SETSOCKOPT_FAILED;
                }
        }

        if (sock_type == SOCK_TYPE_TCP) {
                n->connect = tcp_client_connect;
        }

        return NET_CORE_OK;
}


void net_server_disconnect_client(net_api_t *n)
{
        net_close_socket(n->client_addr.sock);
}


void net_server_shutdown(net_api_t *n)
{
        net_close_socket(n->server_addr.sock);
}


void net_core_shutdown(void)
{
#ifdef _WIN32
        WSACleanup();
#endif
}


static SOCKET net_get_socket(net_api_t *n)
{
        switch (n->obj_type) {
        case NET_SERVER:
                if (n->sock_type == SOCK_TYPE_UDP) {
                        return n->server_addr.sock;
                } else {
                        return n->client_addr.sock;
                }
        case NET_CLIENT: return n->client_addr.sock;
        default: return SOCKET_INVAL;
        }

        return SOCKET_INVAL;
}


ssize_t net_recv_single(net_api_t *n, net_addr_t *a, char *dst,
                        size_t num_bytes)
{
        const SOCKET s = net_get_socket(n);

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(s, &fds);

        struct timeval tv;
        tv = n->state.recv_timeout;

        int sel = select(SELECT_FIRST((int)s + 1), &fds, NULL, NULL, &tv);
        if (sel <= 0) {
                return sel;
        }

        socklen_t addr_len = sizeof(struct sockaddr_in);

        int res;

        switch (n->sock_type) {
        case SOCK_TYPE_UDP:
                if (a) {
                        memset(a, 0, sizeof(net_addr_t));
                }
                res = recvfrom(s, dst, WSOCK_INT(num_bytes), 0,
                               (struct sockaddr *)&a->saddr, &addr_len);
                info("recvfrom returned %d", res);
                return res;
        case SOCK_TYPE_TCP: return recv(s, dst, WSOCK_INT(num_bytes), 0);
        default: return NET_CORE_NOT_IMPLEMENTED;
        }
}


void net_set_dest_addr(net_api_t *n, net_addr_t *a)
{
        net_addr_t *dest_addr;

        switch (n->obj_type) {
        case NET_SERVER: dest_addr = &n->client_addr; break;
        case NET_CLIENT: dest_addr = &n->server_addr; break;
        default: return;
        }

        dest_addr->saddr = a->saddr;
}


static net_addr_t *net_get_response_addr(net_api_t *n)
{
        /* if it is a server, the client_addr is of the connected
         * client, or must be set by set_dest_addr in case of UDP */
        switch (n->obj_type) {
        case NET_SERVER: return &n->client_addr;
        case NET_CLIENT: return &n->server_addr;
        default: return NULL;
        }

        return NULL;
}


static SOCKET net_get_response_socket(net_api_t *n)
{
        switch (n->obj_type) {
        case NET_SERVER:
                if (n->sock_type == SOCK_TYPE_UDP) {
                        /* Use the socket of the server, with the address
                         * of the client packet to respond */
                        return n->server_addr.sock;
                }
                /* Use the socket of the connected client to respond */
                return n->client_addr.sock;
        case NET_CLIENT:
                /* Use the only socket there is */
                return n->client_addr.sock;
        default: return SOCKET_INVAL;
        }
}


ssize_t net_send_single(net_api_t *n, const char *src, size_t num_bytes)
{
        const net_addr_t *dest = net_get_response_addr(n);
        const SOCKET s         = net_get_response_socket(n);

        struct timeval tv;
        tv = n->state.send_timeout;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(s, &fds);

        int sel = select(SELECT_FIRST((int)s + 1), NULL, &fds, NULL, &tv);
        if (sel <= 0) {
                return sel;
        }

        int res;

        switch (n->sock_type) {
        case SOCK_TYPE_UDP:
                res = sendto(s, (void *)src, WSOCK_INT(num_bytes), 0,
                             (struct sockaddr *)&dest->saddr,
                             sizeof(struct sockaddr_in));
                info("sendto returned %d", res);
                return res;
        case SOCK_TYPE_TCP:
                return send(s, (void *)src, WSOCK_INT(num_bytes), 0);
        default: return NET_CORE_NOT_IMPLEMENTED;
        }
}


int net_send_packetized(net_api_t *n, const char *src, size_t num_bytes)
{
        size_t bytes_remaining = num_bytes;

        net_addr_t from, *pfrom;

        if (n->sock_type == SOCK_TYPE_UDP) {
                pfrom = &from;
        } else {
                pfrom = NULL;
        }

        char send_buf[sizeof(net_header_t) + NET_CORE_PACKET_SIZE];
        char ack_buf[sizeof(net_header_t)];

        net_header_t *send_header = (net_header_t *)send_buf;
        net_header_t *ack_header  = (net_header_t *)ack_buf;

        n->state.seq_no = 0;

        while (bytes_remaining) {
                size_t bytes_to_send;

                if (bytes_remaining < NET_CORE_PACKET_SIZE) {
                        bytes_to_send = bytes_remaining;
                } else {
                        bytes_to_send = NET_CORE_PACKET_SIZE;
                }

                /* prepare data to send */
                memcpy(send_buf + sizeof(net_header_t), src, bytes_to_send);

                /* reset/update state machine and update header */
                n->state.retry_no = n->state.max_retries;
                n->state.seq_no++;
                send_header->seq = n->state.seq_no;

                do {
                        send_header->retry = n->state.retry_no;

                        info("net_send_packetized, seq=%d, try=%d",
                             n->state.seq_no, n->state.retry_no);

                        const ssize_t bytes_sent = net_send_single(
                            n, send_buf, bytes_to_send + sizeof(net_header_t));

                        if (bytes_sent !=
                            bytes_to_send + sizeof(net_header_t)) {
                                /* ignore possible ack, send again */
                                if (bytes_sent == -1) {
                                        /* this is a socket error, return */
                                        return bytes_sent;
                                }
                                n->state.retry_no--;
                                continue;
                        }
                retry_ack:
                        n->state.retry_no--;
                        const ssize_t bytes_received = net_recv_single(
                            n, pfrom, ack_buf, sizeof(net_header_t));
                        if (bytes_received == 0) {
                                /* timeout */
                                warning("send_packetized: timeout on ack");
                        }

                        if (bytes_received == -1) {
                                error("send_packetized: socket error");
                                return -1;
                        }

                        if (bytes_received != sizeof(net_header_t)) {
                                /* ACK does not match, do not resend, but
                                 * retry ACK only, as ACK may come from a
                                 * previous incomplete send */
                                if (bytes_received == -1) {
                                        /* this is a socket error, return */
                                        return bytes_received;
                                }
                                if (n->state.retry_no > 0) {
                                        warning(
                                            "net_send_packetized: retry ACK");
                                        goto retry_ack;
                                }
                                error("net_send_packetized: Got no ACK, "
                                      "aborting");
                        } else {
                                /* check retry and sequence number */
                                if (ack_header->seq != send_header->seq ||
                                    ack_header->retry != send_header->retry) {
                                        warning("net_send_packetized: ACK "
                                                "mismatch");
                                        if (n->state.retry_no > 0) {
                                                warning("net_send_packetized: "
                                                        "retry ACK");
                                                goto retry_ack;
                                        }
                                        error("net_send_packetized: abort");
                                        break;
                                }
                                /* Here, ACK is complete, break retry loop */
                                bytes_remaining -= bytes_to_send;
                                src += bytes_to_send;
                                break;
                        }

                } while (n->state.retry_no > 0);

                if (n->state.retry_no == 0) {
                        return -1;
                }
        }

        return NET_CORE_OK;
}


int net_recv_packetized(net_api_t *n, net_addr_t *from, char *dst,
                        size_t num_bytes)
{
        size_t bytes_remaining = num_bytes;

        net_addr_t f, *pfrom;

        if (n->sock_type == SOCK_TYPE_UDP) {
                pfrom = &f;
        } else {
                pfrom = NULL;
        }

        char recv_buf[sizeof(net_header_t) + NET_CORE_PACKET_SIZE];
        char ack_buf[sizeof(net_header_t)];

        net_header_t *recv_header = (net_header_t *)recv_buf;
        net_header_t *ack_header  = (net_header_t *)ack_buf;

        n->state.seq_no = 0;

        while (bytes_remaining) {
                size_t bytes_to_receive;

                if (bytes_remaining < NET_CORE_PACKET_SIZE) {
                        bytes_to_receive = bytes_remaining;
                } else {
                        bytes_to_receive = NET_CORE_PACKET_SIZE;
                }

                n->state.retry_no = n->state.max_retries;
                n->state.seq_no++;

                do {
                        info("net_recv_packetized: seq #%d, retry #%d",
                             n->state.seq_no, n->state.retry_no);

                        const ssize_t bytes_received = net_recv_single(
                            n, pfrom, recv_buf,
                            bytes_to_receive + sizeof(net_header_t));

                        if (bytes_received == 0) {
                                warning("recv_packetized: timeout");
                                n->state.retry_no--;
                                continue;
                        }

                        if (bytes_received == -1) {
                                /* this is a socket error, return */
                                error("recv_packetized: socket error");
                                return bytes_received;
                        }

                        if (bytes_received !=
                            bytes_to_receive + sizeof(net_header_t)) {
                                /* size mismatch */
                                n->state.retry_no--;
                                warning(
                                    "recv_packetized: Size mismatch, got %ld",
                                    bytes_received);
                                continue;
                        }

                        /* check if it is coming from the same sender as before
                         */
                        if (n->sock_type == SOCK_TYPE_UDP) {
                                if (n->state.seq_no > 1) {
                                        if (f.saddr.sin_addr.s_addr !=
                                            from->saddr.sin_addr.s_addr) {
                                                /* no, throw away and retry */
                                                warning(
                                                    "recv_packetized: Sender "
                                                    "mismatch");
                                                n->state.retry_no--;
                                                continue;
                                        }
                                } else {
                                        /* first packet, expect all following to
                                         * come from the same sender */
                                        *from = *pfrom;
                                }
                        }
                        /* check if this is the correct sequence number */
                        if (recv_header->seq != n->state.seq_no) {
                                /* wrong sequence number, try again */
                                n->state.retry_no--;
                                warning("recv_packetized: Mismatch in "
                                        "sequence number");
                                continue;
                        }

                        /* data good, store datagram */
                        memcpy(dst, recv_buf + sizeof(net_header_t),
                               bytes_to_receive);
                        /* UDP server needs hint to where to send the ACK */
                        if (n->obj_type == NET_SERVER &&
                            n->sock_type == SOCK_TYPE_UDP) {
                                net_set_dest_addr(n, pfrom);
                        }
                retry_ack2:
                        n->state.retry_no--;
                        /* sequence numbers are identical (checked above) */
                        ack_header->seq = n->state.seq_no;
                        /* ACK the retry number issued by the sender */
                        ack_header->retry = recv_header->retry;
                        const ssize_t bytes_sent =
                            net_send_single(n, ack_buf, sizeof(net_header_t));

                        if (bytes_sent != sizeof(net_header_t)) {
                                if (bytes_sent == -1) {
                                        /* this is a socket error, return */
                                        return -1;
                                }
                                /* timeout or size mismatch */
                                if (n->state.retry_no > 0) {
                                        goto retry_ack2;
                                }
                        } else {
                                /* Here, ACK is complete, break retry loop */
                                bytes_remaining -= bytes_to_receive;
                                dst += bytes_to_receive;
                                break;
                        }

                } while (n->state.retry_no > 0);

                if (n->state.retry_no == 0) {
                        error("recv_packetized: Retries failed");
                        return -1;
                }
        }

        return NET_CORE_OK;
}


ssize_t net_recv_stream(net_api_t *n, net_addr_t *from, char *dst,
                        size_t num_bytes)
{
        if (n->sock_type != SOCK_TYPE_TCP) {
                return NET_CORE_WRONG_SOCK_TYPE;
        }

        (void)from;

        ssize_t bytes_received = 0;
        while (bytes_received < num_bytes) {

                ssize_t bytes_read;
                n->state.retry_no = n->state.max_retries;
                do {
                        bytes_read =
                            net_recv_single(n, NULL, dst + bytes_received,
                                            num_bytes - bytes_received);
                } while (--(n->state.retry_no) > 0 && bytes_read == 0);

                if (bytes_read == 0) {
                        return NET_CORE_STREAM_TIMEOUT;
                }

                if (bytes_read == -1) {
                        return NET_CORE_INVALID_SOCKET;
                }

                bytes_received += bytes_read;
        }

        return bytes_received;
}


ssize_t net_send_stream(net_api_t *n, const char *src, size_t num_bytes)
{
        /* this function is a wrapper just for naming convention */
        return net_send_single(n, src, num_bytes);
}


ssize_t net_recv_stream_inflate(net_api_t *n, net_addr_t *from, char *dst,
                                size_t num_bytes)
{
        (void)from;

        if (n->sock_type != SOCK_TYPE_TCP) {
                return NET_CORE_WRONG_SOCK_TYPE;
        }

        char *zbuff = malloc(NET_CORE_ZSTREAM_DEFLATE_BLOCK);
        if (!zbuff) {
                return NET_CORE_ENOMEM;
        }

        z_stream z = {0};

        if (inflateInit(&z) != Z_OK) {
                free(zbuff);
                return NET_CORE_ZLIB_ERROR;
        };

        /* directly decompress to the destination buffer */
        z.next_out  = (Bytef *)dst;
        z.avail_out = num_bytes;

        ssize_t bytes_remaining = num_bytes;
        n->compression_ratio    = 0;

        bool stream_end = false;

        while (bytes_remaining) {
                ssize_t bytes_read;
                n->state.retry_no = n->state.max_retries;

                do {
                        bytes_read = net_recv_single(
                            n, NULL, zbuff, NET_CORE_ZSTREAM_DEFLATE_BLOCK);

                } while (--(n->state.retry_no) > 0 && bytes_read == 0);
                n->compression_ratio += (double)bytes_read;

                if (bytes_read <= 0) {
                        free(zbuff);
                        inflateEnd(&z);
                        return bytes_read == 0 ? NET_CORE_STREAM_TIMEOUT
                                               : NET_CORE_INVALID_SOCKET;
                }

                /* here we read a chunk of compressed data which might not be
                 * decompressible fully yet */
                z.next_in  = (Bytef *)zbuff;
                z.avail_in = bytes_read;

                const int ret = inflate(&z, Z_SYNC_FLUSH);
                switch (ret) {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                        free(zbuff);
                        (void)inflateEnd(&z);
                        return NET_CORE_ZSTREAM_ERROR;
                }

                bytes_remaining = z.avail_out;
                if (ret == Z_STREAM_END) {
                        /* Here we have the zstream end, no matter if we got
                         * all bytes or not */
                        stream_end = true;
                        break;
                }
        }
        free(zbuff);
        inflateEnd(&z);

        if (!stream_end) {
                return NET_CORE_ZSTREAM_ERROR;
        }
        n->compression_ratio /= (num_bytes - bytes_remaining);
        return num_bytes - bytes_remaining;
}


ssize_t net_send_stream_deflate(net_api_t *n, const char *src, size_t num_bytes)
{
        if (n->sock_type != SOCK_TYPE_TCP) {
                return NET_CORE_WRONG_SOCK_TYPE;
        }

        size_t raw_bytes_left   = num_bytes;
        const size_t block_size = NET_CORE_ZSTREAM_DEFLATE_BLOCK;
        size_t bytes_to_send;

        z_stream z = {0};
        z.avail_in = num_bytes;
        z.next_in  = (Bytef *)src;

        if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
                return NET_CORE_ZLIB_ERROR;
        }

        n->compression_ratio = 0;
        while (raw_bytes_left > 0) {
                if (raw_bytes_left >= block_size) {
                        bytes_to_send = block_size;
                } else {
                        bytes_to_send = raw_bytes_left;
                }
                /* estimate the buffer needed for deflation of a block of size
                 * bytes_to_send */
                const size_t est_zlen = deflateBound(&z, bytes_to_send);
                char *zbuff           = malloc(est_zlen);
                if (!zbuff) {
                        deflateEnd(&z);
                        return NET_CORE_ENOMEM;
                }
                z.avail_out = est_zlen;
                z.next_out  = (Bytef *)zbuff;
                if (raw_bytes_left == bytes_to_send) {
                        deflate(&z, Z_FINISH); /* tell the decompressor the end
                                                  of stream */
                } else {
                        deflate(&z, Z_BLOCK); /* emmit all data, so that
                                                 decompressor can work with
                                                 this block after receiving
                                                 it as a whole, but do not
                                                 reset the compressor */
                }

                const size_t real_zlen = est_zlen - z.avail_out;
                char *send_buff        = zbuff;
                size_t send_bytes_left = real_zlen;
                while (send_bytes_left) {
                        const ssize_t bytes_sent =
                            net_send_single(n, send_buff, real_zlen);
                        if (bytes_sent <= 0) {
                                /* timeout or socket error */
                                free(zbuff);
                                deflateEnd(&z);
                                return bytes_sent == 0
                                           ? NET_CORE_STREAM_TIMEOUT
                                           : NET_CORE_INVALID_SOCKET;
                        }
                        send_buff += bytes_sent;
                        send_bytes_left -= bytes_sent;
                }
                free(zbuff);
                raw_bytes_left -= bytes_to_send;
                n->compression_ratio += (double)real_zlen;
        }

        deflateEnd(&z);
        n->compression_ratio /= num_bytes;
        return num_bytes;
}


static log_func_t log_err_fn;
static log_func_t log_warn_fn;
static log_func_t log_info_fn;


void net_set_log_err_func(log_func_t log_func)
{
        log_err_fn = log_func;
}


void net_set_log_warn_func(log_func_t log_func)
{
        log_warn_fn = log_func;
}


void net_set_log_info_func(log_func_t log_func)
{
        log_info_fn = log_func;
}


void net_log_err(char *msg, ...)
{
        va_list args;

        va_start(args, msg);
        if (log_err_fn) {
                log_err_fn(msg, args);
        } else {
                char buffer[256];
                snprintf(buffer, 255, "net_core: ERROR: %s\n", msg);
                vprintf(buffer, args);
        }
        va_end(args);
}


void net_log_warn(char *msg, ...)
{
        va_list args;

        va_start(args, msg);
        if (log_warn_fn) {
                log_warn_fn(msg, args);
        } else {
                char buffer[256];
                snprintf(buffer, 255, "net_core: WARNING: %s\n", msg);
                vprintf(buffer, args);
        }
        va_end(args);
}


void net_log_info(char *msg, ...)
{
        va_list args;

        va_start(args, msg);
        if (log_info_fn) {
                log_info_fn(msg, args);
        } else {
                char buffer[256];
                snprintf(buffer, 255, "net_core: INFO: %s\n", msg);
                vprintf(buffer, args);
        }
        va_end(args);
}
