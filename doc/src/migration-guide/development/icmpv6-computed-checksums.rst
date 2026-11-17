ICMPv6 Computed Checksums
~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ICMPv6 compositions must connect ``Icmpv6.networkProtocolModule`` to the
IPv6 module implementing ``INetfilter``. The post-routing hook computes the
checksum after address selection and before source fragmentation. Computed
receive packets need the actual source and destination in ``L3AddressInd``;
checksums over ICMPv6 bytes alone are no longer accepted. Update custom byte
vectors to include the IPv6 pseudo-header and the complete message. Declared
checksum modes retain their previous behavior. ICMPv6 error quotations now fit
within 1280 bytes including the outer IPv6 header.
