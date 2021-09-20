#ifndef NETCORE_MSG_H
#define NETCORE_MSG_H

/* This defines a simple protocol with a header
 * So that the receiver knows how many bytes
 * are to be received */

#include <stdint.h>
/* nETCOREf */
#define I_NET_MSG_HEADER_MAGIC 0x237C0A3F

#pragma pack(push, 1)
typedef struct {
        uint32_t header_magic; /* can be set to NET_CORE_HEADER */
        uint32_t len;          /* length of data to be transmitted  */
        uint8_t type;          /* user defined msg type */
} i_net_msg_t;
#pragma pack(pop)

_Static_assert(sizeof(i_net_msg_t) == 9, "Error, sizeof(i_net_msg_t) != 9");

#define I_NET_MSG_WRONG_HEADER  -65536;
#define I_NET_MSG_WRONG_LENGTH  -65537;
#define I_NET_MSG_SIZE_MISMATCH -65538;

ssize_t net_recv_packet(net_api_t *n, char *dst, size_t *size,
                        uint8_t *user_type);
ssize_t net_send_packet(net_api_t *n, char *data, size_t size,
                        uint8_t user_type);

#endif
