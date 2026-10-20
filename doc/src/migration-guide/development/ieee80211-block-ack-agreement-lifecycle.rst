IEEE 802.11 Block Ack Agreement Lifecycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom QoS recipient services must return accepted, complete Actions from ``managementFrameReceived()``. HCF owns their agreement dispatch after duplicate checks.

The recipient response now determines the buffer size and inactivity timeout. The built-in recipient selects the smaller of 64 and its configured maximum. A request size of zero supplies no size guidance. The response copies the request dialog token.

Custom agreement policies must implement ``isProfileAllowed()`` with the supplied typed profile and usable capability inputs. Basic admission must preserve its historical non-HT compatibility.
