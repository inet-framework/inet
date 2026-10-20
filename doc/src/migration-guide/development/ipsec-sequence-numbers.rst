IPsec Sequence Numbers
~~~~~~~~~~~~~~~~~~~~~~

``SecurityAssociation::getAndIncSeqNum()`` returned the counter and then incremented it, so the
first packet of every SA carried the Sequence Number 0. RFC 4302 and RFC 4303 increment the counter
first, so that the first packet carries 1. The method is now
``SecurityAssociation::incrementAndGetSeqNum()``, and returns the incremented value. C++ code that
called the old method must use the new name, and expects one more than before.
