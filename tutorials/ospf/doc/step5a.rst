Step 5a. Mismatched Parameters between two OSPF neighbors
=========================================================

Goals
-----

The goal of this step is to demonstrate that OSPF routers will not form a full adjacency
if certain parameters are mismatched.

For OSPF neighbors to successfully form an adjacency, several parameters must match:

*   **Area ID**: Must be the same on both sides of the link.
*   **Hello interval and Dead interval**: Must match.
*   **Authentication**: Type and keys must match.
*   **Network type**: While not always required to match exactly, mismatches can prevent
    proper adjacency formation.
*   **Stub area flag**: Must match if the area is configured as a stub.

When these parameters mismatch, routers may get stuck in states like 2-Way or Init, unable
to reach the Full state.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 5. The OSPF configuration file introduces a parameter
mismatch:

*   **R1 and R2**: Mismatched Hello intervals

.. figure:: media/InterfaceNetworkType.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5a
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_mismatch.xml
   :language: xml

Results
~~~~~~~

The simulation demonstrates adjacency formation failures:

1.  **R1 and R2**: Because they have different Hello intervals, they cannot properly
    synchronize their neighbor discovery. They may detect each other but fail to maintain
    a stable adjacency or have timing issues.

.. 2.  **R4 and R5**: With mismatched network types, they have incompatible views of how
..     the link should operate (e.g., one expecting DR election, the other not), preventing
..     proper adjacency formation. TODO they have full adjacency, but they shouldn't

The OSPF module logs show the routers detecting neighbors but failing to reach the Full
state. The routing tables reflect the missing adjacencies - routes that would normally
use these links are either absent or use alternative paths.

This step highlights the importance of consistent OSPF configuration across interfaces
forming an adjacency.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`InterfaceNetworkType.ned <../InterfaceNetworkType.ned>`,
:download:`ASConfig_mismatch.xml <../ASConfig_mismatch.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
