DSDV base-station profile
~~~~~~~~~~~~~~~~~~~~~~~~~

The optional DSDV base-station profile adds default reachability through one explicitly configured station. Existing host-only domains keep their version-1 wire format. Enabling the base-station profile selects version 2 on every participant; mixed versions reject routing updates rather than negotiate an upgrade. Configure a whole domain consistently. Parameter declarations and defaults remain in :ned:`Dsdv`.

The following is a configuration example for three :ned:`DsdvRouter` nodes. Their ad hoc IPv4 addresses are 10.0.0.1, 10.0.0.2 and 10.0.0.3, with only adjacent wireless nodes in range. The last node is the base station:

.. code-block:: ini

   *.mobile.dsdv.baseStationAddress = "10.0.0.3"
   *.relay.dsdv.baseStationAddress = "10.0.0.3"
   *.base.dsdv.baseStationAddress = "10.0.0.3"
   *.base.numEthInterfaces = 1
   *.base.dsdv.externalInterface = "eth0"

Assign the example's base-station uplink address 10.0.1.2/24 and wire it to router 10.0.1.1. Give that router a second interface 10.0.2.1/24 toward an external host at 10.0.2.2. Supply the base station's external default through 10.0.1.1, a router return route for 10.0.0.0/24 through 10.0.1.2, and the external host's default through 10.0.2.1. These addresses and routes are example settings, not automatic configuration or parameter defaults.

Enable IPv4 forwarding on the relay, base station and wired router. Retain the connected 10.0.0.0/24 route on wireless participants so an internal destination with no DSDV host route uses local address resolution instead of the external default. Use ordinary local ARP, distinct wired prefixes and no proxy ARP. Do not supply configurator-generated multihop host routes that bypass DSDV.

The base station may use its explicitly named external interface in addition to its ad hoc interface; ordinary participants still support one active non-loopback interface. DSDV control remains on the ad hoc interface. The station advertises external service only when the modeled uplink, forwarding and external default route are usable. A missing uplink route leaves it operating as an ordinary host router. This adds neither NAT nor remote-network health probing.

When external service disappears, default capability is withdrawn while the base station remains reachable over a usable wireless host route. Losing the wireless path withdraws its dependent routes as well. A foreign default on a mobile participant is preserved and suppresses the DSDV default. Do not configure multiple gateways or change the shared base-station identity during a run.

Version 2 appends a capability flag to each route entry. Update external decoders or byte-based fixtures that opt into this profile. The model profile under ``doc/project/evidence/model/dsdv/profile.md`` owns the version-2 byte layout and selection rules; the version-1 definition remains under ``doc/project/evidence/protocol/dsdv/standards.md``. No external wire interoperability is claimed.
