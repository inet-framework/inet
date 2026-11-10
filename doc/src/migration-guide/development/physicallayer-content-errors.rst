Content Errors in the Physical Layer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Failed Regions of a Reception Decision
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:cpp:`IReceptionDecision` has a new pure virtual method:

.. code-block:: c++

   virtual const std::vector<ErrorRegion>& getErrorRegions() const = 0;

It returns the regions of the transmitted packet that the decision found
erroneous. An :cpp:`ErrorRegion` has the offset and the length of the region in
the transmitted packet, and the error rate of one bit of the region. An
:cpp:`IReceptionDecision` implementation outside INET no longer compiles until
it overrides the method; it can return an empty list. :cpp:`ReceptionDecision`
takes the regions as an optional last constructor argument.

:cpp:`ReceiverBase::computeReceptionResult()` now calls the new protected
overload :cpp:`computeReceivedPacket(snir, decisions)`, which calls the overload
with the success flag as before. A receiver that needs the decisions for the
received packet overrides the new overload.
