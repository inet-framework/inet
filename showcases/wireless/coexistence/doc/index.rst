Coexistence of IEEE 802.11 and 802.15.4
=======================================

Goals
-----

Wireless technologies such as IEEE 802.11 (Wi-Fi) and IEEE 802.15.4 (Zigbee)
often operate in the same frequency range, which can lead to cross-technology
interference (CTI) between the signals of the two protocols, affecting the
performance of both. In the case of 802.11 and 802.15.4, both
technologies have versions that use the 2.4 GHz Industrial, Scientific, and
Medical (ISM) band, which can result in CTI between the two protocols.

INET has support for simulating CTI between any of its
wireless protocol models, including 802.11 and 802.15.4. This allows users to
examine how the different protocols interact and affect each other's operation
in a simulated environment.

In this showcase, we will demonstrate the
coexistence of INET's 802.11 and 802.15.4 models and examine how they interact
with each other. By the end of this showcase, you will understand how these two
protocols can coexist in the same frequency range and the impact of CTI on their
operation.


| INET version: ``4.2``
| Source files location: `inet/showcases/wireless/coexistence <https://github.com/inet-framework/inet/tree/master/showcases/wireless/coexistence>`__

The Model
---------

We'll examine how CTI can be simulated, see if two interfering wireless technology
models can cooperate, and if their cooperation is balanced. We'll run the example
simulation with both 802.11 and 802.15.4 models present, and measure their
performance. Also, to get a baseline of their performance, we'll run the simulation
with both models, with just one of them present at a time. Then, we can compare the
baseline performance of both models to their concurrent performance.

The example simulation features a Wifi (802.11) and a WPAN (802.15.4) network close
to each other. All nodes communicate in the 2.4 GHz band. The signals
for the two wireless protocols have different center frequencies and bandwidths,
but the signal spectra can overlap. In this showcase, we will configure the two
networks to actually use overlapping channels. The channel spectra for both
technologies are shown on the following image:

.. figure:: media/channels.png
   :width: 100%
   :align: center

For the WPAN, we'll use INET's 802.15.4 narrowband version, in which transmissions
have a 2450 MHz carrier frequency and a 2.8 MHz bandwidth by default. For the Wifi,
we'll use 802.11g, in which transmissions have a 20 MHz bandwidth. We'll leave the
frequency and bandwidth of 802.15.4 on default, and we'll use Wifi Channel 9
(center frequency of 2452 MHz) so that the Wifi and WPAN transmission spectra
overlap:

.. figure:: media/channels8.png
   :width: 30%
   :align: center

In INET, a radio signal, as a physical phenomenon, is represented by the
analog model while it is being transmitted, propagated, and received.
As the signal center frequencies and bandwidths of the 802.11 and 802.15.4
models are not identical, the dimensional analog model needs to be used,
instead of the scalar analog model. The scalar analog model represents
signals with a scalar signal power, and a constant center frequency and
bandwidth. The scalar model can only handle situations when the spectra
of two concurrent signals are identical or don't overlap at all. When using
the dimensional analog model, signal power can change in both time and frequency;
more realistic signal shapes can be specified. This model is also able to
calculate the interference of signals whose spectra partially overlap.

