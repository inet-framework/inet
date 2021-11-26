:orphan:

.. _dg:cha:ethernet:

.. The Ethernet Model
   ==================

   Sending Ethernet Frames
   -----------------------

   TODO tags etc

   Receiving Ethernet Frames
   -------------------------

   TODO tags etc

   Frames
   ------

   The INET defines these frames in the :file:`EtherFrame.msg` file.
   The models supports Ethernet II, 803.2 with LLC header, and 803.3 with
   LLC and SNAP headers. The corresponding classes are:
   :msg:`EthernetIIFrame`, :msg:`EtherFrameWithLlc` and
   :msg:`EtherFrameWithSNAP`. They all class from :msg:`EtherFrame` which
   only represents the basic MAC frame with source and destination
   addresses. :ned:`EthernetCsmaMac` only deals with :msg:`EtherFrame`’s, and does
   not care about the specific subclass.

   Ethernet frames carry data packets as encapsulated cMessage objects.
   Data packets can be of any message type (cMessage or cMessage subclass).

   The model encapsulates data packets in Ethernet frames using the
   ``encapsulate()`` method of cMessage. Encapsulate() updates the
   length of the Ethernet frame too, so the model doesn’t have to take care
   of that.

   The fields of the Ethernet header are passed in a :cpp:`Ieee802Ctrl`
   control structure to the LLC by the network layer.

   EtherJam, EtherPadding (interframe gap), EtherPauseFrame?

   EtherLlc
   --------

   EtherFrameWithLLC

   SAP registration

   :ned:`Ieee8022Llc` and higher layers
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   The :ned:`Ieee8022Llc` module can serve several applications (higher layer
   protocols), and dispatch data to them. Higher layers are identified by
   DSAP. See section "Application registration" for more info.

   :ned:`EthernetEncapsulation` doesn’t have the functionality to dispatch to
   different higher layers because in practice it’ll always be used with
   IP.

   Communication between LLC and Higher Layers
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   Higher layers (applications or protocols) talk to the :ned:`Ieee8022Llc`
   module.

   When a higher layer wants to send a packet via Ethernet, it just passes
   the data packet (a cMessage or any subclass) to :ned:`Ieee8022Llc`. The
   message kind has to be set to IEEE802CTRL_DATA.

   In general, if :ned:`Ieee8022Llc` receives a packet from the higher layers,
   it interprets the message kind as a command. The commands include
   IEEE802CTRL_DATA (send a frame), IEEE802CTRL_REGISTER_DSAP (register
   highher layer) IEEE802CTRL_DEREGISTER_DSAP (deregister higher layer) and
   IEEE802CTRL_SENDPAUSE (send PAUSE frame) – see EtherLLC for a more
   complete list.

   The arguments to the command are NOT inside the data packet but in a
   "control info" data structure of class :cpp:`Ieee802Ctrl`, attached to
   the packet. See controlInfo() method of cMessage (OMNeT++ 3.0).

   For example, to send a packet to a given MAC address and protocol
   identifier, the application sets the data packet’s message kind to
   ETH_DATA ("please send this data packet" command), fills in the
   :ned:`Ieee802Ctrl` structure with the destination MAC address and the
   protocol identifier, adds the control info to the message, then sends
   the packet to :ned:`Ieee8022Llc`.

   When the command doesn’t involve a data packet (e.g.
   IEEE802CTRL_(DE)REGISTER_DSAP, IEEE802CTRL_SENDPAUSE), a dummy packet
   (empty cMessage) is used.

   Rationale
   ~~~~~~~~~

   The alternative of the above communications would be:

   -  adding the parameters such as destination address into the data
      packet. This would be a poor solution since it would make the higher
      layers specific to the Ethernet model.

   -  encapsulating a data packet into an *interface packet* which contains
      the destination address and other parameters. The disadvantages of
      this approach is the overhead associated with creating and destroying
      the interface packets.

   Using a control structure is more efficient than the interface packet
   approach, because the control structure can be created once inside the
   higher layer and be reused for every packet.

   It may also appear to be more intuitive in Tkenv because one can observe
   data packets travelling between the higher layer and Ethernet modules –
   as opposed to "interface" packets.

   EtherLLC: SAP Registration
   ~~~~~~~~~~~~~~~~~~~~~~~~~~

   The Ethernet model supports multiple applications or higher layer
   protocols.

   So that data arriving from the network can be dispatched to the correct
   applications (higher layer protocols), applications have to register
   themselves in :ned:`Ieee8022Llc`. The registration is done with the
   IEEE802CTRL_REGISTER_DSAP command (see section "Communication between
   LLC and higher layers") which associates a SAP with the LLC port.
   Different applications have to connect to different ports of
   :ned:`Ieee8022Llc`.

   The ETHERCTRL_REGISTER_DSAP/IEEE802CTRL_DEREGISTER_DSAP commands use
   only the dsap field in the :cpp:`Ieee802Ctrl` structure.

   EtherMac
   --------

   The operation of the MAC module can be schematized by the following
   state chart:

   .. graphviz:: figures/EtherMAC_txstates.dot
      :align: center

   Unlike :ned:`EthernetMac`, this MAC module processes the incoming
   packets when their first bit is received. The end of the reception is
   calculated by the MAC and detected by scheduling a self message.

   When frames collide the transmission is aborted – in this case the
   transmitting station transmits a jam signal. Jam signals are represented
   by a :msg:`EthernetJamSignal` message. The jam message contains the tree
   identifier of the frame whose transmission is aborted. When the
   :ned:`EthernetCsmaMac` receives a jam signal, it knows that the corresponding
   transmission ended in jamming and have been aborted. Thus when it
   receives as many jams as collided frames, it can be sure that the
   channel is free again. (Receiving a jam message marks the beginning of
   the jam signal, so actually has to wait for the duration of the
   jamming.)

   EtherMacFullDuplex
   ------------------

   Outgoing packets are transmitted according to the following state
   diagram:

   .. graphviz:: figures/EtherMACFullDuplex_txstates.dot
      :align: center

   EthernetInterface
   -----------------

   Queueing
   ~~~~~~~~

   When the transmission line is busy, messages received from the upper
   layer needs to be queued.

   In routers, MAC relies on an external queue module (see
   :ned:`OutputQueue`), and requests packets from this external queue
   one-by-one. The name of the external queue must be given as the
   :par:`queueModule` parameer. There are implementations of
   :ned:`OutputQueue` to model finite buffer, QoS and/or RED.

   In hosts, no such queue is used, so MAC contains an internal queue named
   :var:`txQueue` to queue up packets waiting for transmission.
   Conceptually, :var:`txQueue` is of infinite size, but for better
   diagnostics one can specify a hard limit in the :par:`txQueueLimit`
   parameter – if this is exceeded, the simulation stops with an error.

   .. _subsec:pause_handling:

   PAUSE handling
   ~~~~~~~~~~~~~~

   The 802.3x standard supports PAUSE frames as a means of flow control.
   The frame contains a timer value, expressed as a multiple of 512
   bit-times, that specifies how long the transmitter should remain quiet.
   If the receiver becomes uncongested before the transmitter’s pause timer
   expires, the receiver may elect to send another PAUSE frame to the
   transmitter with a timer value of zero, allowing the transmitter to
   resume immediately.

   :ned:`EthernetCsmaMac` will properly respond to PAUSE frames it receives
   (:msg:`EtherPauseFrame` class), however it will never send a PAUSE frame
   by itself. (For one thing, it doesn’t have an input buffer that can
   overflow.)

   :ned:`EthernetCsmaMac`, however, transmits PAUSE frames received by higher
   layers, and :ned:`Ieee8022Llc` can be instructed by a command to send a
   PAUSE frame to MAC.

   Error handling
   ~~~~~~~~~~~~~~

   If the MAC is not connected to the network ("cable unplugged"), it will
   start up in "disabled" mode. A disabled MAC simply discards any messages
   it receives. It is currently not supported to dynamically
   connect/disconnect a MAC.

   CRC checks are modeled by the :var:`bitError` flag of the packets.
   Erronous packets are dropped by the MAC.

