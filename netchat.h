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
  NETCHAT_OK                    = 0,
  NETCHAT_FAILED                = -1,
  NETCHAT_TIMEOUT               = -2,
  NETCHAT_AUTH                  = -3,
  NETCHAT_SERVER_PROTECTED      = -4,
  NETCHAT_SERVER_OFFLINE        = -5,
  NETCHAT_INVALID_ARG           = -6,
  NETCHAT_INVALID_PASSWORD      = -7,
  NETCHAT_NO_CONNECTION         = -8,
  NETCHAT_INVALID_USERNAME      = -9,
  NETCHAT_INVALID_CALC_KEY      = -10,
  NETCHAT_INVALID_SESSION_TOKEN = -11,
  NETCHAT_WORK_IN_PROGRESS      = -12,
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
    char *sender;           // message sender (min. 3 chars, max. 18 chars)
    char *recipient;        // message recipient (min. 3 chars, max. 18 chars)
    char *message;          // message content
} IncomingMessage;

// Outgoing message (for ex. when sending a message)
typedef struct {
    char *recipient;        // message recipient (min. 3 chars, max. 18 chars)
    char *message;          // message content
} OutgoingMessage;

typedef void (*received_message_callback_t)(IncomingMessage message);

/*
 * Initialize the chat system
 * @param server server information
 * @param received_message_callback function to call when a message is received
*/
netchat_err_t netchat_init(struct netif *netif, ChatServer server, received_message_callback_t received_message_callback);

/*
 * Login to the chat server
*/
netchat_err_t netchat_login(char *username, char *session_token);

/*
 * Send a message
 * @param message message to send
*/
netchat_err_t netchat_send(OutgoingMessage *message);

/*
 * Destroy the chat system
*/
netchat_err_t netchat_destroy();

/*
 * Check if the chat system is connected
 * @return true if connected, false otherwise
*/
bool netchat_is_connected();

/*
 * Check if the chat system is logged in
 * @return true if logged in, false otherwise
*/
bool netchat_is_logged_in();

#ifdef __cplusplus
}
#endif

#endif
