IEEE 802.11 Basic Block Ack Receipt Records
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Code that constructs a ``BlockAckRecord`` directly must supply the agreement's
starting sequence number as the third constructor argument. The recipient uses
it to distinguish missing Basic Block Ack fragments from retired sequence positions.

The two-argument constructor is removed without a deprecation period because it has
no agreement state from which to recover the negotiated sequence number. A fallback
would let Basic Block Ack callers create a record with the wrong receipt boundary,
which can acknowledge fragments that never arrived.
