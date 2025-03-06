Per-Stream Filtering and Policing
=================================

Per-stream filtering and policing is a key feature in Time-Sensitive Networking
(TSN) that enables fine-grained traffic management for deterministic
communication. It extends the IEEE 802.1Q traffic classification mechanisms to
provide control at the individual stream level.

Network devices use this capability to identify specific traffic flows and apply
appropriate policies, protecting against bandwidth violations, malfunctioning
devices, and network attacks. Filtering and policing decisions can be made on a
per-stream, per-priority, or per-frame basis using various metering methods.

The following showcases demonstrate per-stream filtering and policing:

.. toctree::
   :maxdepth: 1

   tokenbucket/doc/index
   statistical/doc/index
   underthehood/doc/index
