TCP Connection Setup
~~~~~~~~~~~~~~~~~~~~

The timer ``connEstabTimer`` of ``TcpConnection`` is now ``deferredSynTimer``, and the constant
``TCP_TIMEOUT_CONN_ESTAB`` is now ``TCP_TIMEOUT_DEFERRED_SYN``. The timer only waits for the SEND
of a deferred Fast Open SYN. It no longer limits a connection setup: the SYN and SYN-ACK
retransmissions do (``synRetries``). When they run out, the connection raises the event
``TCP_E_TIMEOUT_CONN_ESTAB``, as the timer did before, so a subclass that handles this event needs no
change.
