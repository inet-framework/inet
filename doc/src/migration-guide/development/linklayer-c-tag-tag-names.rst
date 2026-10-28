C-Tag Names of the VLAN, PCP and Drop Eligible Tags
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The request and indication tags of the IEEE 802.1Q tag fields carry the C-tag
in their names now, so that the S-tag (IEEE 802.1ad) can have tags of its own.
The old names have no aliases. C++ code that attaches or reads these tags no
longer compiles until it uses the new names, and a :par:`classifierClass`
parameter in an ini or NED file that names an old classifier function must name
the new one:

=====================================  =====================================
Old name                               New name
=====================================  =====================================
:cpp:`VlanReq`, :cpp:`VlanInd`         :cpp:`CVlanReq`, :cpp:`CVlanInd`
:cpp:`PcpReq`, :cpp:`PcpInd`           :cpp:`CPcpReq`, :cpp:`CPcpInd`
:cpp:`DropEligibleReq`                 :cpp:`CDropEligibleReq`
:cpp:`DropEligibleInd`                 :cpp:`CDropEligibleInd`
``inet::PacketVlanReqClassifier``      ``inet::PacketCVlanReqClassifier``
``inet::PacketVlanIndClassifier``      ``inet::PacketCVlanIndClassifier``
=====================================  =====================================

The renames are one-to-one, so a search and replace of the whole words is
enough, for example:

.. code-block:: bash

   sed -i -E 's/\b(VlanReq|VlanInd|PcpReq|PcpInd|DropEligibleReq|DropEligibleInd)\b/C\1/g; s/\bPacketVlan(Req|Ind)Classifier\b/PacketCVlan\1Classifier/g' <files>
