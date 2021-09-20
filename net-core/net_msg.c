#include "net_api.h"
#include "net_msg.h"


ssize_t net_send_packet(net_api_t *n, char *data, size_t size,
                        uint8_t user_type)
{
        ssize_t sent = 0;

        i_net_msg_t header;
        /* create a net_msg_t header and send it first */
        header.len          = size;
        header.type         = user_type;
        header.header_magic = I_NET_MSG_HEADER_MAGIC;

        do {
                ssize_t res;
                res = net_send_single(n, (char *)&header + sent,
                                      sizeof(header) - sent);
                if (res <= 0) {
                        /* socket error or timeout */
                        return res;
                }
                sent += res;
        } while (sent < sizeof(header));

        if (size == 0) {
                /* special case, only send the header */
                return 1;
        }

        sent = 0;
        do {
                ssize_t res;
                res = net_send_single(n, data + sent, size - sent);
                if (res <= 0) {
                        /* socket error or timeout */
                        return res;
                }
                sent += res;
        } while (sent < size);
        return sent;
}


ssize_t net_recv_packet(net_api_t *n, char *dst, size_t *size,
                        uint8_t *user_type)
{
        ssize_t got = 0;

        i_net_msg_t header;

        do {
                ssize_t res;
                res = net_recv_single(n, NULL, (char *)&header + got,
                                      sizeof(header) - got);
                if (res <= 0) {
                        /* socket error or timeout */
                        return res;
                }
                got += res;
        } while (got < sizeof(header));

        if (header.header_magic != I_NET_MSG_HEADER_MAGIC) {
                /* this is not a header, return error */
                return I_NET_MSG_WRONG_HEADER;
        }

        if (user_type) {
                *user_type = header.type;
        }

        size_t rsize = size ? *size : 0;

        if (header.len != rsize) {
                if (size) {
                        *size = header.len;
                }
                return I_NET_MSG_SIZE_MISMATCH;
        }

        if (rsize == 0) {
                /* Special case were we only want the header */
                return 1;
        }

        got = 0;
        do {
                ssize_t res;
                res = net_recv_single(n, NULL, dst + got, rsize - got);
                if (res <= 0) {
                        /* socket error or timeout */
                        return res;
                }
                got += res;
        } while (got < rsize);
        return got;
}
