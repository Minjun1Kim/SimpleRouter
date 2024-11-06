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
---

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

