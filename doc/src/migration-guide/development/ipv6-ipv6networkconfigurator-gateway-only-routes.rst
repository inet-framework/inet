Ipv6NetworkConfigurator Gateway-Only Routes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A ``<route>`` element of the :ned:`Ipv6NetworkConfigurator` configuration that
names a ``gateway`` and no ``interface`` used to be ignored. It is now
installed on the node's interface on the link where the configurator assigned
the gateway address. When no link of the node carries that address,
initialization stops with ``Host/router <node> has no interface towards
"<gateway>"``, for example for a link-local gateway, or for a gateway given as a
module name, which resolves to a link-local address.

Add ``interface="<name>"`` to such an element.
