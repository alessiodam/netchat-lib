/*
 *--------------------------------------
 * Program Name: NETCHAT Chat system
 * Author: TKB Studios
 * License: Apache License 2.0
 * Description: Allows the user to communicate with a NETCHAT server
 *--------------------------------------
*/

#ifndef NETCHAT_H
#define NETCHAT_H

#include <stdbool.h>
#include <time.h>
#include "lwip/altcp.h"
#include "lwip/altcp_tcp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ERR_OK                = 0,
  ERR_FAILED            = -1,
  ERR_TIMEOUT           = -2,
  ERR_AUTH              = -3,
  ERR_SERVER_PROTECTED  = -4,
} netchat_enum_err_t;
typedef s8_t netchat_err_t;

// NETCHAT Server information
typedef struct {
    char *host;             // domain or IP address
    int port;               // server port
    bool online_mode;       // online mode (true or false)
    char *server_password;  // server password
} ChatServer;

// Incoming message (for ex. when receiving a message)
typedef struct {
    time_t timestamp;       // message timestamp (UNIX time)
    char *recipient;        // message recipient (min. 3 chars, max. 18 chars)
    char *sender;           // message sender (min. 3 chars, max. 18 chars)
    char *message;          // message content
} IncomingMessage;

// Outgoing message (for ex. when sending a message)
typedef struct {
    char *recipient;        // message recipient (min. 3 chars, max. 18 chars)
    char *message;          // message content
} OutgoingMessage;

typedef void (received_message_callback_t)(IncomingMessage message);

/*
 * Initialize the chat system
 * @param server server information
 * @param received_message_callback function to call when a message is received
*/
err_t chat_init(struct netif *netif, ChatServer server, received_message_callback_t received_message_callback);

/*
 * Send a message
 * @param message message to send
*/
netchat_err_t chat_send(OutgoingMessage *message);

/*
 * Destroy the chat system
*/
void chat_destroy();

/*
 * Check if the chat system is connected
 * @return true if connected, false otherwise
*/
bool chat_is_connected();

#ifdef __cplusplus
}
#endif

#endif
