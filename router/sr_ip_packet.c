#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include "sr_if.h"
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
#include "sr_arpcache.h"
#include "sr_utils.h"
#include "sr_ip_packet.h"
#include "sr_vns_comm.h"

void sr_handleIPpacket(struct sr_instance* sr,
        uint8_t * packet,
        unsigned int len,
        char* interface) {

  /* check if the packet is large enough for IP header */
  if (len < sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t)) {
    fprintf(stderr, "Packet is too short\n");
    return;
  }

  /* extract the IP header from the packet */
  sr_ip_hdr_t *ip_hdr = (sr_ip_hdr_t *) (packet + sizeof(sr_ethernet_hdr_t));

  /* validate IP header */
  if (ip_hdr->ip_v != 4) {
    fprintf(stderr, "Invalid IP version\n");
    return;
  }

  if (ip_hdr->ip_hl < 5) {
    fprintf(stderr, "Invalid IP header length\n");
    return;
  }

  /* validate IP checksum */
  uint16_t checksum = ip_hdr->ip_sum;
  ip_hdr->ip_sum = 0;

  uint16_t calculated_checksum = cksum(ip_hdr, sizeof(sr_ip_hdr_t));

  if (checksum != calculated_checksum) {
    fprintf(stderr, "Invalid IP checksum\n");
    return;
  }

  ip_hdr->ip_sum = checksum;

  struct sr_if *iface = sr->if_list;
  int is_for_router = 0;

  while (iface) {
    if (iface->ip == ip_hdr->ip_dst) {
      is_for_router = 1;
      break;
    }
    iface = iface->next;
  }

  if (is_for_router) {
    /* packet is destined for the router */
    if (ip_hdr->ip_p == ip_protocol_icmp) {
      /* handle ICMP packet */
      handle_icmp_packet(sr, packet, len, interface);
    } else if (ip_hdr->ip_p == ip_protocol_tcp || ip_hdr->ip_p == ip_protocol_udp) {
      send_icmp_error(sr, packet, len, 3, 3, interface); /* Port unreachable */
    } else {
      fprintf(stderr, "Unknown IP protocol\n");
      return;
    }
  } else {
    /* packet is not destined for the router */
    forward_ip_packet(sr, packet, len, interface);

  }
}


/* if ICMP echo request, send ICMP echo reply */
void handle_icmp_packet(struct sr_instance *sr,
                        uint8_t *packet,
                        unsigned int len,
                        char *interface)
{
    /* Extract IP header */
    sr_ip_hdr_t *ip_hdr = (sr_ip_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t));
    unsigned int ip_header_len = sizeof(sr_ip_hdr_t);

    /* Ensure packet is long enough for ICMP header */
    if (len < sizeof(sr_ethernet_hdr_t) + ip_header_len + sizeof(sr_icmp_hdr_t)) {
        fprintf(stderr, "Failed to process ICMP header, insufficient length\n");
        return;
    }

    /* Extract ICMP header */
    sr_icmp_hdr_t *icmp_hdr = (sr_icmp_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t) + ip_header_len);

    /* Validate ICMP checksum */
    uint16_t icmp_len = ntohs(ip_hdr->ip_len) - ip_header_len;
    uint16_t received_icmp_cksum = icmp_hdr->icmp_sum;
    icmp_hdr->icmp_sum = 0;
    uint16_t computed_icmp_cksum = cksum(icmp_hdr, icmp_len);
    if (received_icmp_cksum != computed_icmp_cksum) {
        fprintf(stderr, "Invalid ICMP checksum\n");
        return;
    }
    icmp_hdr->icmp_sum = received_icmp_cksum; /* Restore original checksum */

    if (icmp_hdr->icmp_type == 8 && icmp_hdr->icmp_code == 0) {
        /* ICMP Echo Request - send Echo Reply */
        send_icmp_echo_reply(sr, packet, len, interface);
    }
}


