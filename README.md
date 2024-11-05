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
a. if the packet is for me (i.e. the router), check whether it is an ICMP echo request or a TCP/UDP. `sr_handleIPpacket`
b. if ICMP echo request, we send an echo reply. `send_icmp_echo_reply`
c. if TCP/UDP, send a ICMP error message saying port is unreachable.

d. if the packet is not for me, then check the routing table using Longest Prefix Matching to see which interface is used to forward the packet to its destination.
e. If there's no match, then send a ICMP error message indicating "Destination net unrachable" with type 3 and code 0.
f. if there's a match, then we check the ARP cache for the MAC address of the destination.
g. If the MAC address can be found, forward the frame to the next hop.
h. If not, enqueue the packet and send an ARP request to the nework to obtain back a MAC address.
i. If no success after 5 ARP requests, send an ICMP error message indicating "Destination host unreachable" with type 3, code 1



List of test cases
