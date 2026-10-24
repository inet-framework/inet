IEEE 802.11 signal extension
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom IEEE 802.11 modes must implement ``getSignalExtensionTime()``. ``Ieee80211ModeBase`` supplies zero. ERP and 2.4 GHz HT modes supply their six-microsecond extension. HT duration now includes that extension, which can change airtime and simulation results. The transmission query derives the actual extension from its waveform, so consumers must not add the nominal value again.
