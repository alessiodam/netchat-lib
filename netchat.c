/*
 *--------------------------------------
 * Program Name: NETCHAT Chat system
 * Author: TKB Studios
 * License: Apache License 2.0
 * Description: Allows the user to communicate with a NETCHAT server
 *--------------------------------------
*/

#include "netchat.h"

struct netif *chatif = NULL;
struct altcp_pcb *netchat_tcp_pcb = NULL;
altcp_allocator_t tcp_allocator = {altcp_tcp_alloc, NULL};

ip_addr_t remote_ip = IPADDR4_INIT_BYTES(0, 0, 0, 0);

bool tcp_connected = false;
bool logged_in = false;

received_message_callback_t received_message_callback = NULL;

void tcp_error_callback(void *arg, err_t err)
{
    struct altcp_pcb *pcb = (struct altcp_pcb *)arg;

    if (err == ERR_OK)
    {
        return;
    }

    if (err == ERR_ABRT)
    {
        printf("[netchat-lib] tcp: connection aborted\n");
    }
    else if (err == ERR_RST)
    {
        printf("[netchat-lib] tcp: connection reset by peer\n");
    }
    else
    {
        printf("[netchat-lib] tcp: error - %s\n", lwip_strerr(err));
    }

    if (pcb != NULL)
    {
        altcp_arg(pcb, NULL);
        altcp_sent(pcb, NULL);
        altcp_recv(pcb, NULL);
        altcp_err(pcb, NULL);
        altcp_close(pcb);
        tcp_connected = false;
    }
}

netchat_err_t handle_received_tcp_data(char *data)
{
    if (data == NULL) {
        return NETCHAT_INVALID_ARG;
    }

    if (strcmp(data, "AUTH_SUCCESS\n") == 0) {
        printf("[netchat-lib] logged in successfully\n");
        logged_in = true;
        return NETCHAT_OK;
    }

    if (strcmp(data, "ALREADY_AUTHENTICATED\n") == 0) {
        printf("[netchat-lib] already logged in\n");
        return NETCHAT_OK;
    }

    // if the message has the structure of an IncomingMessage, parse it.
    // timestamp: 0, sender: 1, recipient: 2, message: 3
    // for ex. "0:sender:recipient:message"
    char *timestamp = strtok(data, ":");
    char *sender = strtok(NULL, ":");
    char *recipient = strtok(NULL, ":");
    char *message = strtok(NULL, ":");
    if (timestamp != NULL && sender != NULL && recipient != NULL && message != NULL) {
        IncomingMessage incoming = {
            .timestamp = atoi(timestamp),
            .sender = sender,
            .recipient = recipient,
            .message = message
        };

        if (received_message_callback != NULL) {
            printf("[netchat-lib] triggering callback\n");
            received_message_callback(incoming);
        }
    }
    return NETCHAT_OK;
}

err_t tcp_recv_callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    if (err == ERR_OK && p != NULL)
    {
        handle_received_tcp_data((char *)p->payload);
        altcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
        return ERR_OK;
    }

    else if (err == ERR_OK && p == NULL)
    {
        altcp_close(tpcb);
        tcp_connected = false;
        return ERR_OK;
    }
    else
    {
        printf("[netchat-lib] tcp: receive error - %s\n", lwip_strerr(err));
        return err;
    }
}

err_t tcp_connect_callback(void *arg, struct altcp_pcb *tpcb, err_t err)
{
    printf("[netchat-lib] tcp_connect_callback\n");
    LWIP_UNUSED_ARG(arg);
    if (err == ERR_OK)
    {
        printf("[netchat-lib] Connected to NETCHAT server!\n");
        altcp_err(tpcb, tcp_error_callback);
        altcp_recv(tpcb, tcp_recv_callback);
        tcp_connected = true;
        printf("[netchat-lib] Connect phase ended\n");
    }
    else
    {
        printf("[netchat-lib] connection error: %s\n", lwip_strerr(err));
        return err;
    }

    return ERR_OK;
}

netchat_err_t netchat_init(struct netif *netif, ChatServer server, received_message_callback_t rcv_msg_callback)
{
    printf("[netchat-lib] netchat_init\n");
    chatif = netif;
    tcp_connected = false;

    if (!ipaddr_aton(server.host, &remote_ip))
    {
        printf("[netchat-lib] invalid IP address\n");
        return NETCHAT_INVALID_ARG;
    }

    netchat_tcp_pcb = altcp_new(&tcp_allocator);
    err_t err = altcp_connect(netchat_tcp_pcb, &remote_ip, server.port, tcp_connect_callback);
    if (err != ERR_OK)
    {
        printf("[netchat-lib] tcp connect err\n");
        return err;
    } else {
        printf("[netchat-lib] tcp connect ok\n");
    }
    received_message_callback = rcv_msg_callback;
    return ERR_OK;
}

netchat_err_t netchat_login(char *username, char *session_token)
{
    printf("[netchat-lib] netchat_login\n");
    if (strlen(username) < 3 || strlen(username) > 18) {
        printf("[netchat-lib] Invalid username\n");
        return NETCHAT_INVALID_USERNAME;
    }
    if (strlen(session_token) != 256) {
        printf("[netchat-lib] Invalid session token\n");
        return NETCHAT_INVALID_SESSION_TOKEN;
    }

    size_t len = strlen("AUTH:") + strlen(username) + strlen(":") + strlen(session_token) + strlen("\n") + 1;
    char *connect_str = (char *)malloc(len);

    if (connect_str == NULL) {
        printf("[netchat-lib] malloc failed\n");
        return -1;
    }

    snprintf(connect_str, len, "AUTH:%s:%s\n", username, session_token);
    altcp_write(netchat_tcp_pcb, connect_str, strlen(connect_str), 0);
    free(connect_str);
    return NETCHAT_OK;
}

netchat_err_t netchat_send(OutgoingMessage *message)
{
    printf("[netchat-lib] netchat_send\n");
    return NETCHAT_WORK_IN_PROGRESS;
    if (!tcp_connected || chatif == NULL || netchat_tcp_pcb == NULL || !logged_in) {
        return NETCHAT_FAILED;
    }
    if (strlen(message->recipient) < 3 || strlen(message->recipient) > 18) {
        printf("[netchat-lib] Invalid username\n");
        return NETCHAT_INVALID_USERNAME;
    }
    
    size_t len = strlen(message->recipient) + strlen(":") + strlen(message->message) + strlen("\n") + 1;
    char *send_str = (char *)malloc(len);

    if (send_str == NULL) {
        printf("[netchat-lib] malloc failed\n");
        return -1;
    }

    snprintf(send_str, len, "%s:%s\n", message->recipient, message->message);
    altcp_write(netchat_tcp_pcb, send_str, strlen(send_str), 0);
    free(send_str);
    return NETCHAT_OK;
}

netchat_err_t netchat_destroy()
{
    printf("[netchat-lib] netchat_destroy\n");
    return NETCHAT_WORK_IN_PROGRESS;
    tcp_connected = false;
}

bool netchat_is_connected()
{
    return tcp_connected;
}

bool netchat_is_logged_in()
{
    return logged_in;
}
