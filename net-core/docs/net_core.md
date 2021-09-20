# Net-Core networking abstraction layer

This is a networking abstraction layer which simplifies usage of UDP and TCP
data transfer in firmware and libraries.

The layer consists of the following key components:

* `struct net_addr`
* `struct net_api`
* `net_server_init / net_client_init`
* `net_send_single / net_send_packetized / net_send_stream`
* `net_recv_single / net_recv_packetized / net_recv_stream`

## `struct net_addr`

Net generic `net_addr` structure stores information for accessing a point in
the networking infra structure:


```C
struct net_addr {
        SOCKET sock;
        uint16_t port;
        uint32_t ipaddr_v4;
        struct sockaddr_in saddr;
};
```

`sock` is the socket used for communication. `port` and `ipaddr_v4` store the
values in host byte order. `saddr` is the `sockaddr_in` struct that is used for
the common `unix` networking functions or alike.

## `struct net_api`

This structure provides an object that describes some functionality as well as
information assigned with a server or client object.

```C
struct net_api {
        ...
        net_obj_type_t obj_type;
        sock_type_t sock_type;
        net_addr_t server_addr;
        net_addr_t client_addr;
        ...

}
```

The object type sets wether this struct describes a `NET_SERVER` or a
`NET_CLIENT`. The `sock_type` currently provides either `SOCK_TYPE_UDP`
or `SOCK_TYPE_TCP`

The structure is initialized with `net_server_init` or `net_client_init`.

The combination of `server_addr` and `client_addr` is used as follows:

| obj_type   | task          |  how                     |
| ---        | ---           | ---                      |
| tcp server | send to cli   |  to `client_addr.sock`   |
|            | recv from cli |  from `client_addr.sock` |
| tcp client | send to srv   |  to `client_addr.sock`, connected to `server_addr.saddr` |
|            | recv from srv |  from `client_addr.sock`, connected to `server_addr.saddr` |
| udp server | send to cli   |  to `server_addr.sock`, using `net_set_dest_addr`, which updates `client_addr.saddr` |
|            | recv from cli |  from `server_addr.sock` |
| udp client | send to srv   |  to `client_addr.sock`, using `server_addr.saddr` |
|            | recv from srv |  from `client_addr.sock` |


After being initialized, it provides two functions for connection based TCP
protocol:

```C
struct net_api {
        ...
        server_await_cli_t await_cli;
        client_connect_t connect;
        ...
}
```

```C
int await_cli(net_api_t *n, net_api_t *cli);
int connect(net_api_t *n);
```

The `await_cli` function copies the server state from `n` to `cli`, waits for
a connection and fills in the connection parameters into the `cli`.

This way it is possible to have more than one client connected, each with its own
`cli` object.


## `net_server_init / net_client_init`

These two functions initialize a `net_api` structure for further usage.

To initialize a server, that listens on any address on port 23 tcp, use the
following:

```C
net_api_t server;

int res = net_server_init(&server, SOCK_TYPE_TCP, INADDR_ANY, 23);
if (res != NET_CORE_OK) {
        /* server init failed */
}
```

To initialize a client structure, use

```C
net_api_t client;

int res = net_client_init(&client, SOCK_TYPE_TCP, SERVER_IP, 23);
```

The client does not connect automatically, but it stores the server's address
in the `server_addr.saddr struct member`.  In case of UDP, if the client sends
to the server, this works automatically. For the server sending to a client in
UDP, the `net_set_dest_addr` function must be utilized.

Please note, the IP address and the port are both in **HOST BYTE ORDER**.
You can use `ntohl(inet_addr("127.0.0.1"))` for example to fill in the ip address
parameter. Only IPv4 is supported at the moment.


After calling `net_client_init`, the `net_set_slx_instance` function must be called.


## `net_send_* / net_recv_*`

To accomodate different protocols, there are different send and receive functions. The following table summarizes supported protocols:

| function              | TCP     | UDP    | Remarks                                   |
| ---                   | ---     | ---    | ---                                       |
| `net_send_single`     | **yes** |**yes** | 65507 bytes for UDP, 2 GB for TCP |
| `net_recv_single`     | **yes** |**yes** | needs to be called multiple times for TCP |
| `net_send_stream`     | **yes** | no     |                                           |
| `net_recv_stream`     | **yes** | no     |                                           |
| `net_send_packetized` | **yes** |**yes** | implemented for UDP, should work for TCP  |
| `net_recv_packetized` | **yes** |**yes** | if additional handshaking is required |


