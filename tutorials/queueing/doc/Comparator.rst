Ordering the Packets in the Queue
=================================

The :ned:`PacketQueue` module can order the packets it contains using a packet
comparator function, which makes it suitable for implementing priority
queueing. (Other ways of implementing priority queueing, for example using
:ned:`PriorityScheduler`, will be covered in later steps.)

In this example network, an active packet source (:ned:`ActivePacketSource`) creates
1-byte packets every second with a random byte as content.
The packets are pushed into a queue (:ned:`PacketQueue`). The queue is configured
to use the ``PacketDataComparator`` function, which orders packets by data.
The packets are popped from the queue by an active packet sink (:ned:`ActivePacketSink`)
every 2 seconds.

During simulation, the producer creates packets more frequently than the
collector pops them from the queue, so packets accumulate in the queue. When
this happens, packets will be ordered in the queue, so the collector will receive
a series of ordered sequences.

.. figure:: media/Comparator.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network ComparatorTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Comparator
   :end-at: comparatorClass
   :language: ini

.. The following screenshots demonstrate the queue's contents at the end of the simulation,
   first without, then with the comparator function:

   .. figure:: media/nocomparator.png
      :width: 90%
      :align: center

   .. figure:: media/comparator.png
      :width: 90%
      :align: center

The following screenshot demonstrates the queue's contents at the end of the simulation
without the comparator function:

.. figure:: media/nocomparator.png
   :width: 90%
   :align: center

And this one using the comparator function (head at the top, tail at the bottom):

.. figure:: media/withcomparator.png
   :width: 90%
   :align: center
