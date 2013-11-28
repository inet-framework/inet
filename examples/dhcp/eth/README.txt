DHCP (Dynamic Host Configuration Protocol) Examples
===================================================


1. WiredNetWithDHCP
-------------------

This scenario introduces how DHCP works when multiple hosts
and a single DHCP server is present.

The server is continuously offering addresses from its pool
starting from ipAddressStart in response to each client request.

client[0], client[1], ..., client[9] get 192.168.1.100, 192.168.1.101,
,..., 192.168.1.109 addresses respectively, and throughout the simulation
they periodically renew their addresses.

2. DHCPWithLifeCycle
--------------------

This scenario demonstrates how DHCP works when a client or a server
reboots.

After rebooting, the client remembers its last assigned IP address
and if it has not expired yet, then the client starts up in the
INIT-REBOOT state and tries to reallocate this old IP address.

Client starts DHCP initialization at 0.5s and gets IP address
192.168.1.100.

Client shutdowns at 60s and reboots at 70s. After rebooting it tries
to renew its previously allocated address. The client's old lease has
not expired yet, since the lease timeout is 150s in this scenario,
thus it will sucessfully renew it.

Server shutdowns at 80s and reboots at 90s and looses its lease database.
When client reaches T1 timeout at 145s (T1 = 0.5*leaseTime) tries to
extend its current lease. This request will be rejected by server because
it has no any information about client. After this refusal, client restarts
the whole DHCP process and asks for a new address. Server offers the first
address of its pool which is again 192.168.1.100 since there are no other
clients in the network.
