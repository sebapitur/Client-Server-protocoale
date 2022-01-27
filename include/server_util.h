#ifndef _SERVER_UTIL_H_
#define _SERVER_UTIL_H_
#include "util.h"
#define MAX_CLIENTS 5
#define MAX_LEN_BUF 1551
#define NEW_CONN_DISP "New client %s connected from %s:%hu.\n"
#define DISC_DISP "Client %s disconnected.\n"
#define SEND_SUB_ERR "Failed to send data to subscriber\n"
#define CON_REF "Client %s already connected.\n"
#define USAGE_DISP "Wrong usage, the correct one is ./server <PORT>\n"
#define UDP_SOCK_ERR "Cannot open UPD socket\n"
#define TCP_BIND_ERR "Failed to open connection for TCP\n"
#define LISTEN_ERR "Listen failed\n"
#define UDP_CONN_ERR "Failed to open connection for UDP\n"
#define UDP_RECV_ERR "Failed to receive UDP message\n"
typedef struct
{
    char *topic;
    unsigned char type;
    int bytes;
    void *data;
} message;

typedef struct
{
    char topic[STRING_LEN];
    unsigned char sf;
} topic_structure;

void remove_by_id(struct LinkedList *list, char id[]);
client_structure *get_client_by_id(struct LinkedList *list, char id[]);
client_structure *get_client_by_sock(struct LinkedList *list, int sock);
void build_msg(char *buf, message *msg, int bytes);
void build_sentBuffer(char *buf, message msg, struct sockaddr *from_udp_station);
void send_data(char *data, char *topic, struct LinkedList *clients_container, int bytes);
void empty_queue(client_structure *client);
void manage_connection_request(int sockfd_tcp, struct sockaddr_in *cli_addr,
                               struct LinkedList *clients_container, fd_set *read_fds,
                               char *buf, int *fdmax);
void close_all_sockets(struct LinkedList *clients_container);
int manage_command_from_client(int i, struct LinkedList *clients_container, char *buf, fd_set *read_fds, int *fdmax);
#endif