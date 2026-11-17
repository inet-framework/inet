Radio medium interference observation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

External implementations of ``IRadioMedium`` must implement
``bool supportsInterferenceObservation() const``. The pure virtual query is
available after initialization. Return true only when relevant modeled
interference arrivals reach observing radios in receiver or transceiver mode
without listenability, destination-address or communication-range-only
delivery filtering. Interference-range filtering is permitted. Return false
when the implementation cannot provide this guarantee.

The query does not promise successful packet decoding or analog-model
accuracy. It does not cover periods when a radio cannot observe, including
mode switching or sleep; the radio owns any resulting uncertainty in its
idle history. ``RadioMedium`` supplies the implementation for its subclasses
using its effective initialized filter configuration, without changing
delivery behavior or configuration defaults.

Subclasses of ``RadioMedium`` that override arrival delivery or filtering
must also override the query whenever the inherited answer no longer
describes their behavior.
