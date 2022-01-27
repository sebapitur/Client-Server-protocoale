#include "include/server_util.h"
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include "include/util.h"

void remove_by_id(struct LinkedList *list, char id[])
{
    struct Node *curr = list->head;
    struct Node *prev = NULL, *temp;

    for (; curr;)
    {
        if (!strncmp(((client_structure *)curr->data)->id, id, ID_LEN))
        {
            if (!prev)
            {
                temp = list->head;
                list->head = list->head->next;
                free(temp);
            }
            else
            {
                temp = curr;
                prev->next = curr->next;
                free(temp);
            }
            break;
        }
        prev = curr;
        curr = curr->next;
    }
}

client_structure *get_client_by_id(struct LinkedList *list, char id[])
{
    struct Node *curr = list->head;
    for (; curr;)
    {
        if (!strncmp(((client_structure *)curr->data)->id, id, ID_LEN))
        {
            return (client_structure *)curr->data;
        }
        curr = curr->next;
    }
    return NULL;
}

client_structure *get_client_by_sock(struct LinkedList *list, int sock)
{
    struct Node *curr = list->head;
    for (; curr;)
    {
        if (((client_structure *)curr->data)->sock == sock)
        {
            return (client_structure *)curr->data;
        }
        curr = curr->next;
    }
    return NULL;
}

void build_msg(char *buf, message *msg, int bytes)
{
    int topic_len = strlen(buf);
    msg->topic = malloc((topic_len + 1) * sizeof(char));
    memset(msg->topic, 0, (topic_len + 1) * sizeof(char));
    memcpy(msg->topic, buf, topic_len);
    msg->type = *(unsigned char *)(buf + STRING_LEN);
    msg->data = NULL;
    switch (msg->type)
    {
    case 0:

        msg->data = malloc(sizeof(unsigned int));
        memcpy(msg->data, buf + STRING_LEN + 2, sizeof(unsigned int));
        *(unsigned int *)msg->data = ntohl(*(unsigned int *)msg->data);
        int int_val = (int)*(unsigned int *)(msg->data);

        if (*(unsigned char *)(buf + STRING_LEN + 1) == 1)
        {
            int_val = -int_val;
        }
        int_val = htonl(int_val);
        memcpy(msg->data, &int_val, sizeof(int));
        msg->bytes = sizeof(int);
        break;
    case 1:
        msg->data = malloc(sizeof(unsigned short));
        memcpy(msg->data, buf + STRING_LEN + 1, sizeof(unsigned short));
        msg->bytes = sizeof(unsigned short);
        break;
    case 2:
        msg->data = malloc(sizeof(unsigned int) + 2 * sizeof(unsigned char));
        memcpy(msg->data, buf + STRING_LEN + 1, sizeof(unsigned int) + 2 * sizeof(unsigned char));
        msg->bytes = sizeof(unsigned int) + 2 * sizeof(unsigned char);
        break;
    case 3:
        msg->data = malloc(bytes - STRING_LEN - 1);
        memset(msg->data, 0, bytes - STRING_LEN - 1);
        memcpy(msg->data, buf + STRING_LEN + 1, bytes - STRING_LEN - 1);
        msg->bytes = bytes - STRING_LEN - 1;
        break;
    default:
        break;
    }
}

void build_sentBuffer(char *buf, message msg, struct sockaddr *from_udp_station)
{
    memset(buf, 0, MAX_LEN_BUF);
    server_to_client_hdr sv_hdr;
    struct sockaddr_in udp_station_addr;
    memcpy(&udp_station_addr, from_udp_station, sizeof(*from_udp_station));
    sv_hdr.udp_ip = htonl(udp_station_addr.sin_addr.s_addr);
    sv_hdr.udp_port = (uint16_t)udp_station_addr.sin_port;
    sv_hdr.bytes = msg.bytes;
    memset(sv_hdr.topic, 0, STRING_LEN);
    memcpy(sv_hdr.topic, msg.topic, STRING_LEN);
    sv_hdr.type = msg.type;
    memcpy(buf, &sv_hdr, sizeof(sv_hdr));
    memcpy(buf + sizeof(sv_hdr), msg.data, msg.bytes);
}