void send_icmp_echo_reply(struct sr_instance *sr,
                          uint8_t *orig_packet,
                          unsigned int orig_len,
                          char *interface)
{
    /* Extract original Ethernet and IP headers */
    sr_ethernet_hdr_t *eth_hdr_orig = (sr_ethernet_hdr_t *)orig_packet;
    sr_ip_hdr_t *ip_hdr_orig = (sr_ip_hdr_t *)(orig_packet + sizeof(sr_ethernet_hdr_t));

    /* Determine lengths */
    unsigned int ip_header_len = ip_hdr_orig->ip_hl * 4;
    unsigned int icmp_len = ntohs(ip_hdr_orig->ip_len) - ip_header_len;
    unsigned int new_packet_len = sizeof(sr_ethernet_hdr_t) + ip_header_len + icmp_len;

    /* Allocate space for new packet */
    uint8_t *packet = malloc(new_packet_len);
    if (!packet) {
        fprintf(stderr, "Failed to allocate memory for ICMP Echo Reply packet\n");
        return;
    }

    /* Construct Ethernet header */
    sr_ethernet_hdr_t *eth_hdr_new = (sr_ethernet_hdr_t *)packet;
    memcpy(eth_hdr_new->ether_dhost, eth_hdr_orig->ether_shost, ETHER_ADDR_LEN);
    memcpy(eth_hdr_new->ether_shost, eth_hdr_orig->ether_dhost, ETHER_ADDR_LEN);
    eth_hdr_new->ether_type = eth_hdr_orig->ether_type;

    /* Construct IP header */
    sr_ip_hdr_t *ip_hdr_new = (sr_ip_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t));
    memcpy(ip_hdr_new, ip_hdr_orig, ip_header_len);
    ip_hdr_new->ip_src = ip_hdr_orig->ip_dst;
    ip_hdr_new->ip_dst = ip_hdr_orig->ip_src;
    ip_hdr_new->ip_ttl = 64;
    ip_hdr_new->ip_sum = 0;
    ip_hdr_new->ip_sum = cksum(ip_hdr_new, ip_header_len);

    /* Construct ICMP header and payload */
    sr_icmp_hdr_t *icmp_hdr_new = (sr_icmp_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t) + ip_header_len);
    memcpy(icmp_hdr_new, orig_packet + sizeof(sr_ethernet_hdr_t) + ip_header_len, icmp_len);
    icmp_hdr_new->icmp_type = 0; /* Echo Reply */
    icmp_hdr_new->icmp_code = 0;
    icmp_hdr_new->icmp_sum = 0;
    icmp_hdr_new->icmp_sum = cksum(icmp_hdr_new, icmp_len);

    /* Send the packet */
    sr_send_packet(sr, packet, new_packet_len, interface);

    /* Free allocated memory */
    free(packet);
}

void forward_ip_packet(struct sr_instance *sr,
                       uint8_t *packet,
                       unsigned int len,
                       char *interface)
{
    /* Extract Ethernet and IP headers */
    sr_ethernet_hdr_t *eth_hdr = (sr_ethernet_hdr_t *)packet;
    sr_ip_hdr_t *ip_hdr = (sr_ip_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t));

    /* Check TTL */
    if (ip_hdr->ip_ttl <= 1) {
        /* Send ICMP Time Exceeded */
        send_icmp_error(sr, packet, len, 11, 0, interface); /* Type 11, Code 0 */
        return;
    }

    /* Decrement TTL and recompute IP checksum */
    ip_hdr->ip_ttl--;
    ip_hdr->ip_sum = 0;
    ip_hdr->ip_sum = cksum(ip_hdr, sizeof(sr_ip_hdr_t));

    /* Perform Longest Prefix Match */
    struct sr_rt *rt_entry = lpm(sr, ip_hdr->ip_dst);

    if (!rt_entry) {
        /* Send ICMP Destination Net Unreachable */
        send_icmp_error(sr, packet, len, 3, 0, interface); /* Type 3, Code 0 */
        return;
    }

    /* Get the next-hop IP and outgoing interface */
    uint32_t next_hop_ip = rt_entry->gw.s_addr;
    struct sr_if *out_iface = sr_get_interface(sr, rt_entry->interface);

    /* Check ARP cache for next-hop MAC address */
    struct sr_arpentry *arp_entry = sr_arpcache_lookup(&sr->cache, next_hop_ip);

    if (arp_entry) {
        /* Update Ethernet header */
        memcpy(eth_hdr->ether_shost, out_iface->addr, ETHER_ADDR_LEN);
        memcpy(eth_hdr->ether_dhost, arp_entry->mac, ETHER_ADDR_LEN);

        /* Send packet */
        sr_send_packet(sr, packet, len, out_iface->name);

        /* Free ARP entry */
        free(arp_entry);
    } else {
        /* Queue the packet & rely on the function to send ARP request  */
        sr_arpcache_queuereq(&sr->cache, next_hop_ip, packet, len, out_iface->name);
    }
}


