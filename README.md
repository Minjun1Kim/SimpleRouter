# SimpleRouter

### Authors
- **Minjun Kim**
- **Ahmad Hakim**

---

## Contributions

### Minjun Kim
Implemented in `sr_ip_packet.c`:

1. **Receive Raw Ethernet Frames**
   - **Packet Type Check**: Determines if the received packet is an IP packet or an ARP packet.

#### Case 1: IP Packet
   - **If the packet is intended for the router**:
     - **ICMP Echo Request**: Sends an echo reply using `send_icmp_echo_reply`.
     - **TCP/UDP Packet**: Sends an ICMP error message indicating "port unreachable."

   - **If the packet is not intended for the router**:
     - **Routing Table Lookup**: Uses Longest Prefix Matching to determine the correct interface for forwarding the packet.
     - **No Match**: Sends an ICMP "Destination net unreachable" error (Type 3, Code 0).
     - **Match Found**:
       - **ARP Cache Check**: Looks up the MAC address for the destination.
       - **If MAC Address Found**: Forwards the frame to the next hop.
       - **If MAC Address Not Found**: Enqueues the packet and sends an ARP request to resolve the MAC address.
       - **No Response After 5 ARP Requests**: Sends an ICMP "Destination host unreachable" error (Type 3, Code 1).

---

### Ahmad Hakim
**Implemented `sr_handleARPpacket` within `sr_router.c`**:

Handles ARP packets in the `sr_handlepacket` function with two main cases:

1. **Receiving an ARP Request Packet**:
   - **Creates an ARP Reply Packet**: Initializes a reply packet with most headers mirroring the request packet and modifies the following fields:

     - **Ethernet Header**:
       - **Destination Address**: Set to the request packet's source address.
       - **Source Address**: Set to the address of the interface that received the request.
     
     - **ARP Header**:
       - **Opcode**: Set to 2 (ARP Reply).
       - **Target IP Address**: Set to the request packet's sender IP address.
       - **Sender IP Address**: Set to the IP address of the interface that received the request.
       - **Target Hardware Address**: Set to the request packet's sender hardware address.
       - **Sender Hardware Address**: Set to the address of the interface that received the request.

2. **Receiving an ARP Reply Packet**:
   - Inserts (packet's sender's hardware address, sender's IP address) as an entry in the ARP cache.
   - Loops over the packets waiting for this reply and sends them after modifying the ethernet header.

If it's not one of those 2 cases, it ignores the packet.


**Implemented `send_arp_request` in `sr_arpcache.c`**:

Sends an ARP request packet that corresponds to an ARP request entry in the ARP cache:
   
   - **Ethernet Header**:
    - **Destination Address**: Set to 0 to broadcast the request.
    - **Source Address**: Set to the address of the interface that received the request.
    - **Packet Type ID**: Set to ARP opcode.
     
   - **ARP Header**:
      - **Opcode**: Set to 1 (ARP Request).
      - **Target IP Address**: Set to the IP address attached to the ARP request entry.
      - **Sender IP Address**: Set to the IP address of the interface that received the request.
      - **Target Hardware Address**: 0.
      - **Sender Hardware Address**: Set to the address of the interface that received the request.
      - **Format of hardware address**: 1 (ARP header format opcode).
      - **Format of protocol address**: Set to IP opcode.
      - **Length of hardware address**: 6.
      - **Length of protocol address**: 4.
    

**Implemented `sr_arpcache_sweepreqs` in `sr_arpcache.c`**:

This function gets called every second. For each request sent out, we keep checking whether we should resend an request or destroy the arp request.

   - Loops over ARP requests in cache and check for 3 cases:
      - **If request have been sent 5 times**: Loops over the packets waiting for a reply from this request and send back `Destination host unreachable` ICMP packet.
      - **Else if the request hasn't been sent for the last second**: Send an ARP request packet again using `send_arp_request`.
      - **Else**: Ignore this request for now.
