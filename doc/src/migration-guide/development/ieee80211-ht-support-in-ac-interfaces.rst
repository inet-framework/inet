IEEE 802.11 HT Support in ac Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``opMode = "ac"`` now enables existing HT support. HT management requires standard channel numbers for HT Operation elements. An ``ac`` AP on an arithmetic 5 GHz band therefore fails initialization, including simplified management. For standard channel 36 at 5180 MHz, use:

.. code-block:: ini

   **.wlan[*].opMode = "ac"
   **.wlan[*].radio.bandName = "5 GHz (mapped 20 MHz)"
   **.wlan[*].radio.channelNumber = 0
   **.wlan[*].radio.centerFrequency = 5.18GHz

Remove an earlier band override if it would take precedence over these settings. Preserve the required data rate, channel width, and spatial stream constraints. For centers outside the mapped subset, use a custom band with explicit standard channel conversion.

The supplied configurations now use index 0 at 5180 MHz:

.. list-table:: Effective initial center changes
   :header-rows: 1

   * - Configuration
     - Previous center
   * - ``tests/protocol/wifi/ini/_ac.ini``
     - 5040 MHz
   * - ``examples/wireless/lan80211ac/omnetpp.ini``
     - 5010 MHz
   * - ``tests/protocol/wifi/11ac/Ac_160mhz.test``
     - 5080 MHz

These are effective radio frequencies; the earlier ``centerFrequency = 5GHz`` setting did not make them 5000 MHz. The new frequencies can change propagation results and fingerprints. Added HT entries can also change ACK/CTS candidates and adaptive rate steps. See :ref:`ug:sec:80211:ac-ht` for the channel mapping, radio requirements, and current model limits.
