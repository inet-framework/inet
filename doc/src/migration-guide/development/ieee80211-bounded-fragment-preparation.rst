IEEE 802.11 bounded fragment preparation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``IFragmentationPolicy`` implementations must implement
``computeBoundedFragmentSizes(frame, maxMpduBytes)``. The bound includes the
MAC header and FCS. Return an unfragmented frame, a legal partition, or an
impossible partition. Each nonfinal fragment must have the same even length.
A partition can contain at most sixteen fragments.
