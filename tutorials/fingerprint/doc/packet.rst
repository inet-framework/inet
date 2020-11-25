Changing Packet Length
======================

.. TODO

  tplx -> tpx
  ha csak mondjuk az app csomaghossza valtozott 1-rol 2-byte-ra, ethernet padding miatt min csomag length 64
  vagy
  tplx -> px
  ha nagyobb a valtozas

  workflow -> ha megváltoztatjuk a csomag hosszát de belefér a min ehternet frame-be akkor az az expectation hogy nem változik meg a fingerprint (a csomag hossza a network-ön nem változik) és így az időzítések se
  de ha nagy a változás akkor igen

  TODO minden stepnél -> expectation -> a mögöttes gondolkodás

  l -> minden eseménynél figyelembe veszi a csomag hosszát

  mit gondolsz az adott change-nél

  so

  - what happens when you change the packet length? the tplx fingerprints change
  - the packet length is part of the ingredients
  - it affects fingerprints in all stages of the packet -> as it travels down the protocol stack
  - so the packet length should be dropped
  - what is the point in this ? you change the header length and you think it doesnt alter the model
  and want to make sure it doesn't ? or just show what fingerprints break or not break when
  the packet length is changed
  - the thinking is that when the packet is small it doesn't break the fingerprints

  - change something in the model that causes the packet length to change, e.g. a change in a protocol header

Some changes in the model, e.g. adding fields to a protocol header, can cause the packet lengths to change.
This in turn leads to changes in the ``tplx`` fingerprints. The ``l`` stands for packet length, and the length of packets anywhere in the network (inlcuding host submodules) is taken into account at every event.
If we drop the packet length from the fingerprint ingredients, the fingerprints might still change due to the different timings of the packets.

.. **TODO** is there a missing logical step here? that we drop the packet length from the fingerprints to see if the model behaves the same way as before?

.. **V1** Even if we drop the packet length from the fingerprint ingredients, the fingerprints might still change due to the different timings of the longer/shorter packets/frames.

.. Our expectation is that we change the packet length, but if the packets are small and still fit in a minimum Ethernet frame size of 64 bytes, the frame is padded, and the packet length change doesn't matter in terms of fingerprints/doesn't affect fingerprints. If the packet is smaller than the minimum ethernet size, the frame is padded.

.. However, packets that are small enough to fit in a minimum-size Ethernet frame are padded

However, small packets are padded to fit into a minimum-size Ethernet frame of 64 bytes.
In this case, the changed protocol header size wouldn't affect ``tpx`` fingerprints, as the Ethernet frame sizes, and thus the timings, are the same.
A similar padding effect happens in 802.11 frames. The data contained in the frame is a multiple of the bits/symbol times the number of subcarriers for the given modulation. For QAM-64, this is around 30 bytes.
Thus we expect that for small packets the fingerprints wouldn't change; for larger ones, they would
(assuming small changes in the header size).

.. **TODO** this is for small changes in the header size

.. **TODO** so we expect that for small packets the fingerprints don't change; for larger ones they do

In the following example, we'll increase the Udp header size from 8 bytes to 10 bytes.

.. **TODO** do we need additional configs for that? a short packet version of each of the configs?

.. **TODO** do we want to use the ethernet and the wifi padding as well ?

Before making the change, we drop the packet length from the fingerprints and run the tests:

.. TODO .csv with tpx

.. code-block:: text

  .,        -f omnetpp.ini -c Ethernet -r 0,               0.2s,         4500-0673/tpx, PASS,
  .,        -f omnetpp.ini -c EthernetShortPacket -r 0,    0.2s,         ea97-154f/tpx, PASS,
  .,        -f omnetpp.ini -c Wifi -r 0,                     5s,         791d-aba6/tpx, PASS,
  .,        -f omnetpp.ini -c WifiShortPacket -r 0,          5s,         d801-fc01/tpx, PASS,

.. TODO fingerprints fail