struct sr_rt *lpm(struct sr_instance *sr, uint32_t ip_dst)
{
    struct sr_rt *rt_walker = sr->routing_table;
    struct sr_rt *lpm_entry = NULL;
    uint32_t longest_mask = 0;

    while (rt_walker) {
        if ((rt_walker->dest.s_addr & rt_walker->mask.s_addr) == (ip_dst & rt_walker->mask.s_addr)) {
            if (rt_walker->mask.s_addr >= longest_mask) {
                longest_mask = rt_walker->mask.s_addr;
                lpm_entry = rt_walker;
            }
        }
        rt_walker = rt_walker->next;
    }
    return lpm_entry;
}


void send_icmp_error(struct sr_instance *sr,
                     uint8_t *orig_packet,
                     unsigned int orig_len,
                     uint8_t type,
                     uint8_t code,
                     char *interface)
{
    /* Extract original IP header */
    sr_ip_hdr_t *ip_hdr_orig = (sr_ip_hdr_t *)(orig_packet + sizeof(sr_ethernet_hdr_t));

    /* Create new packet */
    unsigned int len = sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t) + sizeof(sr_icmp_t3_hdr_t);
    uint8_t *packet = malloc(len);

    sr_ethernet_hdr_t *eth_hdr_orig = (sr_ethernet_hdr_t *)orig_packet;
    /* Ethernet header */
    sr_ethernet_hdr_t *eth_hdr_new = (sr_ethernet_hdr_t *)packet;
    /* IP header */
    sr_ip_hdr_t *ip_hdr_new = (sr_ip_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t));
    /* ICMP header */
    sr_icmp_t3_hdr_t *icmp_hdr_new = (sr_icmp_t3_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t));

    /* Set source IP to the IP of the interface where the packet came in */
    struct sr_rt *rt_entry = lpm(sr, ip_hdr_orig->ip_src);
    struct sr_if *iface = sr_get_interface(sr, rt_entry->interface);
    if (!iface) {
        fprintf(stderr, "Interface not found when sending ICMP error\n");
        free(packet);
        return;
    }
    /* Set up Ethernet header */
    memcpy(eth_hdr_new->ether_shost, iface->addr, ETHER_ADDR_LEN);
    /* Destination MAC address will be set after ARP resolution */
    eth_hdr_new->ether_type = htons(ethertype_ip);

    /* Set up Ethernet header */

    /* Set up IP header */
    ip_hdr_new->ip_v = 4;
    ip_hdr_new->ip_hl = ip_hdr_orig->ip_hl;
    ip_hdr_new->ip_tos = 0;
    ip_hdr_new->ip_len = htons(sizeof(sr_ip_hdr_t) + sizeof(sr_icmp_t3_hdr_t));
    ip_hdr_new->ip_id = htons(0);
    ip_hdr_new->ip_off = htons(IP_DF);
    ip_hdr_new->ip_ttl = 64;
    ip_hdr_new->ip_p = ip_protocol_icmp;
    ip_hdr_new->ip_src = iface->ip;
    ip_hdr_new->ip_dst = ip_hdr_orig->ip_src;

    /* Set up ICMP header */
    icmp_hdr_new->icmp_type = type;
    icmp_hdr_new->icmp_code = code;
    icmp_hdr_new->unused = 0;
    icmp_hdr_new->next_mtu = 0;
    memcpy(icmp_hdr_new->data, ip_hdr_orig, ICMP_DATA_SIZE);

    /* Compute ICMP checksum */
    icmp_hdr_new->icmp_sum = 0;
    icmp_hdr_new->icmp_sum = cksum(icmp_hdr_new, sizeof(sr_icmp_t3_hdr_t));

    /* Compute IP checksum */
    ip_hdr_new->ip_sum = 0;
    ip_hdr_new->ip_sum = cksum(ip_hdr_new, sizeof(sr_ip_hdr_t));

    /* Set Ethernet addresses */
    memcpy(eth_hdr_new->ether_dhost, eth_hdr_orig->ether_shost, ETHER_ADDR_LEN);
    
    /* Send packet */
    sr_send_packet(sr, packet, len, iface->name);
}
