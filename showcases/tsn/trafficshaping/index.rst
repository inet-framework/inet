Scheduling and Traffic Shaping
==============================

.. Traffic scheduling and shaping aim to achieve bounded low latency and zero
   congestion loss. Scheduling and shaping decisions are made on a per-stream,
   per-priority, per-frame, etc. basis using various methods.

Traffic shaping alters traffic patterns so they comform to some requirements, such as
bounded low latency, maximum jitter, maximum data rate and burst size.
Traffic shapers delay the forwarding of incoming packets, as opposed to filtering that
drops packets to make the incoming stream comform to specifications.
Scheduling and shaping decisions are made on a per-stream,
per-priority, per-frame, etc. basis using various methods.

.. Traffic shaping usually happens in the MAC layer of Ethernet interfaces of network switches.

Traffic shaping happens in the MAC layer of Ethernet interfaces (whereas filtering is done in the bridging layer).

.. **TODO** in interfaces (as opposed to policing)

The following showcases demonstrate scheduling and traffic shaping:

.. toctree::
   :maxdepth: 1

   timeawareshaper/doc/index
   creditbasedshaper/doc/index
   asynchronousshaper/doc/index
   cbsandats/doc/index
   cbsandtas/doc/index
   underthehood/doc/index

.. cbsandtas/doc/index
.. tokenbucketbasedshaper/doc/index
.. htbshaper/doc/index
