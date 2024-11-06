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

#### Implemented Functionalities:

##### IP Packet Handling (`sr_handleIPpacket`):
- Validates incoming IP packets.
- Determines if the packet is destined for the router or should be forwarded.
- Handles ICMP Echo Requests and generates Echo Replies.
- Sends ICMP error messages for unreachable ports.
- Forwards packets using Longest Prefix Match routing.

##### ICMP Packet Processing (`handle_icmp_packet` and `send_icmp_echo_reply`):
- Processes ICMP packets received by the router.
- Validates ICMP checksum.
- Generates ICMP Echo Replies for Echo Requests.

##### Packet Forwarding (`forward_ip_packet`):
- Decrements TTL and updates the IP checksum.
- Performs Longest Prefix Match to find the appropriate routing entry.
- Checks the ARP cache for the next-hop MAC address.
- Sends packets directly if the MAC address is known.
- Queues packets and initiates ARP requests if the MAC address is unknown.

##### Longest Prefix Match (`lpm`):
- Implements the Longest Prefix Match algorithm to find the best routing entry for a given destination IP address.

##### Sending ICMP Error Messages (`send_icmp_error`):
- Constructs and sends ICMP error messages (e.g., Time Exceeded, Destination Unreachable).
- Ensures that error messages are sent correctly according to the protocol specifications.

---
<a id="functions-minjun"></a>
#### Function Descriptions

##### `void sr_handleIPpacket(struct sr_instance* sr, uint8_t *packet, unsigned int len, char* interface)`

**Description:**  
Handles incoming IP packets received by the router. It performs validation checks, determines whether the packet is destined for the router, and either processes it locally or forwards it to the next hop.

**Parameters:**  
- `sr`: Pointer to the router instance.
- `packet`: Pointer to the incoming packet buffer.
- `len`: Length of the incoming packet.
- `interface`: Name of the interface on which the packet was received.

**Functionality:**

- **Validation:**
  - Checks if the packet length is sufficient for an IP packet.
  - Validates the IP version (IPv4) and header length.
  - Verifies the IP checksum.

- **Destination Check:**
  - Determines if the destination IP address matches any of the router's interfaces.
  - If it does, processes the packet locally.
  - If not, forwards the packet.

- **Local Processing:**
  - If the packet is an ICMP Echo Request, calls `handle_icmp_packet`.
  - If the packet is a TCP or UDP packet, sends an ICMP Port Unreachable error message using `send_icmp_error`.

- **Forwarding:**
  - Calls `forward_ip_packet` to forward the packet to the next hop.

---

##### `void handle_icmp_packet(struct sr_instance *sr, uint8_t *packet, unsigned int len, char *interface)`

**Description:**  
Processes ICMP packets that are destined for the router. It validates the ICMP checksum and generates appropriate responses for ICMP Echo Requests.

**Parameters:**  
- `sr`: Pointer to the router instance.
- `packet`: Pointer to the incoming packet buffer.
- `len`: Length of the incoming packet.
- `interface`: Name of the interface on which the packet was received.

**Functionality:**

- **Validation:**
  - Ensures the packet length is sufficient for an ICMP packet.
  - Extracts the ICMP header and validates the ICMP checksum.

- **Processing:**
  - Checks if the ICMP packet is an Echo Request (Type 8, Code 0).
  - If it is, calls `send_icmp_echo_reply` to generate an Echo Reply.

---

##### `void send_icmp_echo_reply(struct sr_instance *sr, uint8_t *orig_packet, unsigned int orig_len, char *interface)`

**Description:**  
Generates and sends an ICMP Echo Reply in response to an Echo Request. Constructs a new packet to maintain the integrity of the original packet.

**Parameters:**  
- `sr`: Pointer to the router instance.
- `orig_packet`: Pointer to the original Echo Request packet buffer.
- `orig_len`: Length of the original packet.
- `interface`: Name of the interface on which the original packet was received.

**Functionality:**

