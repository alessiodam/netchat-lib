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
bool tcp_connected = false;

ip_addr_t remote_ip = IPADDR4_INIT_BYTES(0, 0, 0, 0);


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

err_t tcp_recv_callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    if (err == ERR_OK && p != NULL)
    {
        printf("[netchat-lib] %s\n", (char *)p->payload);
        altcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
        return ERR_OK;
    }

    else if (err == ERR_OK && p == NULL)
    {
        printf("[netchat-lib] tcp: connection closed by remote\n");
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
    LWIP_UNUSED_ARG(arg);
    if (err == ERR_OK)
    {
        // Connection successfully established
        altcp_err(tpcb, tcp_error_callback);
        altcp_recv(tpcb, tcp_recv_callback);
        tcp_connected = true;
        printf("[netchat-lib] Connected to NETCHAT server!\n");
    }
    else
    {
        printf("[netchat-lib] connection error: %s\n", lwip_strerr(err));
        return err;
    }

    return ERR_OK;
}

err_t chat_init(struct netif *netif, ChatServer server, received_message_callback_t received_message_callback) {
    printf("[netchat-lib] chat_init\n");
    chatif = netif;
    tcp_connected = false;

    if (!ipaddr_aton(server.host, &remote_ip))
    {
        printf("[netchat-lib] invalid IP address\n");
        return ERR_ARG;
    }

    netchat_tcp_pcb = altcp_new(&tcp_allocator);
    err_t err = altcp_connect(netchat_tcp_pcb, &remote_ip, server.port, tcp_connect_callback);
    if (err != ERR_OK)
    {
        printf("[netchat-lib] tcp connect err\n");
        return err;
    }
    return ERR_OK;
}

netchat_err_t chat_send(OutgoingMessage *message) {
    printf("[netchat-lib] chat_send\n");
    if (!tcp_connected) {
        return ERR_FAILED;
    }
    return 0;
}

void chat_destroy() {
    printf("[netchat-lib] chat_destroy\n");
    tcp_connected = false;
}

bool chat_is_connected() {
    return tcp_connected;
}
