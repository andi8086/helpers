#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <criterion/criterion.h>
#include "net_api.h"

#include <windows.h>
#include <time.h>
#include <stdio.h>

#define TEST_UDP_PORT    45678
#define TEST_BUFFER_SIZE 10481415

#define TEST_TCP_PORT 45677


static DWORD WINAPI test_server_thread_func_tcp(LPVOID lpParam)
{
        net_api_t server;
        net_api_t client;

        cr_expect_eq(NET_CORE_OK, net_server_init(&server, SOCK_TYPE_TCP,
                                                  INADDR_ANY, TEST_TCP_PORT));

        server.state.recv_timeout.tv_sec = 1;
        server.state.send_timeout.tv_sec = 1; /* for sending ACK */

        cr_expect_eq(NET_CORE_OK, server.await_cli(&server, &client));

        char *recv_buff;

        recv_buff = (char *)malloc(TEST_BUFFER_SIZE);

        clock_t begin = clock();
        ssize_t bytes_received =
            net_recv_stream(&client, NULL, recv_buff, TEST_BUFFER_SIZE);

        clock_t end       = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        cr_log_info("TCP send %u bytes needs %lf seconds\n", TEST_BUFFER_SIZE,
                    time_spent);

        cr_expect(bytes_received == TEST_BUFFER_SIZE);

        net_close_socket(server.server_addr.sock);

        free(recv_buff);
        return 0;
}


Test(NET_CORE, tcp_send_recv_stream)
{
        HANDLE server_thread;
        DWORD thread_id;

        net_api_t client;

        cr_expect_eq(NET_CORE_OK, net_client_init(&client, SOCK_TYPE_TCP,
                                                  ntohl(inet_addr("127.0.0.1")),
                                                  TEST_TCP_PORT));

        server_thread = CreateThread(NULL, 0, test_server_thread_func_tcp, NULL,
                                     0, &thread_id);

        cr_expect(server_thread != NULL);

        char *send_buff = (char *)malloc(TEST_BUFFER_SIZE);

        client.state.send_timeout.tv_sec = 1;
        client.state.recv_timeout.tv_sec = 1; /* for receiving ACK */

        client.connect(&client);

        int res = net_send_single(&client, send_buff, TEST_BUFFER_SIZE);
        cr_expect(res == TEST_BUFFER_SIZE);

        free(send_buff);

        WaitForSingleObject(server_thread, INFINITE);
        CloseHandle(server_thread);

        net_close_socket(client.client_addr.sock);
}


static DWORD WINAPI test_server_thread_func_tcp_compressed(LPVOID lpParam)
{
        net_api_t server;
        net_api_t client;

        cr_expect_eq(NET_CORE_OK, net_server_init(&server, SOCK_TYPE_TCP,
                                                  INADDR_ANY, TEST_TCP_PORT));

        server.state.recv_timeout.tv_sec = 3;
        server.state.send_timeout.tv_sec = 3; /* for sending ACK */

        cr_expect_eq(NET_CORE_OK, server.await_cli(&server, &client));

        char *recv_buff = (char *)lpParam;

        clock_t begin = clock();
        ssize_t bytes_received =
            net_recv_stream_inflate(&client, NULL, recv_buff, TEST_BUFFER_SIZE);

        clock_t end       = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        cr_log_info("TCP send %u bytes needs %lf seconds\n", TEST_BUFFER_SIZE,
                    time_spent);

        cr_log_info("bytes received: %lu\n", bytes_received);
        cr_log_info("stream_inflate: Decompression Ratio = 1/%lf\n",
                    server.compression_ratio);
        cr_expect(bytes_received == TEST_BUFFER_SIZE);

        net_close_socket(server.server_addr.sock);

        return 0;
}


