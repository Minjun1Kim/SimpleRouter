# SimpleRouter

### Authors
- **Minjun Kim - 1007705146**
- **Ahmad Hakim**

---

## <span style="color:#ADD8E6">Table of Contents </span> 
- [Contributions](#contributions)
  - [Minjun](#minjun)
     - [Descriptions of Functions](#functions-minjun)
  - [Ahmad](#ahmad)
- [Test Cases & Results](#tests)

---

<a id="contributions"></a>
## Contributions

<a id="minjun"></a>
### Minjun Kim

In the `sr_ip_packet.c` file, I implemented the logic for handling IP packets in our software router. This includes processing incoming IP packets, generating appropriate responses, and forwarding packets to the next hop based on the routing table.

I also defined an enumeration for IP protocols in `sr_protocol.h` to improve code readability:
The descriptions for the functions are listed in: [`src/sr_ip_packet.c`](https://github.com/Minjun1Kim/SimpleRouter/blob/ip%2Barp/router/sr_ip_packet.h):

<a id="functions-minjun"></a>
#### Implemented Functionalities:

**A. Implemented `sr_handleIPpacket` within `sr_ip_packet.c`**:

Handles incoming IP packets in our software router, with responsibilities for processing, forwarding, and generating responses:

1. **IP Packet Handling (`sr_handleIPpacket`)**:
   - **Validation**:
     - Checks if the packet length is sufficient for an IP packet.
     - Validates the IP version (IPv4) and header length.
     - Verifies the IP checksum.
   - **Destination Check**:
     - Determines if the destination IP matches any router interfaces.
     - If matched, processes the packet locally.
     - If unmatched, forwards the packet.
   - **Local Processing**:
     - If the packet is an ICMP Echo Request, calls `handle_icmp_packet`.
     - If it's a TCP or UDP packet, sends an ICMP Port Unreachable error using `send_icmp_error`.
   - **Forwarding**:
     - Calls `forward_ip_packet` to forward the packet to the next hop.

---

**B. Defined `handle_icmp_packet` and `send_icmp_echo_reply` for ICMP Packet Processing**:

Handles ICMP packets received by the router, including checksum validation and echo reply generation:

1. **ICMP Packet Processing (`handle_icmp_packet`)**:
   - **Validation**:
     - Ensures packet length suffices for ICMP.
     - Extracts the ICMP header and verifies checksum.
   - **Processing**:
     - Checks for ICMP Echo Request (Type 8, Code 0).
     - Calls `send_icmp_echo_reply` if it's an Echo Request.

2. **Generating ICMP Echo Replies (`send_icmp_echo_reply`)**:
   - **Packet Construction**:
     - Allocates memory for a new Echo Reply packet.
     - Constructs Ethernet header with swapped source/destination MACs.
     - Sets IP TTL to 64 and recalculates checksum.
     - Changes ICMP type to Echo Reply and recalculates checksum.
   - **Sending Packet**:
     - Sends the new packet using `sr_send_packet`.
     - Frees memory post-send.

---

**C. Implemented `forward_ip_packet` for IP Packet Forwarding**:

Forwards IP packets that are not destined for the router, decrements TTL, updates checksums, performs routing, and resolves ARP:

1. **Forwarding Functionality (`forward_ip_packet`)**:
   - **TTL Handling**:
     - Sends ICMP Time Exceeded error if TTL ≤ 1 using `send_icmp_error`.
     - Otherwise, decrements TTL and updates checksum.
   - **Routing**:
     - Uses Longest Prefix Match (`lpm`) for the best route.
     - Sends ICMP Destination Net Unreachable if no route is found.
   - **ARP Resolution**:
     - If MAC is known, updates Ethernet header and sends packet.
     - If MAC is unknown, queues packet and initiates ARP request.

---

**D. Implemented `lpm` Function for Longest Prefix Match**:

Finds the best routing entry for a destination IP address using Longest Prefix Match:

1. **Longest Prefix Match (`lpm`)**:
   - **Iteration and Matching**:
     - Iterates through routing entries.
     - Matches destination IP with network addresses based on subnet masks.
   - **Selection**:
     - Returns entry with longest matching prefix.

---

**E. Implemented `send_icmp_error` for Sending ICMP Error Messages**:

Constructs and sends ICMP error messages (e.g., Destination Unreachable, Time Exceeded) in response to unforwardable packets:

1. **ICMP Error Message Handling (`send_icmp_error`)**:
   - **Packet Construction**:
     - Allocates memory for the ICMP error packet.
     - **Ethernet Header**:
       - Sets source MAC to the router’s MAC on the received interface.
       - Destination MAC resolved via ARP.
     - **IP Header**:
       - Sets source IP to router's IP on received interface.
       - Sets destination IP to the source IP of original packet.
       - Calculates IP checksum.
     - **ICMP Header**:
       - Sets type and code.
       - Includes IP header and first 8 bytes of original packet payload.
       - Calculates ICMP checksum.
   - **ARP Resolution and Sending**:
     - Checks ARP cache for destination MAC.
     - If MAC found, sets destination MAC and sends packet.
     - If MAC not found, queues packet and initiates ARP request.

---
---

<a id="ahmad"></a>
### Ahmad Hakim
A. **Implemented `sr_handleARPpacket` within `sr_router.c`**:

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

***

B. **Implemented `send_arp_request` in `sr_arpcache.c`**:

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
    
***

C. **Implemented `sr_arpcache_sweepreqs` in `sr_arpcache.c`**:

This function gets called every second. For each request sent out, we keep checking whether we should resend an request or destroy the arp request.

   - Loops over ARP requests in cache and check for 3 cases:
      - **If request have been sent 5 times**: Loops over the packets waiting for a reply from this request and send back `Destination host unreachable` ICMP packet.
      - **Else if the request hasn't been sent for the last second**: Send an ARP request packet again using `send_arp_request`.
      - **Else**: Ignore this request for now.
    
---

<a id="tests"></a>
## Test Cases
After following the steps mentioned in the handout (Running `./run_pox.sh` and `./run_mininet.sh` in two seperate terminals)
Open a third terminal, cd into the router folder, run `make` to compile, then run `./sr` to run our solution.
Now we can test on the mininet terminal:

   - **Pinging the router's interfaces**:
      - Run `client ping -c 3 10.0.1.1` (you can replace 10.0.1.1 with 192.168.2.1 or 172.64.3.1 to ping the other interfaces)
      - You should get an output similar to this:
```
PING 10.0.1.1 (10.0.1.1) 56(84) bytes of data.
64 bytes from 10.0.1.1: icmp_seq=1 ttl=64 time=221 ms
64 bytes from 10.0.1.1: icmp_seq=2 ttl=64 time=52.4 ms
64 bytes from 10.0.1.1: icmp_seq=3 ttl=64 time=50.6 ms

--- 10.0.1.1 ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 2002ms
rtt min/avg/max/mdev = 50.609/107.853/220.516/79.668 ms
```


   - **Pinging a server**:
      - Run `client ping -c 3 192.168.2.2` (you can replace 192.168.2.2 with 172.64.3.10 to ping the other server)
      - You should get an output similar to this:
```
PING 192.168.2.2 (192.168.2.2) 56(84) bytes of data.
64 bytes from 192.168.2.2: icmp_seq=2 ttl=63 time=202 ms
64 bytes from 192.168.2.2: icmp_seq=1 ttl=63 time=1236 ms
64 bytes from 192.168.2.2: icmp_seq=3 ttl=63 time=89.4 ms

--- 192.168.2.2 ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 2035ms
rtt min/avg/max/mdev = 89.432/509.266/1236.334/516.165 ms, pipe 2
```

   - **Tracerouting to the router's interfaces**:
      - Run `client traceroute -n 10.0.1.1` (you can replace 10.0.1.1 with 192.168.2.1 or 172.64.3.1 to traceroute to the other interfaces)
      - You should get an output similar to this:
```
traceroute to 10.0.1.1 (10.0.1.1), 30 hops max, 60 byte packets
 1  10.0.1.1  124.220 ms  114.438 ms  125.683 ms
```

   - **Tracerouting to a server**:
      - Run `client traceroute -n 192.168.2.2` (you can replace 192.168.2.2 with 172.64.3.10 to traceroute to the other server)
      - You should get an output similar to this:
```
traceroute to 192.168.2.2 (192.168.2.2), 30 hops max, 60 byte packets
 1  10.0.1.1  31.851 ms * *
 2  192.168.2.2  122.137 ms  122.900 ms  123.990 ms
```

   - **Downloading a file using HTTP from one of the app servers**:
      - Run `client wget http://192.168.2.2` (you can replace 192.168.2.2 with 172.64.3.10 to download from server2)
      - You should get an output similar to this and have a file `index.html` downloaded:
```
--2024-11-05 17:12:24--  http://192.168.2.2/
Connecting to 192.168.2.2:80... connected.
HTTP request sent, awaiting response... 200 OK
Length: 161 [text/html]
Saving to: ‘index.html.3’

index.html.3        100%[===================>]     161  --.-KB/s    in 0s      

2024-11-05 17:12:26 (17.2 MB/s) - ‘index.html.3’ saved [161/161]
```

