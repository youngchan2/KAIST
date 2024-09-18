/*
 * transport.c
 *
 * CS244a HW#3 (Reliable Transport)
 *
 * This file implements the STCP layer that sits between the
 * mysocket and network layers. You are required to fill in the STCP
 * functionality in this file.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "mysock.h"
#include "stcp_api.h"
#include "transport.h"
#include <arpa/inet.h>

#define WINDOW 3072
enum
{
    CSTATE_ESTABLISHED,
    CSTATE_LISTEN,
    CSTATE_SYNSENT,
    CSTATE_SYNRCVD,
    CSTATE_FIN_WAIT_1,
    CSTATE_FIN_WAIT_2,
    CSTATE_CLOSE_WAIT,
    CSTATE_LAST_ACK,
    CSTATE_CLOSING,
    CSTATE_CLOSED
}; /* obviously you should have more states */

/* this structure is global to a mysocket descriptor */
typedef struct
{
    bool_t done; /* TRUE once connection is closed */

    int connection_state; /* state of the connection (established, etc.) */
    tcp_seq initial_sequence_num;

    /* any other connection-wide global variables go here */
    tcp_seq send_seq;
    tcp_seq recv_seq;
    tcp_seq send_ack; // ack that sender receives
    tcp_seq recv_ack; // ack that receiver receives

    u_int16_t window;
} context_t;

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active)
{
    context_t *ctx;

    ctx = (context_t *)calloc(1, sizeof(context_t));
    assert(ctx);

    generate_initial_seq_num(ctx);

    /* XXX: you should send a SYN packet here if is_active, or wait for one
     * to arrive if !is_active.  after the handshake completes, unblock the
     * application with stcp_unblock_application(sd).  you may also use
     * this to communicate an error condition back to the application, e.g.
     * if connection fails; to do so, just set errno appropriately (e.g. to
     * ECONNREFUSED, etc.) before calling the function.
     */
    ctx->connection_state = CSTATE_LISTEN;
    ctx->send_seq = ctx->initial_sequence_num;
    ctx->window = WINDOW;

    if (is_active) // sender side
    {
        // 1. send SYNbit
        STCPHeader *synmsg = (STCPHeader *)malloc(sizeof(STCPHeader));
        synmsg->th_seq = htonl(ctx->send_seq);
        synmsg->th_flags = TH_SYN;
        synmsg->th_win = htons(WINDOW);
        stcp_network_send(sd, synmsg, sizeof(STCPHeader), NULL);

        ctx->connection_state = CSTATE_SYNSENT;

        STCPHeader *recvack = (STCPHeader *)malloc(sizeof(STCPHeader));
        // 2. recv SYNbit, ACKbit
        stcp_network_recv(sd, recvack, sizeof(STCPHeader));

        ctx->send_seq += 1;
        ctx->send_ack = ntohl(recvack->th_ack);
        ctx->recv_seq = ntohl(recvack->th_seq);

        ctx->connection_state = CSTATE_ESTABLISHED;

        // 3. send ACKbit
        STCPHeader *synack = (STCPHeader *)malloc(sizeof(STCPHeader));
        synack->th_ack = htonl(ctx->recv_seq + 1);
        synack->th_flags = TH_ACK;
        synack->th_seq = htonl(ctx->send_seq);
        synack->th_win = htons(WINDOW);

        stcp_network_send(sd, synack, sizeof(STCPHeader), NULL);
        ctx->recv_ack = ntohl(synack->th_ack);
        free(synack);
        free(recvack);
        free(synmsg);
    }
    else // receiver side
    {
        // 1. recv SYNbit
        STCPHeader *recvsyn = (STCPHeader *)malloc(sizeof(STCPHeader));
        stcp_network_recv(sd, recvsyn, sizeof(STCPHeader));

        ctx->connection_state = CSTATE_SYNRCVD;
        ctx->recv_seq = ntohl(recvsyn->th_seq);

        if (recvsyn->th_flags != TH_SYN)
        {
            errno = ECONNREFUSED;
            free(recvsyn);
            return;
        }

        // 2. send SYNbit, ACKbit
        STCPHeader *synack = (STCPHeader *)malloc(sizeof(STCPHeader));
        synack->th_seq = htonl(ctx->send_seq);
        synack->th_flags = TH_SYN | TH_ACK;
        synack->th_ack = htonl(ctx->recv_seq + 1);
        synack->th_win = htons(WINDOW);
        stcp_network_send(sd, synack, sizeof(STCPHeader), NULL);

        ctx->send_seq += 1;
        ctx->recv_ack = ntohl(synack->th_ack);

        // 3. recv ACKbit
        STCPHeader *recvack = (STCPHeader *)malloc(sizeof(STCPHeader));
        stcp_network_recv(sd, recvack, sizeof(STCPHeader));

        ctx->send_ack = ntohl(recvack->th_ack);
        ctx->connection_state = CSTATE_ESTABLISHED;
        free(recvack);
        free(synack);
        free(recvsyn);
    }

    stcp_unblock_application(sd);

    control_loop(sd, ctx);

    /* do any cleanup here */
    free(ctx);
}

/* generate initial sequence number for an STCP connection */
static void generate_initial_seq_num(context_t *ctx)
{
    assert(ctx);
    ctx->initial_sequence_num = 1;
}

/* control_loop() is the main STCP loop; it repeatedly waits for one of the
 * following to happen:
 *   - incoming data from the peer
 *   - new data from the application (via mywrite())
 *   - the socket to be closed (via myclose())
 *   - a timeout
 */
