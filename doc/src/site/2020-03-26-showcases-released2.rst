:orphan:
:date: 2020-03-26
:category: news

==============================================================================================================
New 802.11 Showcases: Coexistence of IEEE 802.11 and 802.15.4; Crosstalk Between Adjacent IEEE 802.11 Channels
==============================================================================================================

Coexistence
~~~~~~~~~~~

Different wireless technologies sometimes operate in the same frequency range.
For example, both IEEE 802.11 and IEEE 802.15.4 have versions that use the 2.4 GHz ISM band.
As such, the signals of the two protocols can interfere.
This is called cross-technology interference, or CTI.

INET has support for simulating CTI between any of its wireless protocol models.
Simulation can be used to examine how the different protocols interact and affect
each other’s operation. This showcase demonstrates the coexistence of
INET’s 802.11 and 802.15.4 models.

Crosstalk
~~~~~~~~~

By default, 802.11 hosts and access points in INET are configured to use the same
Wifi channel (Channel 1.) In reality, however, it is rarely the case. There are many
Wifi networks at the same location, typically spread out on all channels (especially
in the case of 2.4 GHz Wifi where only a few channels are available, compared to 5 GHz.)
Transmissions on many adjacent channels also overlap in frequency and can cause interference.

INET has support for simulating communication on the different Wifi channels, both in the
2.4 GHz and 5 GHz frequency range. This showcase demonstrates using both overlapping and
non-overlapping Wifi channels in simulations, and how transmissions on different channels
interfere with each other.

Links:

- `Coexistence of IEEE 802.11 and 802.15.4 <https://inet.omnetpp.org/docs/showcases/wireless/coexistence/doc/>`__
- `Crosstalk Between Adjacent IEEE 802.11 Channels <https://inet.omnetpp.org/docs/showcases/wireless/crosstalk/doc/>`__
