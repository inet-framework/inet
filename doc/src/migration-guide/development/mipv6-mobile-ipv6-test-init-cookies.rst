Mobile IPv6 Test Init Cookies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``Mipv6`` gives every Home Test Init and Care-of Test Init it sends a new
random 64-bit cookie. ``BindingUpdateList::addOrUpdateBUL()`` takes the cookie as
``uint64_t`` instead of ``int``, and the ``cookieHoTI`` and ``cookieCoTI``
fields of ``BindingUpdateListEntry`` are ``uint64_t``. A subclass that
overrides ``addOrUpdateBUL()`` must change the type of its cookie parameter, or
it no longer overrides the method.
