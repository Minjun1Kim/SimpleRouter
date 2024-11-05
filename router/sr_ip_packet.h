#ifndef SR_IP_PACKET_H
#define SR_IP_PACKET_H

void sr_handleIPpacket(struct sr_instance* sr, uint8_t * packet, unsigned int len, char* interface);
void handle_icmp_packet(struct sr_instance *sr, uint8_t *packet, unsigned int len, char *interface);
void send_icmp_echo_reply(struct sr_instance *sr, uint8_t *orig_packet, unsigned int len, char *interface);
void forward_ip_packet(struct sr_instance *sr, uint8_t *packet, unsigned int len, char *interface);

struct sr_rt *lpm(struct sr_instance *sr, uint32_t ip_dst);
void send_icmp_error(struct sr_instance *sr, uint8_t *orig_packet, unsigned int orig_len, uint8_t type, uint8_t code, char *interface);

#endif