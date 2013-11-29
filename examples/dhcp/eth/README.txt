DHCP (Dynamic Host Configuration Protocol) Examples
===================================================


1. WiredNetWithDHCP
-------------------

This scenario introduces how DHCP works when multiple hosts and a single DHCP
server are present. At random startup times, the clients request IP addresses
from the server, which serves them from its address pool.

client[0], client[1], ..., client[9] receive the addresses 192.168.1.100 through
192.168.1.109, respectively. The lease time is configured to be relatively low
(1000s), causing the clients to periodically renew their addresses.


2. DHCPWithLifeCycle
--------------------

This scenario demonstrates how DHCP works when a client or a server reboots.

The client starts DHCP initialization at 0.5s, and receives IP address 
192.168.1.100 shortly after.

The client shuts down at 60s and reboots at 70s. The behavior implemented
in DHCPClient is that the client remembers its last assigned IP address
after the reboot, and if the address has not expired yet, then DHCP will 
start up in the INIT-REBOOT state and trys to reallocate this old IP address.

In this example, the client's old lease has not expired yet (the lease time 
in this scenario is 150s), thus the client will sucessfully renew it.

The server shuts down at 80s and reboots at 90s, and loses its lease database.
When client reaches T1 timeout at 145s (T1 = 0.5*leaseTime), it tries to
extend its current lease. This request will be rejected by the server, because
it no longer knows about the client. After the refusal, the client restarts
the whole DHCP process, and asks for a new address. The server will offer the 
first available address from its pool, which is again 192.168.1.100 since there 
are no other clients in the network.

3. WirelessNetWithDHCP
----------------------

