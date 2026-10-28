IEEE 802.11 VHT Mode Queries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The physical layer (PHY) mode interface ``IIeee80211Mode`` gains two pure
virtual methods. Direct implementations outside INET must implement them:

.. code-block:: c++

   int getVhtMcsIndex() const override;
   bool isVhtShortGuardInterval() const override;

``getVhtMcsIndex()`` returns the VHT MCS index of the mode, or -1 for a mode
of another PHY generation. VHT means Very High Throughput.
``isVhtShortGuardInterval()`` returns true if the mode uses the optional
400 ns guard interval, and false for the 800 ns guard interval and for a mode
of another PHY generation. The two methods mirror ``getHtMcsIndex()`` and
``isHtShortGuardInterval()``.

Subclasses of ``Ieee80211ModeBase`` inherit -1 and false and need no new
override to compile. A custom VHT mode overrides both methods.
