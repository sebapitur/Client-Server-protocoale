#include "include/subscriber_util.h"

void usage(char *file)
{
    fprintf(stderr, "USAGE %s <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n", file);
    exit(0);
}

int main(int argc, char *argv[])
{
    struct in_addr adr;
    void *uni_val;
    unsigned short int udp_port;
    double double_val, float_val;
    int sockfd, ret, sf, int_val;
    struct sockaddr_in serv_addr;
    char buffer[BUFLEN], *aux_buffer, command[COMMAND_LEN], response[sizeof(client_to_server_hdr) + 1], topic[STRING_LEN + 1] = {0};
    client_to_server_hdr cls_hdr;
    server_to_client_hdr *sv_cl;
    fd_set read_fds, temp_fds;
    /************************************/
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    /************************************/
    if (argc < 3)
    {
        usage(argv[0]);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_TCP, TCP_NODELAY, NULL, sizeof(int));
    DIE(sockfd < 0, TCP_SOCK_ERR);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((unsigned short int)atoi(argv[3]));
    ret = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(ret == 0, INET_ATON);
    ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(ret < 0, CONN_ERR);
    manage_id(argv, sockfd, ret, buffer, &sv_cl);
    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);

    while (1)
    {
        // se trimite mesaj la server
        temp_fds = read_fds;
        ret = select(sockfd + 1, &temp_fds, NULL, NULL, NULL);
        DIE(ret < 0, SELECT_ERR);
        int end = 0;
        for (int i = 0; i <= sockfd; i++)
        {
            if (FD_ISSET(i, &temp_fds))
            {
                if (i == 0)
                {
                    read_first_parameter(ret, buffer, command);
                    if (!strncmp(command, SUBSCRIBE, strlen(SUBSCRIBE)))
                    {
                        cls_hdr = process_subscribe(sockfd, &sf, buffer, command, &cls_hdr);
                        printf(SUB_SUCC);
                    }
                    else if (!strncmp(command, UNSUBSCRIBE, strlen(UNSUBSCRIBE)))
                    {
                        cls_hdr = process_unsubscribe(sockfd, buffer, command, &cls_hdr);
                        printf(UNSUB_SUCC);
                    }
                    else if (!strncmp(command, EXIT, strlen(EXIT)))
                    {
                        client_to_server_hdr cl_sv;
                        cl_sv.type = 3;
                        memset(buffer, 0, BUFLEN);
                        memcpy(buffer, &cl_sv, sizeof(cl_sv));
                        ret = send(sockfd, buffer, sizeof(cl_sv), 0);
                        DIE(ret == -1, DISC_SIG);
                        memset(buffer, 0, BUFLEN);
                        ret = recv(sockfd, buffer, BUFLEN, 0);
                        DIE(ret == -1, LACK_ACK);
                        sv_cl = (server_to_client_hdr *)buffer;
                        DIE(sv_cl->type != 6, LACK_ACK);
                        close(sockfd);
                        return 0;
                    }
                    else
                    {
                        printf(WRONG_COM);
                    }
                }
                else if (i == sockfd)
                {
                    memset(buffer, 0, BUFLEN);
                    ret = recv(sockfd, buffer, BUFLEN, 0);
                    DIE(ret == -1, SUB_RECV_ERR);
                    sv_cl = (server_to_client_hdr *)buffer;

                    switch (sv_cl->type)
                    {
                    case 0:
                    {
                        int_val = ntohl(*(int *)extract_server_to_client_header(buffer, sv_cl, &adr, &udp_port));
                        memset(topic, 0, STRING_LEN + 1);
                        memcpy(topic, sv_cl->topic, STRING_LEN);
                        printf(INT_DISP, inet_ntoa(adr),
                               udp_port, topic, int_val);
                        break;
                    }
                    case 1:
                    {
                        uni_val = extract_server_to_client_header(buffer, sv_cl, &adr, &udp_port);
                        unsigned short short_temp = ntohs(*(unsigned short *)uni_val);
                        float_val = (double)short_temp / 100;
                        memset(topic, 0, STRING_LEN + 1);
                        memcpy(topic, sv_cl->topic, STRING_LEN);
                        printf(SHORT_DISP, inet_ntoa(adr),
                               udp_port, topic, float_val);
                        break;
                    }
                    case 2:
                    {
                        uni_val = extract_server_to_client_header(buffer, sv_cl, &adr, &udp_port);
                        unsigned char sign = *(unsigned char *)(uni_val);
                        unsigned int temp_int = ntohl(*(unsigned int *)(uni_val + sizeof(unsigned char)));
                        unsigned char pow = *(unsigned char *)(uni_val + sizeof(unsigned char) + sizeof(unsigned int));
                        double_val = (double)(temp_int) / my_pow(pow);
                        if (sign)
                        {
                            double_val = -double_val;
                        }
                        memset(topic, 0, STRING_LEN + 1);
                        memcpy(topic, sv_cl->topic, STRING_LEN);
                        printf(FLOAT_DISP, inet_ntoa(adr),
                               udp_port, topic, (double)double_val);
                        break;
                    }
                    case 3:
                    {
                        aux_buffer = (char *)extract_server_to_client_header(buffer, sv_cl, &adr, &udp_port);
                        memset(topic, 0, STRING_LEN + 1);
                        memcpy(topic, sv_cl->topic, STRING_LEN);
                        printf(STRING_DISP, inet_ntoa(adr),
                               udp_port, topic, aux_buffer);
                        break;
                    }
                    case 4:
                    {   

                        end = 1;
                        break;
                    }
                    }
                    
                    memset(response, 0, sizeof(client_to_server_hdr) + 1);
                    cls_hdr.type = 4;
                    memcpy(response, &cls_hdr, sizeof(cls_hdr));
                    ret = send(sockfd, response, sizeof(cls_hdr) + 1, 0);
                    DIE(ret == -1, ACK_FAIL);
                    sv_cl = (server_to_client_hdr *)buffer;
                    
                }
            }
        }
        FD_ZERO(&temp_fds);
        if (end)
            break;
    }
    close(sockfd);

    return 0;
}
