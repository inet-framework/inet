Ipv6NetworkConfigurator Generated Prefixes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A prefix that :ned:`Ipv6NetworkConfigurator` generates from a ``prefix`` template with an ``x``
group now avoids every prefix that an ``address`` attribute fixes on another link. Before this
change, a template link that came earlier in topology order than such a link could get the same
prefix. That template link, and every link after it that uses the same template, now get other
prefixes, so the global addresses on those links change. Update any address of those links that a
configuration writes out in full, for example the ``destAddr`` of an application.
