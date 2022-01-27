#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "include/util.h"
#include "include/server_util.h"

int main(int argc, char *argv[])
{
    DIE(argc != 2, USAGE_DISP);
    int sockfd_tcp, sockfd_udp, portno, fdmax, ret, i;
    unsigned int addr_len;
    __SOCKADDR_ARG from_udp_station = NULL;
    struct sockaddr_in serv_addr, cli_addr;
    struct LinkedList *clients_container;
    fd_set read_fds;
    fd_set tmp_fds;
    char *buf = malloc(MAX_LEN_BUF * sizeof(char)), response[sizeof(server_to_client_hdr) + 1];
    /************************************/
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    /************************************/
    clients_container = malloc(sizeof(struct LinkedList));
    init_list(clients_container);
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd_tcp, SOL_TCP, TCP_NODELAY, NULL, sizeof(int));

    DIE(sockfd_tcp < 0, UDP_SOCK_ERR);

    portno = atoi(argv[1]);
    DIE(portno == 0, ATOI);

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(sockfd_tcp, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    DIE(ret < 0, TCP_BIND_ERR);

    ret = listen(sockfd_tcp, MAX_CLIENTS);
    DIE(ret < 0, LISTEN_ERR);

    sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(sockfd_udp == -1, UDP_SOCK_ERR);

    ret = bind(sockfd_udp, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    DIE(ret < 0, UDP_CONN_ERR);
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd_tcp, &read_fds);
    FD_SET(sockfd_udp, &read_fds);
    fdmax = sockfd_udp;

    while (FOREVER)
    {
        tmp_fds = read_fds;
        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, SELECT_ERR);
        int end = 0;
        for (i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &tmp_fds))
            {
                if (i == sockfd_tcp)
                {
                    manage_connection_request(sockfd_tcp, &cli_addr,
                                              clients_container, &read_fds, buf, &fdmax);
                }
                else if (i == sockfd_udp)
                {
                    // packet de la unul din clientii UDP
                    message msg;
                    addr_len = sizeof(*from_udp_station);
                    memset(buf, 0, MAX_LEN_BUF);
                    from_udp_station = malloc(sizeof(struct sockaddr));
                    ret = recvfrom(sockfd_udp, buf, MAX_LEN_BUF, 0, from_udp_station, &addr_len);

                    DIE(ret <= 0, UDP_RECV_ERR);
                    build_msg(buf, &msg, ret);
                    build_sentBuffer(buf, msg, from_udp_station);
                    send_data(buf, msg.topic, clients_container, sizeof(server_to_client_hdr) + msg.bytes);
                }
                else if (i == STDIN_FILENO)
                {
                    // s-a primit o comanda de la stdin
                    memset(buf, 0, MAX_LEN_BUF);
                    ret = read(STDIN_FILENO, buf, MAX_LEN_BUF);
                    DIE(ret == -1, STDIN_ERR);
                    if (!strncmp(buf, EXIT, strlen(EXIT)))
                    {
                        close_all_sockets(clients_container);
                        end = 1;
                        break;
                    }
                }
                else
                {
                    // instructiune de la un client tcp deja conectat
                    int _close_ = manage_command_from_client(i, clients_container, buf, &read_fds, &fdmax);
                    memset(response, 0, sizeof(server_to_client_hdr) + 1);
                    server_to_client_hdr sv_cl;
                    sv_cl.type = 6;
                    memcpy(response, &sv_cl, sizeof(server_to_client_hdr));
                    ret = send(i, response, sizeof(server_to_client_hdr) + 1, 0);
                    DIE(ret == -1, ACK_FAIL);
                    if (_close_ != -1)
                    {
                        close(_close_);
                    }
                }
            }
        }
        FD_ZERO(&tmp_fds);
        if (end == 1)
        {
            break;
        }
    }

    close(sockfd_udp);
    close(sockfd_tcp);

    return 0;
}
