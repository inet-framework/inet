IEEE 802.11 Block Ack Agreement Lifecycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom QoS recipient services must return accepted, complete Actions from ``managementFrameReceived()``. HCF owns their agreement dispatch after duplicate checks.

The recipient response now determines the buffer size and inactivity timeout. The built-in recipient selects the smaller of 64 and its configured maximum. A request size of zero supplies no size guidance. The response copies the request dialog token.

Custom agreement policies must implement ``isProfileAllowed()`` with the supplied typed profile and usable capability inputs. Basic admission must preserve its historical non-HT compatibility.

Custom subclasses of ``OriginatorQosMacDataService`` must adapt the protected ``aMsduAggregateIfNeeded(queue, selected)`` helper. The selected packet is the first eligible packet in service order. An aggregate candidate list must start with that packet. The service validates every candidate before source removal and calls the aggregation policy once.

The eligibility callback in ``OriginatorQosMacDataService::setEligibility()`` accepts ``const Packet *``. The service retains ordinary dequeue when the advertised next frame is eligible. Otherwise, it supplies an eligibility filter to ``canPullPacket()``. If the query finds no eligible frame, the service waits. If the query finds an eligible frame, ``pullPacket(gate, filter)`` transfers that frame through an identity filter.

Custom queues must preserve ordinary pull effects for filtered pulls, including ownership, residence metadata, events, statistics, and applicable animation. Built-in queues, priority schedulers, and compound queues already support these operations. The service propagates queue and filter errors. It does not replace a failed pull with collection removal. These operations transfer complete packets.

The QoS data service assigns an increasing allocation index before fragmentation. All fragments retain this index in ``Ieee80211DataSequenceTag``. Custom QoS data services must supply the same metadata. Custom originator code must call the new ``recordTransmittedData(allocationIndex)`` operation when the transmitter accepts DATA. The agreement exposes its highest transmitted index through the new ``getHighestTransmittedDataIndex()`` query. HCF supplies this integration for the built-in implementation.
