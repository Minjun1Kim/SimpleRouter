/*-----------------------------------------------------------------------------
 *
 * Header file for sr_vns_comm.c
 *
 *---------------------------------------------------------------------------*/

#ifndef SR_VNS_COMM_H
#define SR_VNS_COMM_H

#include <stdint.h>
#include "sr_router.h"

/*-----------------------------------------------------------------------------
 * Function Prototypes
 *---------------------------------------------------------------------------*/

/**
 * Connects to the virtual network server.
 *
 * @param sr Pointer to the router instance.
 * @param port Port number to connect to.
 * @param server Server hostname or IP address.
 * @return 0 on success, non-zero on failure.
 */
int sr_connect_to_server(struct sr_instance* sr, unsigned short port, char* server);

/**
 * Reads data from the server and handles incoming commands.
 *
 * @param sr Pointer to the router instance.
 * @return 0 on success, non-zero on failure.
 */
int sr_read_from_server(struct sr_instance* sr);

/**
 * Sends a packet (including Ethernet header) to the server to be injected
 * onto the network.
 *
 * @param sr Pointer to the router instance.
 * @param buf Pointer to the packet buffer.
 * @param len Length of the packet.
 * @param iface Name of the interface to send the packet out.
 * @return 0 on success, non-zero on failure.
 */
int sr_send_packet(struct sr_instance* sr, uint8_t* buf, unsigned int len, const char* iface);

#endif /* SR_VNS_COMM_H */
