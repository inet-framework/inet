Queued datagram failure reporting
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Implementations of ``INetfilter`` must provide ``failQueuedDatagram(const Packet *)``. The operation consumes an existing queued obligation, reports the network protocol's native unreachable indication where supported, and disposes of the original packet. Remove the obligation before invoking the error path. A pointer that is not queued must be ignored without dereferencing it.

Keep ``dropQueuedDatagram()`` silent for cancellation and lifecycle cleanup. Protocols without a native error protocol may implement failure as silent disposal through that existing operation.
