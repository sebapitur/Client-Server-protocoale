#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "LinkedList.h"

#define FOREVER 1
#define STRING_LEN 50
#define ID_REQ "ID_REQUEST"
#define ID_REQ_ERR_MSG "Unable to send ID request\n"
#define ID_REP_ERR_MSG "Unable to receive ID reply\n"
#define EXIT "exit"
#define ID_LEN 10
#define STDIN_ERR "Error receiving stdin command\n"
#define COMMAND_LEN 14
#define BUFLEN 1600
#define SUBSCRIBE "subscribe"
#define UNSUBSCRIBE "unsubscribe"
#define NEED_ID_REQ "Did not receive ID request\n"
#define ID_REP_ERR "Failed to send ID reply\n"
#define LACK_ACK "Did not receive ACK\n"
#define ACK_FAIL "Failed to send ack\n"
#define QUEUE_ERR "Failed to send enqueued message\n"
#define ATOI "atoi\n"
#define SELECT_ERR "Error at selection\n"
#define TCP_SOCK_ERR "Cannot open TCP socket\n"
#define INET_ATON "inet_aton\n"
#define CONN_ERR "Failed connection to server\n"
#define SUB_SUCC "Subscribed to topic.\n"
#define UNSUB_SUCC "Unsubscribed from topic.\n"
#define WRONG_COM "Wrong command.\n"
#define DIE(assertion, call_description)  \
    do                                    \
    {                                     \
        if (assertion)                    \
        {                                 \
            fprintf(stderr, "(%s, %d): ", \
                    __FILE__, __LINE__);  \
            perror(call_description);     \
            exit(EXIT_FAILURE);           \
        }                                 \
    } while (0)

typedef struct
{
    char id[ID_LEN];
    struct LinkedList *subscribed_topics;
    int sock;
    unsigned char online;
    struct LinkedList *queue;
} client_structure;

typedef struct
{
    unsigned char type; // 0,1, 2 sau 3, 4 = Connection refused, 5 = ID request, 6 = ACK
    unsigned int udp_ip;
    unsigned short int udp_port;
    char topic[STRING_LEN];
    int bytes; // lungimea utila a payload-ului
} server_to_client_hdr;

typedef struct
{
    unsigned char type; // 0 = subscribe, 1 = unsubscribe, 2 = ID reply, 3 = disconnect, 4 = ACK
    unsigned char sf;
    char topic[STRING_LEN];
} client_to_server_hdr;

typedef struct
{
    char *data;
    int len;
} queue_element;

#endif