- **Packet Construction:**
  - Allocates memory for the new Echo Reply packet.
  - Constructs the Ethernet header by swapping the source and destination MAC addresses.
  - Constructs the IP header:
    - Swaps the source and destination IP addresses.
    - Sets the TTL to a standard value (e.g., 64).
    - Recomputes the IP checksum.
  - Constructs the ICMP header and payload:
    - Changes the ICMP type to Echo Reply (Type 0).
    - Recomputes the ICMP checksum.

- **Sending Packet:**
  - Sends the new packet using `sr_send_packet`.
  - Frees the allocated memory after sending.

---

##### `void forward_ip_packet(struct sr_instance *sr, uint8_t *packet, unsigned int len, char *interface)`

**Description:**  
Forwards IP packets that are not destined for the router. It decrements the TTL, updates the checksum, performs routing, and handles ARP resolution.

**Parameters:**  
- `sr`: Pointer to the router instance.
- `packet`: Pointer to the packet buffer (packet will be modified).
- `len`: Length of the packet.
- `interface`: Name of the interface on which the packet was received.

**Functionality:**

- **TTL Handling:**
  - Checks if the TTL is less than or equal to 1.
  - If so, sends an ICMP Time Exceeded error message using `send_icmp_error`.

- **Decrement and Update:**
  - Decrements the TTL by 1.
  - Recomputes the IP checksum.

- **Routing:**
  - Performs Longest Prefix Match using `lpm` to find the best route.
  - If no route is found, sends an ICMP Destination Net Unreachable error message.

- **ARP Resolution:**
  - Checks the ARP cache for the next-hop MAC address.
  - If found, updates the Ethernet header and sends the packet.
  - If not found, queues the packet and initiates an ARP request.

---

##### `struct sr_rt *lpm(struct sr_instance *sr, uint32_t ip_dst)`

**Description:**  
Performs the Longest Prefix Match algorithm to find the best routing table entry for a given destination IP address.

**Parameters:**  
- `sr`: Pointer to the router instance.
- `ip_dst`: Destination IP address (in network byte order).

**Returns:**  
A pointer to the best matching routing table entry, or `NULL` if no match is found.

**Functionality:**

- **Iteration:**
  - Iterates over the routing table entries.
  - For each entry, computes the bitwise AND of the destination IP and the subnet mask.

- **Matching:**
  - Compares the result with the network address of the routing entry.
  - Keeps track of the entry with the longest matching subnet mask.

- **Selection:**
  - Returns the routing table entry with the longest prefix match.

---

##### `void send_icmp_error(struct sr_instance *sr, uint8_t *orig_packet, unsigned int orig_len, uint8_t type, uint8_t code, char *interface)`

**Description:**  
Constructs and sends an ICMP error message (e.g., Destination Unreachable, Time Exceeded) in response to a packet that cannot be forwarded.

**Parameters:**  
- `sr`: Pointer to the router instance.
- `orig_packet`: Pointer to the original packet that caused the error.
- `orig_len`: Length of the original packet.
- `type`: ICMP error type.
- `code`: ICMP error code.
- `interface`: Name of the interface on which the original packet was received.

**Functionality:**

- **Packet Construction:**
  - Allocates memory for the ICMP error packet.

  - **Constructs the Ethernet header:**
    - Sets the source MAC address to the router's MAC address on the received interface.
    - Destination MAC address will be determined via ARP resolution.

  - **Constructs the IP header:**
    - Sets the source IP address to the router's IP on the received interface.
    - Sets the destination IP address to the source IP of the original packet.
    - Sets the appropriate protocol, TTL, and computes the IP checksum.

  - **Constructs the ICMP header:**
    - Sets the ICMP type and code.
    - Includes the IP header and the first 8 bytes of the original packet's payload.
    - Computes the ICMP checksum.

- **ARP Resolution and Sending:**
  - Checks the ARP cache for the destination IP.
  - If the MAC address is known, sets the destination MAC address and sends the packet.
  - If not, queues the packet and initiates an ARP request.




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