void send_data(char *data, char *topic, struct LinkedList *clients_container, int bytes)
{
    struct Node *curr = clients_container->head;
    int ret;
    char buffer[MAX_LEN_BUF];
    for (; curr;)
    {
        client_structure *client = (client_structure *)curr->data;
        struct Node *curr2 = client->subscribed_topics->head;
        for (; curr2;)
        {
            int len = strlen(topic);
            if (len > STRING_LEN)
                len = STRING_LEN;
            if (!memcmp(((topic_structure *)(curr2->data))->topic, topic, len - 1))
            {
                if (client->online)
                {
                    ret = send(client->sock, data, bytes, 0);
                    DIE(ret == -1, SEND_SUB_ERR);
                    memset(buffer, 0, MAX_LEN_BUF);
                    ret = recv(client->sock, buffer, MAX_LEN_BUF, 0);
                    DIE(ret == -1, LACK_ACK);
                    client_to_server_hdr *cls;
                    cls = (client_to_server_hdr *)buffer;
                    DIE(cls->type != 4, LACK_ACK);
                }
                else if (((topic_structure *)(curr2->data))->sf)
                {
                    queue_element el;
                    el.data = malloc(bytes);
                    memcpy(el.data, data, bytes);
                    el.len = bytes;
                    add_last_improved(client->queue, &el, sizeof(el));
                }
                break;
            }
            curr2 = curr2->next;
        }
        curr = curr->next;
    }
}

void empty_queue(client_structure *client)
{
    int ret;
    char buf[BUFLEN];
    client_to_server_hdr *cl_sv;
    while (client->queue->head)
    {
        struct Node *curr = remove_first(client->queue);
        memset(buf, 0, BUFLEN);
        memcpy(buf, ((queue_element *)curr->data)->data, ((queue_element *)curr->data)->len);
        ret = send(client->sock, buf, ((queue_element *)curr->data)->len, 0);
        DIE(ret == -1, QUEUE_ERR);
        memset(buf, 0, BUFLEN);
        ret = recv(client->sock, buf, BUFLEN, 0);
        DIE(ret == -1, LACK_ACK);
        cl_sv = (client_to_server_hdr *)buf;
        DIE(cl_sv->type != 4, LACK_ACK);
    }
}

void manage_connection_request(int sockfd_tcp, struct sockaddr_in *cli_addr,
                               struct LinkedList *clients_container, fd_set *read_fds,
                               char *buf, int *fdmax)
{
    // cerere de conexiune a unui client tcp
    server_to_client_hdr sv_cl;
    client_to_server_hdr *cl_sv_pointer;
    unsigned int *clilen = malloc(sizeof(unsigned int));
    int new_sockfd, ret;
    (*clilen) = sizeof((*cli_addr));
    new_sockfd = accept(sockfd_tcp, (struct sockaddr *)cli_addr, clilen);
    DIE(new_sockfd == -1, ID_REQ_ERR_MSG);
    setsockopt(new_sockfd, SOL_TCP, TCP_NODELAY, NULL, sizeof(int));
    // adaugare client in container
    client_structure new_client;
    new_client.queue = malloc(sizeof(struct LinkedList));
    init_list(new_client.queue);
    new_client.online = 1;
    new_client.sock = new_sockfd;
    new_client.subscribed_topics = malloc(sizeof(struct LinkedList));
    init_list(new_client.subscribed_topics);
    sv_cl.type = 5;
    memcpy(buf, &sv_cl, sizeof(sv_cl));
    ret = send(new_sockfd, buf, sizeof(sv_cl), 0);
    DIE(ret == -1, ID_REQ_ERR_MSG);
    ret = recv(new_sockfd, buf, MAX_LEN_BUF, 0);
    DIE(ret == -1, ID_REP_ERR_MSG);
    cl_sv_pointer = (client_to_server_hdr *)buf;
    DIE(cl_sv_pointer->type != 2, ID_REP_ERR_MSG);
    memcpy(new_client.id, buf + sizeof(*cl_sv_pointer), ID_LEN);
    client_structure *old_client = get_client_by_id(clients_container, new_client.id);
    if (!old_client)
    {
        // new client
        printf(NEW_CONN_DISP, new_client.id, inet_ntoa(cli_addr->sin_addr), htons(cli_addr->sin_port));
        add_last_improved(clients_container, &new_client, sizeof(client_structure));
        FD_SET(new_sockfd, read_fds);
        (*fdmax)++;
    }
    else
    {
        // old client
        if (old_client->online)
        {
            memset(buf, 0, MAX_LEN_BUF);
            printf(CON_REF, old_client->id);
            sv_cl.type = 4;
            memcpy(buf, &sv_cl, sizeof(sv_cl));
            send(new_sockfd, buf, sizeof(sv_cl), 0);
            close(new_sockfd);
        }
        else
        {
            old_client->sock = new_sockfd;
            old_client->online = 1;
            printf(NEW_CONN_DISP, old_client->id, inet_ntoa(cli_addr->sin_addr), htons(cli_addr->sin_port));
            empty_queue(old_client);
            FD_SET(new_sockfd, read_fds);
            (*fdmax)++;
        }
    }
}