.. code-block:: fp

  $ inet_fingerprinttest -m ChangingPacketLength
  . -f omnetpp.ini -c Ethernet -r 0  ... : FAILED
  . -f omnetpp.ini -c Wifi -r 0  ... : FAILED
  . -f omnetpp.ini -c WifiShortPacket -r 0  ... : FAILED
  . -f omnetpp.ini -c EthernetShortPacket -r 0  ... : FAILED

The tests failed because the values in the .csv file were calculated with the default ingredients. We can update the .csv with the new fingerprints:

.. code-block:: fp

   $ mv baseline.csv.UPDATED baseline.csv

Then we increase the Udp header size:

.. literalinclude:: ../sources/UdpHeader.msg.mod
   :diff: ../sources/UdpHeader.msg.orig

The change needs to be followed in ``UdpHeaderSerializer.cc`` as well:
(otherwise the packets couldn't be serialized, leading to dropped packets)

.. literalinclude:: ../sources/UdpHeaderSerializer.cc.modified
   :diff: ../sources/UdpHeaderSerializer.cc.orig

We run the fingerprint tests again:

.. code-block:: fp

  $ inet_fingerprinttest
  . -f omnetpp.ini -c Ethernet -r 0  ... : FAILED
  . -f omnetpp.ini -c Wifi -r 0  ... : FAILED
  . -f omnetpp.ini -c WifiShortPacket -r 0  ... : PASS
  . -f omnetpp.ini -c EthernetShortPacket -r 0  ... : PASS

As expected, the tests pass when the packet size is small.

.. TODO why the two fingerprints didn't change -> for small packets, the fingerprints didn't change because described earlier. For normal/large ones, they did.

.. ----

.. **TODO** finish later

.. **TODO** run with only, and it doesnt work

  .. code-block:: text

    $ inet_fingerprinttest temp.p.nottcpevents.csv -a --fingerprint-events='"not name=~tcp*"'
    . -f omnetpp.ini -c Ethernet -r 0  ... : PASS
    . -f omnetpp.ini -c Ospf -r 0  ... : PASS
    . -f omnetpp.ini -c Wifi -r 0  ... : PASS
    . -f omnetpp.ini -c WifiShortPacket -r 0  ... : PASS
    . -f omnetpp.ini -c EthernetShortPacket -r 0  ... : PASS

  it works this way...because the udp takes longer now, and it might happen that the tcp goes first

  ping request-nek is van length-e es azt megvaltoztatod
  ping req, wlan ack, ping reply, wlan ack
  fingerprint packet name (n), network node full path (N), filter for for network communication (~)

  TODO 100B + p -> csak a sorrend számít -> így se jó

  -> ha 100ra írod akkor egyik se jó (show why not for each) -> majd később mutatunk vmi bonyolultabb példát

  TODO -m -> filter for relevant configs

  ha megváltoztatjuk a header length-et akkor van olyan fingerprint ingredient hogy nem változik meg
  mert csak a node-ok és a message-ek sorrendje számít -> ezt akarjuk megmutatni

  so

  for normal/large packets, the tpx fingerprints change (because of the t) -> but if we use just p,
  then only the order of nodes and messages affect the fingerprints

.. We can find fingerprint ingredients which make the tests for the large packet configs pass as well.
   Before changing the Udp header length, we run the fingerprint tests with just ``p`` as the ingredient.

  We replace the ingredients with ``p`` in the .csv file:

  .. code-block:: text

    .,        -f omnetpp.ini -c Wireless -r 0,        5s,         5e6e-3064/p, PASS,
    .,        -f omnetpp.ini -c Mixed -r 0,           5s,         0bf0-4adf/p, PASS,
    .,        -f omnetpp.ini -c Wired -r 0,           5s,         a92f-8bfe/p, PASS,
    .,        -f omnetpp.ini -c WirelessNID -r 0,     5s,         d410-0d99/NID,  PASS,
    .,        -f omnetpp.ini -c WiredNID -r 0,        5s,         c369-4f80/NID,  PASS,
    .,        -f omnetpp.ini -c MixedNID -r 0,        5s,         50f5-ce11/NID,  PASS,
    .,        -f omnetpp.ini -c WirelessNIDDim -r 0,  5s,         d410-0d99/NID,  PASS,
    .,        -f omnetpp.ini -c WirelessDim -r 0,     5s,         5e6e-3064/p, PASS,
    .,        -f omnetpp.ini -c Ospf -r 0,            5000s,      4e14-28c4/p, PASS,
    .,        -f omnetpp.ini -c Bgp -r 0,            5000s,      4e14-28c4/p, PASS,

  Then we run the fingerprint tests:

  .. code-block:: fp

    $ inet_fingerprinttest
    . -f omnetpp.ini -c WirelessNID -r 0  ... : ERROR
    . -f omnetpp.ini -c Wired -r 0  ... : FAILED
    . -f omnetpp.ini -c Mixed -r 0  ... : FAILED
    . -f omnetpp.ini -c WirelessNIDDim -r 0  ... : ERROR
    . -f omnetpp.ini -c Wireless -r 0  ... : FAILED
    . -f omnetpp.ini -c Ospf -r 0  ... : FAILED
    . -f omnetpp.ini -c WiredNID -r 0  ... : PASS
    . -f omnetpp.ini -c Bgp -r 0  ... : ERROR
    . -f omnetpp.ini -c MixedNID -r 0  ... : PASS
    . -f omnetpp.ini -c WirelessDim -r 0  ... : FAILED

  We can update the .csv file:

  .. code-block:: fp

     $ mv baseline.csv.UPDATED baseline.csv

  Now, when we run the fingerprint tests again, they pass:

  .. code-block:: fp

    $ inet_fingerprinttest
    . -f omnetpp.ini -c WirelessNID -r 0  ... : ERROR
    . -f omnetpp.ini -c Wired -r 0  ... : PASS
    . -f omnetpp.ini -c Mixed -r 0  ... : PASS
    . -f omnetpp.ini -c WirelessNIDDim -r 0  ... : ERROR
    . -f omnetpp.ini -c Wireless -r 0  ... : PASS
    . -f omnetpp.ini -c Ospf -r 0  ... : PASS
    . -f omnetpp.ini -c WiredNID -r 0  ... : PASS
    . -f omnetpp.ini -c Bgp -r 0  ... : ERROR
    . -f omnetpp.ini -c MixedNID -r 0  ... : PASS
    . -f omnetpp.ini -c WirelessDim -r 0  ... : PASS

  **TODO** this seems very convoluted...i think by this time the users might get it how its done (how to create valid fingerprints with different ingredients...from valid fingerprints, without any change in the model)(especially if they are engineers developing models)(the target audience)

  Now, we change the Udp header size from 8 bytes to 10 bytes:

  .. literalinclude:: ../sources/UdpHeader.msg.mod
     :diff: ../sources/UdpHeader.msg.orig

  Then, we run the fingerprint tests again:

  .. code-block:: fp

    $ inet_fingerprinttest
    . -f omnetpp.ini -c WirelessNID -r 0  ... : ERROR
    . -f omnetpp.ini -c Wired -r 0  ... : FAILED
    . -f omnetpp.ini -c Mixed -r 0  ... : FAILED
    . -f omnetpp.ini -c WiredNID -r 0  ... : FAILED
    . -f omnetpp.ini -c WirelessNIDDim -r 0  ... : ERROR
    . -f omnetpp.ini -c Wireless -r 0  ... : FAILED
    . -f omnetpp.ini -c Bgp -r 0  ... : ERROR
    . -f omnetpp.ini -c Ospf -r 0  ... : PASS
    . -f omnetpp.ini -c MixedNID -r 0  ... : FAILED
    . -f omnetpp.ini -c WirelessDim -r 0  ... : FAILED

  **TODO** and it apparently FAILs

  because why?

  now we can try the NID~ thing

  is the problem that it wasnt a 100B packet but the default 1500B?
