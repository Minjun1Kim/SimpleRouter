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
Implemented in `sr_handleARPpacket` within `sr_router.c`:

Handles ARP packets in the `sr_handlepacket` function with two main cases:

1. **Receiving an ARP Request Packet**:
   - **Creates an ARP Reply Packet**: Initializes a reply packet with most headers mirroring the request packet and modifies the following fields:

     - **Ethernet Header**:
       - **Destination Address**: Set to the request packet's source address.
       - **Source Address**: Set to the address of the interface that received the request.
     
     - **ARP Header**:
       - **Opcode**: Set to 2 (ARP Reply).
       - **Target Hardware Address**: Set to the request packet's sender hardware address.
       - **Sender Hardware Address**: Set to the address of the interface that received the request.
       - **Target IP Address**: Set to the request packet's sender IP address.