void close_all_sockets(struct LinkedList *clients_container)
{
    struct Node *curr = clients_container->head;
    server_to_client_hdr sv_cl;
    char buf[MAX_LEN_BUF];
    for (; curr;)
    {   
        memset(buf, 0, MAX_LEN_BUF);
        sv_cl.type = 4;
        memcpy(buf, &sv_cl, sizeof(sv_cl));
        send(((client_structure *)curr->data)->sock, buf, sizeof(sv_cl), 0);
        memset(buf, 0, MAX_LEN_BUF);
        recv(((client_structure *)curr->data)->sock, buf, MAX_LEN_BUF, 0);
        client_to_server_hdr *cls;
        cls = (client_to_server_hdr *)buf;
        DIE(cls->type != 4, LACK_ACK);
        close(((client_structure *)curr->data)->sock);
        curr = curr->next;
    }
}

int manage_command_from_client(int i, struct LinkedList *clients_container, char *buf, fd_set *read_fds, int *fdmax)
{
    memset(buf, 0, MAX_LEN_BUF);
    recv(i, buf, MAX_LEN_BUF, 0);
    client_to_server_hdr cls_hdr;
    memcpy(&cls_hdr, buf, sizeof(cls_hdr));
    client_structure *client;
    struct Node *curr, *temp, *prev = NULL;
    switch (cls_hdr.type)
    {
    case 0: // subscribe command
        client = get_client_by_sock(clients_container, i);
        curr = client->subscribed_topics->head;
        int contained = 0;
        for (; curr;)
        {
            int len = strlen(cls_hdr.topic);
            if (len > STRING_LEN)
                len = STRING_LEN;
            if (!strncmp(((topic_structure *)curr->data)->topic, cls_hdr.topic, len - 1))
            {
                ((topic_structure *)curr->data)->sf = cls_hdr.sf;
                contained = 1;
                break;
            }
            curr = curr->next;
        }
        if (!contained)
        {
            topic_structure topicStructure;
            topicStructure.sf = cls_hdr.sf;
            memset(topicStructure.topic, 0, STRING_LEN);
            memcpy(topicStructure.topic, cls_hdr.topic, STRING_LEN);
            add_last_improved(client->subscribed_topics, &topicStructure, sizeof(topic_structure));
        }
        break;
    case 1: // unsubscribe command
        client = get_client_by_sock(clients_container, i);
        curr = client->subscribed_topics->head;
        for (; curr;)
        {
            if (!strncmp(((topic_structure *)curr->data)->topic, cls_hdr.topic, STRING_LEN))
            {
                ((topic_structure *)curr->data)->sf = cls_hdr.sf;
                if (!prev)
                {
                    temp = client->subscribed_topics->head;
                    client->subscribed_topics->head = client->subscribed_topics->head->next;
                    free(temp->data);
                    free(temp);
                }
                else
                {
                    temp = curr;
                    prev->next = curr->next;
                    free(temp->data);
                    free(temp);
                }
                break;
            }
            prev = curr;
            curr = curr->next;
        }
        break;
    case 3: // exit command
        client = get_client_by_sock(clients_container, i);
        printf(DISC_DISP, client->id);
        FD_CLR(client->sock, read_fds);
        if (*fdmax == client->sock)
            (*fdmax)--;
        client->sock = -1;
        client->online = 0;
        return i;
    }
    return -1;
}