static void control_loop(mysocket_t sd, context_t *ctx)
{
    assert(ctx);

    char *send_seg = (char *)malloc(sizeof(STCPHeader) + STCP_MSS);
    char *recv_seg = (char *)malloc(sizeof(STCPHeader) + STCP_MSS);
    STCPHeader *send_header = (STCPHeader *)send_seg;
    STCPHeader *recv_header = (STCPHeader *)recv_seg;
    char *send_payload = send_seg + sizeof(STCPHeader);
    char *recv_payload = recv_seg + sizeof(STCPHeader);

    while (!ctx->done)
    {
        unsigned int event;
        /* see stcp_api.h or stcp_api.c for details of this function */
        /* XXX: you will need to change some of these arguments! */

        event = stcp_wait_for_event(sd, ANY_EVENT, NULL);

        memset(send_seg, 0, STCP_MSS + sizeof(STCPHeader));
        memset(recv_seg, 0, STCP_MSS + sizeof(STCPHeader));
        send_header->th_win = htons(WINDOW);
        /* check whether it was the network, app, or a close request */
        if (event & APP_DATA)
        {
            /* the application has requested that data be sent */
            /* see stcp_app_recv() */
            if (ctx->window > 0)
            {
                size_t payload_size = stcp_app_recv(sd, send_payload, ctx->window < STCP_MSS ? ctx->window : STCP_MSS);

                send_header->th_seq = htonl(ctx->send_seq);
                send_header->th_ack = htonl(ctx->recv_ack);
                send_header->th_flags |= TH_ACK;
                stcp_network_send(sd, send_seg, sizeof(STCPHeader) + payload_size, NULL);

                ctx->send_seq += payload_size;
                ctx->window -= payload_size;
            }
        }
        if (event & NETWORK_DATA)
        {
            size_t packet_size = stcp_network_recv(sd, recv_seg, sizeof(STCPHeader) + STCP_MSS);

            tcp_seq new_ack = ntohl(recv_header->th_ack);
            tcp_seq new_seq = ntohl(recv_header->th_seq);

            if (recv_header->th_flags == TH_ACK)
            {
                // transport -> application
                if (packet_size - sizeof(STCPHeader) > 0)
                {
                    size_t payload_size = packet_size - sizeof(STCPHeader);
                    stcp_app_send(sd, recv_payload, payload_size);

                    send_header->th_ack = htonl(new_seq + payload_size);
                    send_header->th_seq = htonl(ctx->send_seq);
                    send_header->th_flags = TH_ACK;
                    stcp_network_send(sd, send_header, sizeof(STCPHeader), NULL);
                    ctx->recv_ack += payload_size;
                }
                ctx->window += new_ack - ctx->send_ack;
                ctx->window = ctx->window > WINDOW ? WINDOW : ctx->window;
                ctx->send_ack = new_ack;
                if (ctx->connection_state == CSTATE_LAST_ACK)
                {
                    if (new_ack == ctx->send_seq)
                    {
                        ctx->done = TRUE;
                        ctx->connection_state = CSTATE_CLOSED;
                        continue;
                    }
                }
                if (ctx->connection_state == CSTATE_FIN_WAIT_1 && ctx->send_seq == new_ack)
                    ctx->connection_state = CSTATE_FIN_WAIT_2;
                if (ctx->connection_state == CSTATE_CLOSING && ctx->send_seq == new_ack)
                {
                    ctx->connection_state = CSTATE_CLOSED;
                    ctx->done = TRUE;
                }
            }

            else if (recv_header->th_flags & TH_FIN)
            {
                stcp_fin_received(sd);

                if (ctx->connection_state == CSTATE_ESTABLISHED)
                    ctx->connection_state = CSTATE_CLOSE_WAIT;
                if (ctx->connection_state == CSTATE_FIN_WAIT_1)
                {
                    if (ctx->send_seq == new_ack && (recv_header->th_flags & TH_ACK))
                    {
                        ctx->connection_state = CSTATE_CLOSED;
                        ctx->done = TRUE;
                    }
                    else
                        ctx->connection_state = CSTATE_CLOSING;
                }
                if (ctx->connection_state == CSTATE_FIN_WAIT_2)
                {
                    ctx->connection_state = CSTATE_CLOSED;
                    ctx->done = TRUE;
                }

                send_header->th_ack = htonl(new_seq + 1);
                send_header->th_seq = htonl(ctx->send_seq);
                send_header->th_flags = TH_ACK;
                stcp_network_send(sd, send_header, sizeof(STCPHeader), NULL);

                ctx->recv_ack += 1;
            }
        }

        if (event & APP_CLOSE_REQUESTED)
        {
            if (ctx->connection_state == CSTATE_ESTABLISHED)
                ctx->connection_state = CSTATE_FIN_WAIT_1;
            if (ctx->connection_state == CSTATE_CLOSE_WAIT)
                ctx->connection_state = CSTATE_LAST_ACK;

            send_header->th_seq = htonl(ctx->send_seq);
            send_header->th_ack = htonl(ctx->recv_ack);
            send_header->th_flags = (TH_FIN | TH_ACK);
            stcp_network_send(sd, send_header, sizeof(STCPHeader), NULL);

            ctx->send_seq += 1;
        }
        /* etc. */
    }
    free(send_seg);
    free(recv_seg);
}

/**********************************************************************/
/* our_dprintf
 *
 * Send a formatted message to stdout.
 *
 * format               A printf-style format string.
 *
 * This function is equivalent to a printf, but may be
 * changed to log errors to a file if desired.
 *
 * Calls to this function are generated by the dprintf amd
 * dperror macros in transport.h
 */
void our_dprintf(const char *format, ...)
{
    va_list argptr;
    char buffer[1024];

    assert(format);
    va_start(argptr, format);
    vsnprintf(buffer, sizeof(buffer), format, argptr);
    va_end(argptr);
    fputs(buffer, stdout);
    fflush(stdout);
}