Test(NET_CORE, tcp_send_recv_stream_compressed)
{
        HANDLE server_thread;
        DWORD thread_id;

        net_api_t client;

        cr_expect_eq(NET_CORE_OK, net_client_init(&client, SOCK_TYPE_TCP,
                                                  ntohl(inet_addr("127.0.0.1")),
                                                  TEST_TCP_PORT));

        char *recv_buff = (char *)malloc(TEST_BUFFER_SIZE);
        server_thread =
            CreateThread(NULL, 0, test_server_thread_func_tcp_compressed,
                         recv_buff, 0, &thread_id);

        cr_expect(server_thread != NULL);

        char *send_buff = (char *)malloc(TEST_BUFFER_SIZE);
        for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
                send_buff[i] = '@' + (char)(rand() % 52);
        }

        client.state.send_timeout.tv_sec = 10;

        client.connect(&client);

        int res = net_send_stream_deflate(&client, send_buff, TEST_BUFFER_SIZE);
        cr_expect(res == TEST_BUFFER_SIZE);
        cr_log_info("stream_deflate: Compression Ratio = %lf\n",
                    client.compression_ratio);

        WaitForSingleObject(server_thread, INFINITE);
        CloseHandle(server_thread);

        net_close_socket(client.client_addr.sock);

        res = memcmp(recv_buff, send_buff, TEST_BUFFER_SIZE);
        cr_expect(res == 0);
        free(send_buff);
        free(recv_buff);
}


static DWORD WINAPI test_server_thread_func(LPVOID lpParam)
{
        net_api_t server;

        cr_expect_eq(NET_CORE_OK, net_server_init(&server, SOCK_TYPE_UDP,
                                                  INADDR_ANY, TEST_UDP_PORT));

        net_addr_t from;
        char *recv_buff;

        recv_buff = (char *)malloc(TEST_BUFFER_SIZE);

        server.state.recv_timeout.tv_sec = 1;
        server.state.send_timeout.tv_sec = 1; /* for sending ACK */

        clock_t begin = clock();
        int res =
            net_recv_packetized(&server, &from, recv_buff, TEST_BUFFER_SIZE);
        clock_t end       = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        cr_log_info("UDP send %u bytes needs %lf seconds\n", TEST_BUFFER_SIZE,
                    time_spent);

        cr_expect(res == NET_CORE_OK);

        net_close_socket(server.server_addr.sock);

        free(recv_buff);
        return 0;
}


Test(NET_CORE, udp_send_recv_packetized)
{
        HANDLE server_thread;
        DWORD thread_id;

        net_api_t client;

        cr_expect_eq(NET_CORE_OK, net_client_init(&client, SOCK_TYPE_UDP,
                                                  ntohl(inet_addr("127.0.0.1")),
                                                  TEST_UDP_PORT));

        server_thread =
            CreateThread(NULL, 0, test_server_thread_func, NULL, 0, &thread_id);

        cr_expect(server_thread != NULL);

        char *send_buff = (char *)malloc(TEST_BUFFER_SIZE);

        client.state.send_timeout.tv_sec = 1;
        client.state.recv_timeout.tv_sec = 1; /* for receiving ACK */
        int res = net_send_packetized(&client, send_buff, TEST_BUFFER_SIZE);
        // int res = 0;
        cr_expect(res == NET_CORE_OK);

        free(send_buff);

        WaitForSingleObject(server_thread, INFINITE);
        CloseHandle(server_thread);

        net_close_socket(client.client_addr.sock);
}


Test(NET_CORE, udp_send_recv_single)
{
        const char *test_data      = "This is some test data";
        const size_t test_data_len = strlen(test_data) + 1;

        char server_buf[128];
        char client_buf[128];

        net_api_t server;
        net_api_t client;
        net_addr_t from_addr;

        cr_expect_eq(NET_CORE_OK, net_server_init(&server, SOCK_TYPE_UDP,
                                                  INADDR_ANY, TEST_UDP_PORT));

        /* connect to local server (127.0.0.1) */
        cr_expect_eq(NET_CORE_OK, net_client_init(&client, SOCK_TYPE_UDP,
                                                  ntohl(inet_addr("127.0.0.1")),
                                                  TEST_UDP_PORT));

        /* message from client to server */
        ssize_t bytes_sent =
            net_send_single(&client, (char *)test_data, test_data_len);
        cr_expect_eq(test_data_len, bytes_sent);

        /* recv message on server side */
        ssize_t bytes_recv = net_recv_single(&server, &from_addr, server_buf,
                                             sizeof(server_buf));
        cr_expect_eq(bytes_sent, bytes_recv);

        /* response from server to client */
        net_set_dest_addr(&server, &from_addr);
        bytes_sent = net_send_single(&server, server_buf, bytes_recv);
        cr_expect_eq(bytes_recv, bytes_sent);

        /* recv response on client side */
        bytes_recv = net_recv_single(&client, &from_addr, client_buf,
                                     sizeof(client_buf));
        cr_expect_eq(bytes_sent, bytes_recv);

        /* original message received */
        cr_expect(strcmp(test_data, client_buf) == 0, "%s %s", test_data,
                  client_buf);
}
