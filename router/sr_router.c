/**********************************************************************
 * file:  sr_router.c
 * date:  Mon Feb 18 12:50:42 PST 2002
 * Contact: casado@stanford.edu
 *
 * Description:
 *
 * This file contains all the functions that interact directly
 * with the routing table, as well as the main entry method
 * for routing.
 *
 **********************************************************************/

#include <stdio.h>
#include <assert.h>


#include "sr_if.h"
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
#include "sr_arpcache.h"
#include "sr_utils.h"
#include "sr_ip_packet.h"

/*---------------------------------------------------------------------
 * Method: sr_init(void)
 * Scope:  Global
 *
 * Initialize the routing subsystem
 *
 *---------------------------------------------------------------------*/

void sr_init(struct sr_instance* sr)
{
    /* REQUIRES */
    assert(sr);

    /* Initialize cache and cache cleanup thread */
    sr_arpcache_init(&(sr->cache));

    pthread_attr_init(&(sr->attr));
    pthread_attr_setdetachstate(&(sr->attr), PTHREAD_CREATE_JOINABLE);
    pthread_attr_setscope(&(sr->attr), PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setscope(&(sr->attr), PTHREAD_SCOPE_SYSTEM);
    pthread_t thread;

    pthread_create(&thread, &(sr->attr), sr_arpcache_timeout, sr);
    
    /* Add initialization code here! */

} /* -- sr_init -- */

/*---------------------------------------------------------------------
 * Method: sr_handlepacket(uint8_t* p,char* interface)
 * Scope:  Global
 *
 * This method is called each time the router receives a packet on the
 * interface.  The packet buffer, the packet length and the receiving
 * interface are passed in as parameters. The packet is complete with
 * ethernet headers.
 *
 * Note: Both the packet buffer and the character's memory are handled
 * by sr_vns_comm.c that means do NOT delete either.  Make a copy of the
 * packet instead if you intend to keep it around beyond the scope of
 * the method call.
 *
 *---------------------------------------------------------------------*/

void sr_handlepacket(struct sr_instance* sr,
        uint8_t * packet/* lent */,
        unsigned int len,
        char* interface/* lent */)
{
  /* REQUIRES */
  assert(sr);
  assert(packet);
  assert(interface);

  printf("*** -> Received packet of length %d \n",len);

  /* fill in code here */

  /* check if the packet is large enogugh to contain an ethernet header */
  if (len < sizeof(sr_ethernet_hdr_t)) {
    fprintf(stderr, "Packet is too short\n");
    return;
  }

  /* extract the ethernet header from the packet */
  sr_ethernet_hdr_t *eth_hdr = (sr_ethernet_hdr_t *) packet;
  
  uint16_t ethertype = ntohs(eth_hdr->ether_type);

  if (ethertype == ethertype_ip) {
    sr_ip_hdr_t *ip_hdr = (sr_ip_hdr_t *) (packet + sizeof(sr_ethernet_hdr_t));
    uint8_t ip_hdr_len = ip_hdr->ip_hl * 4;

    if (len < sizeof(sr_ethernet_hdr_t) + ip_hdr_len) {
      fprintf(stderr, "Packet is too short\n");
      return;
    }

    if (ip_hdr->ip_p == ip_protocol_icmp) {
      sr_icmp_hdr_t *icmp_hdr = (sr_icmp_hdr_t *) (packet + sizeof(sr_ethernet_hdr_t) + ip_hdr_len);
      uint8_t icmp_hdr_len = sizeof(sr_icmp_hdr_t);

      if (len < sizeof(sr_ethernet_hdr_t) + ip_hdr_len + icmp_hdr_len) {
        fprintf(stderr, "Packet is too short\n");
        return;
      }

      if (icmp_hdr->icmp_type == icmp_type_echo_request) {
        sr_handle_icmp_echo_request(sr, packet, len, interface);
      } 

    } else if (ip_hdr->ip_p == ip_protocol_tcp || ip_hdr->ip_p == ip_protocol_udp) {
      sr_handle_tcp_udp_packet(sr, packet, len, interface);
    }

  } else if (ethertype == ethertype_arp) {
    
  } else {
    fprintf(stderr, "Unknown ethertype\n");
    return;
  }




}/* end sr_ForwardPacket */

