# SimpleRouter

Names:
Minjun Kim
Ahmad Hakim

Contributions:

Minjun

- Implemented in `sr_ip_packet.c`

1. Receive Raw Ethernet Frames
2. Check whether the received packet is an IP packet or ARP packet.

Case 1: IP packet

a. if the packet is for me (i.e. the router), check whether it is an ICMP echo request or a TCP/UDP. `sr_handleIPpacket` <br/>
b. if ICMP echo request, we send an echo reply. `send_icmp_echo_reply` <br/>
c. if TCP/UDP, send a ICMP error message saying port is unreachable. <br/>

d. if the packet is not for me, then check the routing table using Longest Prefix Matching to see which interface is used to forward the packet to its destination. <br/>
e. If there's no match, then send a ICMP error message indicating "Destination net unrachable" with type 3 and code 0. <br/>
f. if there's a match, then we check the ARP cache for the MAC address of the destination. <br/>
g. If the MAC address can be found, forward the frame to the next hop. <br/>
h. If not, enqueue the packet and send an ARP request to the nework to obtain back a MAC address. <br/>
i. If no success after 5 ARP requests, send an ICMP error message indicating "Destination host unreachable" with type 3, code 1 <br/>

Ahmad

- Implemented `sr_handleARPpacket` in `sr_router.c`:
This method handles ARP packets in the `sr_handlepacket` function

Implementation details: After checking that the packet's length is valid and that it's targetting one of the router's interfaces, this function handles 2 cases:
1. Recieving ARP request packet:
It creates a new ARP reply packet that's identical to the request packet as most of the headers information are the same and modifies the following:

Ethernet Header:
a. Sets the destination ethernet address to the request packet's source ethernet address (the destination is the address of the one who sent the request)
b. Sets the source ethernet address to the address of the interface that recieved the request packet.

ARP Header:
a. Sets the ARP opcode to 2 (ARP reply opcode)
b. Sets the target hardware address to the request packet's sender hardware address (Because we're targetting the one who sent the request)
c. Sets the sender hardware address to the address of the interface that recieved the request packet.
d. Sets the target IP address to the request packet's sender IP address (Because we're targetting the one who sent the request)
e. Sets the sender IP address to the IP address of the interface that recieved the request packet.

And finally it sends the reply packet.

2. Recieving ARP reply packet:
a. Inserts (packet's sender's hardware address, sender's IP address) as an entry in the ARP cache
b. Loops over the packets waiting for this reply and sends them after modifying the ethernet header.
