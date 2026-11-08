DSDV IPv4 host routing
~~~~~~~~~~~~~~~~~~~~~~

``inet.routing.dsdv.Dsdv`` now implements the bounded Perkins/Bhagwat 1994
network-layer profile using UDP, TTL 1 and one configured ad hoc IPv4 interface.
Custom nodes connect ``socketIn``/``socketOut`` through the transport dispatcher,
or place ``Dsdv`` in an ``app[]`` slot. ``DsdvRouter`` supplies the attachment.
Enable UDP. The C++ class is ``inet::dsdv::Dsdv``.

Replace old DSDV timing/configuration overrides with the parameters documented
in ``Dsdv.ned``: ``interface``, ``controlPort``, ``fullDumpInterval``,
``periodicJitter``, ``neighborTimeout``, ``routeTimeout``, ``settlingCap``,
``minSettlingDelay`` and ``payloadBudget``. This is a new model wire format, not compatible with
``DsdvHello``. The old wire classes remain available for legacy tooling only.
Previous simulation trajectories and recorded DSDV fingerprints do not apply.

New deferred advertisement deadlines have a 10 ms minimum by default, even
when the adaptive settling estimate approaches zero. ``minSettlingDelay`` must
be positive and no greater than ``settlingCap``. If a configuration sets a cap
below 10 ms, also set a positive ``minSettlingDelay`` no larger than that cap.
Urgent advertisements remain immediate. The new minimum changes affected
simulation trajectories and fingerprints; earlier hashes are not preserved.

Use unique static IPv4 addresses, local ARP and forwarding on intermediate
nodes. Disable configurator-generated multihop routes when measuring DSDV.
IPv6, multiple active routing interfaces, prefix/default dissemination and
module destruction/recreation without persistent sequence state are outside
this profile. Stop/start of the same module and interface down/up retain
withdrawal floors; memory and full dumps grow with all observed destinations.
