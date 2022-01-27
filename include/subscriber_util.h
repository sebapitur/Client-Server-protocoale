#ifndef _SUBSCRIBER_UTIL_H_
#define _SUBSCRIBER_UTIL_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "util.h"
#define DISC_SIG "Unable to send disconnection signal\n"
#define SUB_RECV_ERR "Failed to receive subscription data\n"
#define BASE 10
#define INT_DISP "%s:%hu - %s - INT - %d.\n"
#define SHORT_DISP "%s:%hu - %s - SHORT_REAL - %.2f.\n"
#define FLOAT_DISP "%s:%hu - %s - FLOAT - %f.\n"
#define STRING_DISP "%s:%hu - %s - STRING - %s.\n"

void read_first_parameter(int ret, char *buffer, char *command);
client_to_server_hdr process_subscribe(int sockfd, int *sf, char *buffer, const char *command,
                                       client_to_server_hdr *cls_hdr);
client_to_server_hdr process_unsubscribe(int sockfd, char *buffer, const char *command, client_to_server_hdr *cls_hdr);
void *extract_server_to_client_header(char *buffer, const server_to_client_hdr *sv_cl, struct in_addr *adr,
                                      unsigned short *udp_port);
void manage_id(char *const *argv, int sockfd, int ret, char *buffer, server_to_client_hdr **sv_cl);
int my_pow(int power);
#endif
