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
#include <stdlib.h>
#include <assert.h>


#include "sr_if.h"
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
#include "sr_arpcache.h"
#include "sr_utils.h"
#include "sr_ip_packet.h"

static void sr_handleARPpacket(struct sr_instance* sr, uint8_t * packet, 
                               unsigned int len, char* interface);
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
    sr_handleIPpacket(sr, packet, len, interface);
  } else if (ethertype == ethertype_arp) {
    sr_handleARPpacket(sr, packet, len, interface);
  } else {
    fprintf(stderr, "Unknown ethertype\n");
    return;
  }




}/* end sr_ForwardPacket */


static void sr_handleARPpacket(struct sr_instance* sr, uint8_t * packet, 
                               unsigned int len, char* interface)
{
  sr_arp_hdr_t *arp_hdr = (sr_arp_hdr_t *) (packet + sizeof(sr_ethernet_hdr_t));

  if (len < sizeof(sr_ethernet_hdr_t) + sizeof(sr_arp_hdr_t)) {
    fprintf(stderr, "Packet is too short\n");
    return;
  }

  /* Check if it's targetting one of the router's ip addresses. */
  struct sr_if *sr_if = sr_get_interface(sr, interface);
  if (sr_if == 0 || sr_if->ip != arp_hdr->ar_tip) {
    return;
  }

  if (ntohs(arp_hdr->ar_op) == arp_op_request) {
    uint8_t* arp_reply_packet = (uint8_t*)malloc(len);
    memcpy(arp_reply_packet, packet, len);

    sr_ethernet_hdr_t *arp_reply_eth_hdr = (sr_ethernet_hdr_t *) arp_reply_packet;
    memcpy(arp_reply_eth_hdr->ether_dhost, arp_reply_eth_hdr->ether_shost, ETHER_ADDR_LEN);
    memcpy(arp_reply_eth_hdr->ether_shost, sr_if->addr, ETHER_ADDR_LEN);
      
    sr_arp_hdr_t *arp_reply_hdr = (sr_arp_hdr_t *) (arp_reply_packet + sizeof(sr_ethernet_hdr_t));
    arp_reply_hdr->ar_op = htons(arp_op_reply);
    arp_reply_hdr->ar_tip = arp_reply_hdr->ar_sip;
    arp_reply_hdr->ar_sip = sr_if->ip;
    memcpy(arp_reply_hdr->ar_tha, arp_reply_hdr->ar_sha, ETHER_ADDR_LEN);
    memcpy(arp_reply_hdr->ar_sha, sr_if->addr, ETHER_ADDR_LEN);

    sr_send_packet(sr, arp_reply_packet, len, interface);
    free(arp_reply_packet);
  } else if (ntohs(arp_hdr->ar_op) == arp_op_reply) {
    struct sr_arpreq *cached_req = sr_arpcache_insert(&(sr->cache), arp_hdr->ar_sha, arp_hdr->ar_sip);

    if (cached_req) {
      struct sr_packet *waiting_pkts;
      for (waiting_pkts = cached_req->packets; waiting_pkts != NULL; waiting_pkts = waiting_pkts->next) {
        uint8_t *waiting_pkt = waiting_pkts->buf;
        sr_ethernet_hdr_t *pkt_eth_hdr = (sr_ethernet_hdr_t *) waiting_pkt;
        memcpy(pkt_eth_hdr->ether_dhost, arp_hdr->ar_sha, ETHER_ADDR_LEN);
        memcpy(pkt_eth_hdr->ether_shost, sr_if->addr, ETHER_ADDR_LEN);
          
        sr_send_packet(sr, waiting_pkt, waiting_pkts->len, interface);
      }
      sr_arpreq_destroy(&(sr->cache), cached_req);
    }
  } else {
    fprintf(stderr, "Unknown ARP operation\n");
    return;
  }
}
