IEEE 802.11 QoS DATA Lifetime
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HCF enforces a finite lifetime for every QoS DATA frame. Each access category uses ``mac.hcf.edca.edcaf[*].msduLifetime``, with a default of 0.512 seconds. This lifetime applies with Block Ack support enabled or disabled. HCF starts the timer at MAC acceptance, before queue insertion and sequence assignment. Queue residence can therefore cause disposal before the first transmission.

Set a positive, finite value for each required access category. Zero, negative, and nonfinite values fail at initialization. A value that rounds to zero at the simulation time resolution also fails. A long finite value permits compatibility studies without an unlimited mode:

.. code-block:: ini

   **.mac.hcf.edca.edcaf[*].msduLifetime = 100s

This value prevents lifetime expiration for newly accepted DATA during a ten-second run. Acknowledgment, queue loss, and retry-limit disposal can still resolve DATA earlier. Recovery that depends on expiration inherits the configured delay. The value does not guarantee unchanged fingerprints or delivery results.

A-MSDU construction restarts the aggregate timer when each MSDU joins the aggregate. The current builder adds all constituents at one simulation time. Fragments retain the source MSDU deadline. Retransmission and ACK-policy changes do not restart a deadline. An accepted exchange can complete after expiration. HCF retains its referenced DATA until safe completion, but permits no later attempt after expiration.

The concrete ``EdcaTransmitLifetimeHandler`` now uses local packet identities instead of sequence numbers. External C++ consumers must use its registration and replacement operations with packet identities. EDCA owns the handler. The handler cannot be copied. The DCF lifetime interface remains unchanged. This change does not add management lifetime or MAC stop/start recovery.
