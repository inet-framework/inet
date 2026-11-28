AODV link metadata
~~~~~~~~~~~~~~~~~~

Custom interfaces delivering AODV control must supply a valid ``InterfaceInd``. Control
on an unselected interface is ignored. Keep control traffic on the configured
ad hoc interface or explicitly select the intended interfaces when that mode
is used.

Peer-specific transmission failure handling requires ``InterfaceReq`` and
destination ``MacAddressReq`` on the failed packet, plus a unique IP/MAC binding
learned from one-hop control reception on that interface. Missing tags or
unavailable bindings produce a diagnostic and skip peer-specific handling.
AODV no longer guesses the failed peer from the current route to the packet's
final destination.

Bindings assume stable immediate-peer attribution during an operational
interval. A changed IP/MAC association or a MAC claimed by another IP makes the
affected MAC bindings unusable until restart. Reconfigure addresses/interfaces
using stop/reconfigure/start. Direct AckingMac and nonaggregating independent
802.11 DCF paths have focused evidence. Bridging/WDS, aggregation and custom
metadata meanings require their own evidence.
