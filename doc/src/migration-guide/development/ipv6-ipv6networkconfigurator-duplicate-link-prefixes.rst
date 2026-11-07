Ipv6NetworkConfigurator Duplicate Link Prefixes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A configuration of :ned:`Ipv6NetworkConfigurator` that gives two links the same prefix now stops
initialization with ``The link of <interface> and the link of <interface> both get the prefix
<prefix>, but each link needs a unique prefix``. A ``prefix`` template without an ``x`` group used
to give its prefix to every link it matched, without an error. Restrict such a template to one link
with ``hosts``, ``towards`` or ``among``, or put an ``x`` group into it, for example
``prefix='2001:db8:1:x::/64'``. Two ``address`` attributes in one /64 on different links are rejected
the same way; give one of them an address in another prefix.
