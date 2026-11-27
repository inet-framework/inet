IEEE 802.11 Basic Block Ack policy interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Protected Basic Block Ack exchanges can reserve time for a queued DATA frame
before the receiver returns its bitmap. Update custom implementations of the
following C++ interfaces and rebuild the library that contains them.

* In ``IOriginatorQosMacDataService``, implement ``previewBasicFirstMpdu`` to return an owned copy of
  the first fragment when fragmentation applies, or a null pointer when the
  caller can preview the original MPDU. Implement ``commitBasicFrames`` to
  revalidate the prepared sources, consume them and their sequence number
  only on success, and return the committed MPDU or its fragments in order.
  Return a null pointer when the prepared sources can no longer be committed.
