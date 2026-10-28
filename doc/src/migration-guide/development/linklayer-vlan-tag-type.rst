VLAN Tag Type of S-VLAN Nodes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The S-tag (IEEE 802.1ad) has request and indication tags of its own now,
:cpp:`SVlanReq`, :cpp:`SVlanInd`, :cpp:`SPcpReq`, :cpp:`SPcpInd`,
:cpp:`SDropEligibleReq` and :cpp:`SDropEligibleInd`, beside those of the C-tag.
An 802.1Q tag inserter or checker whose :par:`vlanTagType` is ``"s"`` reads and
writes the S-tag tags only. Before, the C-tag and the S-tag shared one set of
tags, so :par:`vlanTagType` chose only the TPID of the tag.

The modules that use the VLAN tags have a :par:`vlanTagType` parameter of their
own, ``"c"`` by default: the VLAN policy modules, the relays with their learners
and selectors, :ned:`Ieee8021qSocketPacketProcessor`, :ned:`PcpClassifier`,
:ned:`PcpTrafficClassClassifier`, :ned:`GroupEligibilityTimeMeter`,
:ned:`StreamEncoder`, :ned:`StreamDecoder` and the packet taggers. The layers
:ned:`Ieee8021qLayer`, :ned:`VlanPolicyLayer`, :ned:`BridgingLayer`,
:ned:`InterfaceRelayLayer` and :ned:`StreamCoderLayer` pass theirs to their
submodules.

A node that works with S-tags, for example an S-VLAN bridge, sets
:par:`vlanTagType` on all of these modules, not only on its 802.1Q layer. One line
for the whole node does it. :ned:`VlanReqMapper` no longer has the :par:`protocol`
parameter: the protocol that it requests follows from its :par:`vlanTagType`.
Delete the lines that set :par:`protocol`.

.. code-block:: ini

   # before
   *.switch.ieee8021q.vlanTagType = "s"
   *.switch.bridging.vlanPolicy.outboundMapper.protocol = "ieee8021qstag"

   # after
   *.switch.**.vlanTagType = "s"

A configuration that still sets ``vlanTagType = "s"`` only on the 802.1Q layer
gets no error message that names the cause. It either stops with an error about
something else, for example a missing ``MacAddressInd`` tag in the relay, or it
runs without VLAN separation: the relay learns and forwards all frames in VLAN
0, and the VLAN filters see every frame as untagged.
