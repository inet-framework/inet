A-MSDU policy prefix evaluation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``BasicMsduAggregationPolicy`` evaluates queue and vector inputs through one
private lazy prefix evaluator. The queue overload no longer calls the virtual
vector overload. A subclass overriding only
``computeAggregateFrames(const std::vector<Packet *>&)`` must also override
``computeAggregateFrames(queueing::IPacketQueue *)`` if it needs custom
selection for queue inputs.

The public signatures and the protected virtual ``isEligible`` and
``isAggregationPossible`` hooks remain available. Subclasses using those
protected hooks retain their selection customization. Results remain
caller-owned vectors of borrowed packet pointers; selection does not remove
or modify packets.
