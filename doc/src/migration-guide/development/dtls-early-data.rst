DTLS Early Data Callbacks
~~~~~~~~~~~~~~~~~~~~~~~~~

Every ``IDtls::ICallback`` implementation must add ``earlyDataArrived(id, packet)`` and ``earlyDataStatus(id, accepted)``. The first transfers packet ownership before completed peer authentication; discard it unless the application explicitly permits that operation. The second reports the client's admission outcome, not delivery of an individual datagram. Ordinary delivery still uses ``dataArrived``. ``DtlsSocketIo`` discards early delivery by default; set ``acceptEarlyData=true`` only with an explicit application policy.

Custom engines must implement the ``open`` overload taking ``DtlsOpenOptions`` and the ownership-transferring ``sendEarly`` operation. Existing open overloads retain their behavior. To request early data, enable both ``resumptionEnabled`` and ``earlyDataEnabled``, open with ``DtlsOpenOptions{true, true}``, and explicitly call ``sendEarly`` before the client's Finished handoff. A ticket must advertise a nonzero allowance. ``send`` never implicitly becomes an early operation. Rejected early data is not automatically resent; the application decides whether to retry after establishment.
