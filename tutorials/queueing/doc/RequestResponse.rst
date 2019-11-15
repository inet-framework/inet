Request Response
================

This test contains a simplified version of a client/server and a request/response
based communication.

Request packets are produced periodically and randomly by an active packet
source (ActivePacketSource) in the client. The generated requests fall into one of
two categories based on the data they contain.

The server processes requests in order, one by one, using a compound consumer
(RequestConsumer). Each request is first classified based on the data it contains,
and then a certain number of tokens are generated as the request is consumed.

The tokens are added to a response server in a compound producer (ResponseProducer).
The response producer generates different traffic randomly over a period of time
for each kind of request.

The client consumes the response packets by a passive packet sink (PassivePacketSink).

TODO

The network contains ... TODO

.. figure:: media/ResponseProducer.png
   :width: 70%
   :align: center

.. figure:: media/RequestResponse.png
   :width: 50%
   :align: center

.. figure:: media/RequestConsumer.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../RequestResponse.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config RequestResponse
   :end-at: provider[1].providingInterval
   :language: ini
