IEEE 802.11 bounded fragment preparation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``IFragmentationPolicy`` implementations must implement
``computeBoundedFragmentSizes(frame, maxMpduBytes)``. The bound includes the
MAC header and FCS. Return an unfragmented frame, a legal partition, or an
impossible partition. Each nonfinal fragment must have the same even length.
A partition can contain at most sixteen fragments.

Custom QoS data services must implement fragment preparation, validation and
commitment. Preparation retains the source and owns all previews. Commitment
transfers the previews with the source sequence, allocation and absolute
deadline. Check the partition status before commitment. A numbered source
must still be live and have no local DATA transmission.

Custom ACK handlers must expose ``dropFrame()`` so that replacement of a
numbered source can retire the original ACK entry.