In order for the signals of Wifi and WPAN to interfere,
the two networks have to share a radio medium module instance.
The radio medium module keeps track of transmitters, receivers, transmissions
and noise on the network, and computes signal and noise power at reception.
The radio medium module has several submodules, such as signal propagation,
path loss, background noise, and analog signal representation modules.
(For more information, read the :doc:`corresponding section </users-guide/ch-transmission-medium>` in the INET User's Guide.)

The standard radio medium module in INET is :ned:`RadioMedium`. Wireless protocols
in INET (such as 802.11 or 802.15.4) often have their own radio medium module types
(e.g. :ned:`Ieee80211DimensionalRadioMedium`), but these modules are actually
:ned:`RadioMedium`, just with different default parameterizations (each
parameterized for its typical use case). For example, they might have different
defaults for path loss type, background noise power, or analog signal representation
type. However, setting these radio medium parameters are not required for the
simulation to work. Most of the time, one could just use RadioMedium with its
default parameters (with the exception of setting the analog signal representation
type to dimensional when simulating CTI).

For our simulation, we'll use :ned:`RadioMedium`. Since we'll have two different
protocols, the analog model and the background noise of the radio medium and the
protocol specific radios need to match (they need to be dimensional).
We'll set just these two parameters, and leave the others on default.

In INET, different types of radio modules (e.g. 802.11 and 802.15.4) can detect
each other's transmissions, but can only receive transmissions of their own type.
Transmissions belonging to the other technology appear to receivers as noise.
However, this noise can be strong enough to make a node defer from transmitting.
As such, the fact that a radio treats transmissions it cannot receive as noise
is the mechanism that allows any wireless protocol models to interfere with each
other.

In reality and INET, both 802.11 and 802.15.4 employ the Clear Channel Assessment
(CCA) technique (they listen to the channel to make sure there are no ongoing
transmissions before starting to transmit) and defer from transmitting for the
duration of a backoff period when the channel is busy. The use of CCA and backoff
enables the two technologies to coexist cooperatively (both of them can communicate
successfully), as opposed to destructively (they ruin each other's communication).
In our case, the nodes of the different technologies sense when the other kind is
transmitting, and tend not to interrupt each other.

Here are some duration values in the example simulation, for both 802.11 and
802.15.4 (sending 1000B application packets with 24 Mbps, and 88B application
packets with 250 kbps, respectively):

+--------------+----------+-----------+
|              | 802.11   | 802.15.4  |
+==============+==========+===========+
| Data         | 382 us   | 4192 us   |
+--------------+----------+-----------+
| SIFS         | 10 us    | 192 us    |
+--------------+----------+-----------+
| ACK          | 34 us    | 352 us    |
+--------------+----------+-----------+
| Backoff (avg)| 600 us   | 1200 us   |
+--------------+----------+-----------+

An 802.15.4 data frame transmission takes about ten times more than an 802.11 one,
even though the payload is about ten times smaller. The SIFS and ack together are
also about ten times longer in 802.15.4. The relative duration of transmissions of
the Wifi and the WPAN is illustrated with the sequence chart below. The chart shows
a packet transmission and ACK, first for the Wifi and then for the WPAN. The scale
is linear.

.. figure:: media/seqchart.png
   :width: 100%
   :align: center

Transmissions are protected within a particular wireless technology, i.e. nodes
receiving a data frame can infer how long the transmission of the data frame and
the subsequent ACK will be, from the data frame's MAC header. They assume the
channel is busy for the duration of the DATA + SIFS + ACK (thus they don't start
transmitting during the SIFS). However, this protection mechanism doesn't work
with the transmissions of other technologies, since nodes cannot receive and make
sense of the MAC header. They just detect some signal power in the channel that
makes them defer for the duration of a backoff period (but this duration is
independent of the actual duration of the ongoing transmission). Also, neither
technology performs CCA before sending an ACK. Thus they are susceptible for
transmitting into each others' ACKs, which can lead to more retransmissions.

Also, the hidden node protection mechanism in 802.11 relies on the successful
reception of RTS and CTS frames, so hidden node protection might not work in a
multi-technology wireless environment.

Configuration
~~~~~~~~~~~~~

The simulation uses the ``CoexistenceShowcase`` network,
defined in :download:`CoexistenceShowcase.ned <../CoexistenceShowcase.ned>`:

.. figure:: media/network2.png
   :width: 100%
   :align: center

The network contains four :ned:`AdhocHost`'s. Two of the hosts, ``wifiHost1``
and ``wifiHost2``, communicate via 802.11, in ad hoc mode. The other two hosts,
``wpanHost1`` and ``wpanHost2``, communicate via 802.15.4. The four hosts are
arranged in a rectangle, and all of them are in communication range with each other
(corresponding hosts are 20 meters apart). One host in each host pair sends frames
to the other (``wifiHost1`` to ``wifiHost2``, and ``wpanHost1`` to ``wpanHost2``).

.. The simulation is defined in the ``Coexistence`` configuration in
.. :download:`omnetpp.ini <../omnetpp.ini>`. 

The ``General`` configuration in :download:`omnetpp.ini <../omnetpp.ini>` contains the
radio, radio medium and visualizer settings.
The radio medium module in the network
is a :ned:`RadioMedium`. It is configured to use the :ned:`DimensionalAnalogModel`.
The background noise type is set to :ned:`IsotropicDimensionalBackgroundNoise`,
with a power of -110 dBm. Here is the radio medium configuration in
:download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: radioMedium.analogModel.typename
   :end-at: radioMedium.backgroundNoise.power
   :language: ini

The Wifi hosts are configured to have :ned:`Ieee80211DimensionalRadio`.
The default ``timeGains`` parameter is not changed in the transmitter,
so the radio uses a flat signal in time. In frequency, instead of the
default flat signal, we configure a more realistic shape.
We'll use the 802.11 OFDM spectral mask, as in the standard:

.. figure:: media/spectralmask_wifi.png
   :width: 100%
   :align: center

Here is the ``frequencyGains`` parameter value specifying this spectrum:

.. literalinclude:: ../omnetpp.ini
   :start-at: wifiHost*.wlan[*].radio.transmitter.frequencyGains
   :end-at: wifiHost*.wlan[*].radio.transmitter.frequencyGains
   :language: ini

Briefly about the syntax:

- The parameter uses frequency and gain pairs to define points on the frequency/gain graph. ``c`` is the center frequency and ``b`` is the bandwidth. These values are properties of the transmission, i.e. the receiver listens on the frequency band defined by the center frequency and bandwidth. However, the signal can have radio energy outside of this range, which can cause interference.
- Between these points, the interpolation mode can be specified, e.g. ``left`` (take value of the left point), ``greater`` (take the greater of the two points), ``linear``, etc.
- The ``-inf Hz/-inf dB`` and the ``+inf Hz/+inf dB`` points are implicit (hence the ``frequencyGains`` string starts with an interpolation mode).

For more on the syntax, see :ned:`DimensionalTransmitterBase`.

Also we set the :par:`snirThresholdMode` parameter in the radio's receiver module,
and the :par:`snirMode` parameter in the error model to ``mean``:

.. literalinclude:: ../omnetpp.ini
   :start-at: snirThresholdMode
   :end-at: snirMode
   :language: ini

In the receiver module, reception of frames under the SNIR threshold is not
attempted (they're discarded). The :par:`snirThresholdMode` specifies how the
SNIR threshold is calculated. The parameter's value is either ``mean`` or
``min``, i.e. either take the `minimum` or the `mean` of the SNIR during
the reception.

The SNIR is important for calculating reception; the error model uses it
to decide if the reception was successful. In the error model, the
:par:`snirMode` parameter specifies how the SNIR is computed when the
receiver attempts to receive a frame; also either ``min`` or ``mean``.
When calculating with the minimum of the SNIR during the reception, a short
spike in the interfering signal might ruin the reception, as it can decrease
the SNIR substantially. Inversely, when two signals overlap substantially
but not entirely (in either time or frequency), the mean SNIR might not be
low enough to ruin the reception (when in this case it would be more realistic
if it did).

We set the :par:`snirMode` to ``mean`` because concurrent Wifi and WPAN signals
don't overlap significantly in the time-frequency space. That is, the WPAN frame's
spectrum is much smaller than the Wifi's; similarly, the Wifi frame is much shorter
than the WPAN.

The Wifi channel is set to Channel 9 (center frequency of 2452MHz) to ensure that
the Wifi transmissions overlap with the 802.15.4 transmissions in frequency.
Here is the configuration for the Wifi host radios in
:download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Ieee80211DimensionalRadio
   :end-at: channelNumber
   :language: ini

.. note:: The channel number is set to 8 because in INET's 802.11 model, the channels are numbered from 0, so that this setting corresponds to Wifi Channel 9.

The WPAN hosts are configured to have an :ned:`Ieee802154NarrowbandInterface`,
with a :ned:`Ieee802154NarrowbandDimensionalRadio`. As in the case of the Wifi hosts,
the default flat signal shape is used in time. In frequency, we'll use a more
realistic shape, based on the modulated spectrum of the CC2420 Zigbee transmitter:

.. figure:: media/spectrum_wpan.png
   :width: 100%
   :align: center

We'll use the approximation indicated with red. Here is the ``frequencyGains``
parameter value specifying this spectrum:

.. literalinclude:: ../omnetpp.ini
   :start-at: wpanHost*.wlan[*].radio.transmitter.frequencyGains = "left c-5MHz
   :end-at: wpanHost*.wlan[*].radio.transmitter.frequencyGains = "left c-5MHz
   :language: ini

The default carrier frequency (2450 MHz) and bandwidth (2.8 MHz) is not changed.
Here is the configuration for the WPAN host radios in
:download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Ieee802154NarrowbandInterface
   :end-at: Ieee802154NarrowbandDimensionalRadio
   :language: ini

The transmission power parameters for both technologies are left on default
(20 mW for the Wifi and 2.24 mW for the WPAN). The Wifi hosts operate in the
default 802.11g mode and use the default data bitrate of 24 Mbps. The WPAN
hosts use the default bitrate of 250 kbps (specified in :ned:`Ieee802154Mac`).

The traffic for the Wifi hosts is contained in the ``WifiHosts`` configuration.
``wifiHost1`` is configured to send a 1000-byte UDP packet to ``wifiHost2``
every 0.4 milliseconds, corresponding to about 20 Mbps traffic, saturating
the Wifi channel. Here is the Wifi traffic configuration in
:download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config WifiHosts
   :end-at: wifiHost2.app[*].localPort = 5000
   :language: ini

The traffic for the Wpan hosts is contained in the ``WpanHosts`` configuration.
``wpanHost1`` is configured to send an 88-byte UDP packet to ``wpanHost2``
every 0.1 seconds, which is about 7 Kbps of traffic
(the packet size is set to 88 bytes in order not to exceed the default
maximum transfer unit in 802.15.4).
Here is the WPAN traffic configuration in :download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config WpanHosts
   :end-at: wpanHost2.app[0].localPort = 5000
   :language: ini

The independent performance data can be obtained by running the ``WifiHosts``
and the ``WpanHosts`` configurations in :download:`omnetpp.ini <../omnetpp.ini>`,
as they each contain just one type of hosts (Wifi or Wpan).

To specify the case where they both coexist, the ``Coexistence`` configuration extends both ``WifiHosts`` and ``WpanHosts``:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Coexistence
   :language: ini

In all configurations, the simulations are run for five seconds and repeated
eight times.

Results
-------

There is contention between the Wifi and WPAN hosts so that it is expected that
they will be able to coexist cooperatively. However, protection mechanisms don't
work between the two technologies, so it is likely that they'll transmit into
each other's transmissions, and the performance of both will be degraded. The
Wifi host waits less than the WPAN host before accessing the channel, so it is
expected that the Wifi host will gain channel access most of the time. The Wifi
might even starve the WPAN.

WPAN transmissions are significantly longer than Wifi transmissions, thus, when
the WPAN host does gain channel access, it'll take lots of air time from the Wifi
host during the transmission of a single 802.15.4 frame. However, the WPAN host
sends frames much less frequently than the Wifi host, so it can afford to wait
for channel access; its performance might be mostly unaffected. Also, the Wifi
host's transmission power is significantly greater than the WPAN host's, so that
when the two transmit concurrently, the Wifi transmission might be correctly
receivable but the WPAN transmission might be corrupted.

The simulation can be run by choosing the ``Coexistence`` configuration from
omnepp.ini. It looks like the following when the simulation is run:

.. video:: media/coexistence9.mp4
   :width: 100%
   :align: center

The Wifi hosts have the MAC contention state, and the WPAN hosts have the
MAC state displayed above them, using :ned:`InfoVisualizer`. The spectrum
and power of received signals is visualized with a spectrum figure (part
of :ned:`MediumVisualizer`) at all hosts. The spectrum figure displays the
sum of all signals present at the receiving node. Signals not being received
are indicated with blue. When receiving a signal, the received signal is
indicated with green; the sum of interfering signals are indicated with red.
Note that the spectrum figure configures its scale automatically based on the
signals displayed; also, all spectrum figures use the same scale so that the
signal spectra and power levels can be compared.

The Wifi and WPAN hosts detect each others' transmissions (but cannot receive them),
and this causes them to defer from transmitting. Sometimes they transmit at the same
time, and the transmissions interfere. However, the interference doesn't ruin the
receptions.

The reason is that the overlap in the time-frequency space is not large.
The two transmissions overlap in frequency, but it is not substantial from
the perspective of the Wifi (the WPAN transmission's spectrum is small compared
to the Wifi's). Similarly, the transmissions overlap in time, but the overlap is
not substantial from the perspective of the WPAN (the Wifi transmission's duration
is small compared to the WPAN's). (Despite the low time-frequency space overlap,
if the signal power for one of the transmissions were significantly higher than
the other, the lower power transmission might not be correctly receivable.)

In the video, ``wifiHost1`` and ``wpanHost1`` transmit concurrently. The Wifi
transmission is correctly received and successfuly ACKED. Then, ``wifiHost2``
senses the ongoing WPAN transmission, and defers from transmitting. The WPAN
tranmission is correctly received by ``wpanHost2``. When the transmission is over,
``wifiHost1`` sends its next frame; since ACKs are not protected by CCA,
``wpanHost2`` sends the ACK concurrently with the Wifi frame (and also the Wifi ACK).
All frames are correctly received; ``wifiHost1`` waited for the remainder of the
WPAN transmission, during which it could have sent multiple frames.

We examine the performance of the two technologies by looking at the number of
received UDP packets at ``wifiHost2`` and ``wpanHost2``. We look at the
independent performance of the Wifi and WPAN hosts (i.e. when there is just
one of the host pairs communicating), and see how their performance changes
when both of them communicate concurrently.

Here are the performance results:

.. figure:: media/wifiperformance.png
   :width: 70%
   :align: center

.. figure:: media/wpanperformance.png
   :width: 70%
   :align: center

In this particular scenario, the performance of Wifi is decreased
by about 5 percent compared to the base performance. The performance of
WPAN didn't decrease, because it had packets to send infrequently; it could
send them all despite the Wifi traffic. Note that the fractional number of
packets is due to the averaging of the repetitions.

In this scenario, the transmissions of both technologies could be received
correctly when interfering with each other. The decrease in performance comes
from the fact that hosts defer from transmitting when they detect ongoing
transmissions.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`CoexistenceShowcase.ned <../CoexistenceShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet-showcases/issues/49>`__ page in the GitHub issue tracker for commenting on this showcase.
