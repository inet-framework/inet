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

Before making the change, we drop the packet length from the fingerprints. We use the ``tpx`` ingredients.
We run the four simulations with the dummy fingerprint ``0000-0000/tpx``. Each run prints the calculated value:

.. code-block:: bash

  $ inet -u Cmdenv -c Ethernet -r 0 --sim-time-limit=0.2s --fingerprint=0000-0000/tpx
  $ inet -u Cmdenv -c EthernetShortPacket -r 0 --sim-time-limit=0.2s --fingerprint=0000-0000/tpx
  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=0000-0000/tpx
  $ inet -u Cmdenv -c WifiShortPacket -r 0 --sim-time-limit=5s --fingerprint=0000-0000/tpx

.. code-block:: fp

  Fingerprint mismatch! calculated: XXXX-XXXX/tpx, expected: 0000-0000/tpx
  Fingerprint mismatch! calculated: XXXX-XXXX/tpx, expected: 0000-0000/tpx
  Fingerprint mismatch! calculated: XXXX-XXXX/tpx, expected: 0000-0000/tpx
  Fingerprint mismatch! calculated: XXXX-XXXX/tpx, expected: 0000-0000/tpx

Note the four calculated values. The model did not change, so they are correct.
The commands do not change the store, so the stored ``tplx`` values stay as they are.

Then we increase the Udp header size:

.. literalinclude:: ../sources/UdpHeader.msg.mod
   :diff: ../sources/UdpHeader.msg.orig

The change needs to be followed in ``UdpHeaderSerializer.cc`` as well:
(otherwise the packets couldn't be serialized, leading to dropped packets)

.. literalinclude:: ../sources/UdpHeaderSerializer.cc.modified
   :diff: ../sources/UdpHeaderSerializer.cc.orig

We run the four simulations again with the calculated values:

.. code-block:: bash

  $ inet -u Cmdenv -c Ethernet -r 0 --sim-time-limit=0.2s --fingerprint=XXXX-XXXX/tpx
  $ inet -u Cmdenv -c EthernetShortPacket -r 0 --sim-time-limit=0.2s --fingerprint=XXXX-XXXX/tpx
  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=XXXX-XXXX/tpx
  $ inet -u Cmdenv -c WifiShortPacket -r 0 --sim-time-limit=5s --fingerprint=XXXX-XXXX/tpx

.. code-block:: fp

  Fingerprint mismatch! calculated: YYYY-YYYY/tpx, expected: XXXX-XXXX/tpx
  Fingerprint successfully verified
  Fingerprint mismatch! calculated: YYYY-YYYY/tpx, expected: XXXX-XXXX/tpx
  Fingerprint successfully verified

As expected, the fingerprints match when the packet size is small.

.. TODO why the two fingerprints didn't change -> for small packets, the fingerprints didn't change because described earlier. For normal/large ones, they did.

.. ----

.. **TODO** finish later
