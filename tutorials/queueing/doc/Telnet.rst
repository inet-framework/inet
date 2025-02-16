Example: Generating Telnet Traffic
==================================

This example demonstrates a telnet client (``TelnetClientTraffic``) and a telnet server (``TelnetServerTraffic``)
module built using queueing components.

These two modules are created by copying the :ned:`TelnetClientApp` and
:ned:`TelnetServerApp` modules available in INET, and omitting the :ned:`IApp`
interface. As such, the telnet traffic apps can be connected to each other
directly, without any sockets or protocols.

.. figure:: media/TelnetClientTraffic.png
   :align: center
   :width: 100%

.. figure:: media/Telnet.png
   :align: center
   :width: 100%

.. figure:: media/TelnetServerTraffic.png
   :align: center
   :width: 100%

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network TelnetTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: module TelnetClientTraffic
   :end-before: //----
   :language: ned

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: module TelnetServerTraffic
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Telnet
   :end-at: providingInterval
   :language: ini
