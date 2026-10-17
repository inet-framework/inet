IEEE 802.11 signal extension
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom IEEE 802.11 modes must implement ``getSignalExtensionTime()``. ``Ieee80211ModeBase`` supplies zero. ERP and 2.4 GHz HT modes supply their six-microsecond extension. HT duration now includes that extension, which can change airtime and simulation results.

The radio supplies actual received extension through ``Ieee80211ModeInd::signalExtensionDuration``. It derives this value from local reception duration, decoded PHY length, and the selected mode. Zero and the nominal extension are supported. A negative value means unavailable. Custom radios must leave the value unavailable when local evidence cannot establish it. A consumer must not substitute the nominal mode value for unavailable actual evidence.
