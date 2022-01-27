#include "include/subscriber_util.h"
#include "include/util.h"
void read_first_parameter(int ret, char *buffer, char *command)
{
    memset(buffer, 0, BUFLEN);
    ret = read(STDIN_FILENO, buffer, BUFLEN);
    DIE(ret == -1, STDIN_ERR);
    memset(command, 0, COMMAND_LEN);
    sscanf(buffer, "%s", command);
}

client_to_server_hdr process_subscribe(int sockfd, int *sf, char *buffer, const char *command,
                                       client_to_server_hdr *cls_hdr)
{
    char response[sizeof(server_to_client_hdr) + 1] = {0};
    memset((*cls_hdr).topic, 0, STRING_LEN);
    sscanf(buffer + strlen(command), "%s", (*cls_hdr).topic);
    sscanf(buffer + strlen(command) + strlen((*cls_hdr).topic) + 2, "%d", sf);
    (*cls_hdr).sf = (unsigned char)(*sf);
    (*cls_hdr).type = (unsigned char)0;
    memset(buffer, 0, BUFLEN);
    memcpy(buffer, cls_hdr, sizeof((*cls_hdr)));
    send(sockfd, buffer, sizeof((*cls_hdr)), 0);
    recv(sockfd, response, sizeof(server_to_client_hdr) + 1, 0);
    server_to_client_hdr *scl;
    scl = (server_to_client_hdr *)response;
    DIE(scl->type != 6, LACK_ACK);
    return (*cls_hdr);
}

client_to_server_hdr process_unsubscribe(int sockfd, char *buffer, const char *command, client_to_server_hdr *cls_hdr)
{
    char response[sizeof(server_to_client_hdr) + 1] = {0};
    sscanf(buffer + strlen(command), "%s", (*cls_hdr).topic);
    (*cls_hdr).type = (unsigned char)1;
    memcpy(buffer, cls_hdr, sizeof((*cls_hdr)));
    send(sockfd, buffer, sizeof((*cls_hdr)), 0);
    recv(sockfd, response, sizeof(server_to_client_hdr) + 1, 0);
    server_to_client_hdr *scl;
    scl = (server_to_client_hdr *)response;
    DIE(scl->type != 6, LACK_ACK);
    return (*cls_hdr);
}

void *extract_server_to_client_header(char *buffer, const server_to_client_hdr *sv_cl, struct in_addr *adr,
                                      unsigned short *udp_port)
{
    void *val = malloc(sv_cl->bytes * sizeof(char));
    (*adr).s_addr = ntohl(sv_cl->udp_ip);
    (*udp_port) = ntohs((unsigned short int)sv_cl->udp_port);
    memcpy(val, buffer + sizeof(*sv_cl), sv_cl->bytes);
    return val;
}

void manage_id(char *const *argv, int sockfd, int ret, char *buffer, server_to_client_hdr **sv_cl)
{
    memset(buffer, 0, BUFLEN);
    ret = recv(sockfd, buffer, BUFLEN, 0);
    DIE(ret < 0, NEED_ID_REQ);
    (*sv_cl) = (server_to_client_hdr *)buffer;
    DIE((*sv_cl)->type != 5, NEED_ID_REQ);
    client_to_server_hdr cl_sv;
    cl_sv.type = 2;
    memset(buffer, 0, BUFLEN);
    memcpy(buffer, &cl_sv, sizeof(cl_sv));
    memcpy(buffer + sizeof(cl_sv), argv[1], ID_LEN);
    ret = send(sockfd, buffer, sizeof(cl_sv) + ID_LEN, 0);
    DIE(ret == -1, ID_REP_ERR);
}

int my_pow(int power)
{
    int res = 1;
    for (int i = 0; i < power; i++)
    {
        res *= BASE;
    }
    return res;
}
