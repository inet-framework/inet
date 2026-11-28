AODV discovery errors
~~~~~~~~~~~~~~~~~~~~~

Applications may now receive native IP destination-unreachable indications when AODV exhausts originating discovery. Error callbacks must consume the indication instead of assuming that route discovery always fails silently. Update fixtures that intentionally exercise exhaustion to expect the error while preserving their packet-disposal assertions.

Successful discovery produces no error. Stop/crash cancellation and the existing local-repair failure path do not use this originating-discovery notification.
