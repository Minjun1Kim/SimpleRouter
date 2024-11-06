#ifndef SR_IP_PACKET_H
#define SR_IP_PACKET_H

/**
 * Handles incoming IP packets received by the router.
 *
 * @param sr Pointer to the router instance.
 * @param packet Pointer to the incoming packet buffer.
 * @param len Length of the incoming packet.
 * @param interface Name of the interface on which the packet was received.
 */
void sr_handleIPpacket(struct sr_instance* sr, uint8_t * packet, unsigned int len, char* interface);

/**
 * Processes ICMP packets destined for the router.
 *
 * @param sr Pointer to the router instance.
 * @param packet Pointer to the incoming packet buffer.
 * @param len Length of the incoming packet.
 * @param interface Name of the interface on which the packet was received.
 */
void handle_icmp_packet(struct sr_instance *sr, uint8_t *packet, unsigned int len, char *interface);

/**
 * Generates and sends an ICMP Echo Reply in response to an Echo Request.
 *
 * @param sr Pointer to the router instance.
 * @param orig_packet Pointer to the original Echo Request packet buffer.
 * @param len Length of the original packet.
 * @param interface Name of the interface on which the original packet was received.
 */
void send_icmp_echo_reply(struct sr_instance *sr, uint8_t *orig_packet, unsigned int len, char *interface);

/**
 * Forwards IP packets to the next hop based on the routing table.
 *
 * @param sr Pointer to the router instance.
 * @param packet Pointer to the packet buffer (packet will be modified).
 * @param len Length of the packet.
 * @param interface Name of the interface on which the packet was received.
 */
void forward_ip_packet(struct sr_instance *sr, uint8_t *packet, unsigned int len, char *interface);

/**
 * Performs Longest Prefix Match to find the best routing table entry for a destination IP.
 *
 * @param sr Pointer to the router instance.
 * @param ip_dst Destination IP address (in network byte order).
 * @return Pointer to the best matching routing table entry, or NULL if no match is found.
 */
struct sr_rt *lpm(struct sr_instance *sr, uint32_t ip_dst);

/**
 * Constructs and sends an ICMP error message in response to a packet that cannot be forwarded.
 *
 * @param sr Pointer to the router instance.
 * @param orig_packet Pointer to the original packet that caused the error.
 * @param orig_len Length of the original packet.
 * @param type ICMP error type.
 * @param code ICMP error code.
 * @param interface Name of the interface on which the original packet was received.
 */
void send_icmp_error(struct sr_instance *sr, uint8_t *orig_packet, unsigned int orig_len, uint8_t type, uint8_t code, char *interface);

#endif