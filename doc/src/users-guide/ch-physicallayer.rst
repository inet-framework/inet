.. _ug:cha:physicallayer:

The Physical Layer
==================

.. _ug:sec:phy:overview:

Overview
--------

Wireless network interfaces contain a radio model component, which is
responsible for modeling the physical layer (PHY). [1]_ The radio model
describes the physical device that is capable of transmitting and
receiving signals on the medium.

Conceptually, a radio model relies on several sub-models:

-  antenna model

-  transmitter model

-  receiver model

-  error model (as part of the receiver model)

-  energy consumption model

The antenna model is shared between the transmitter model and the
receiver model. The separation of the transmitter model and the receiver
model allows asymmetric configurations. The energy consumer model is
optional, and it is only used when the simulation of energy consumption
is necessary.

.. _ug:sec:phy:generic-radio:

Generic Radio
-------------

In INET, radio models implement the :ned:`IRadio` module interface. A
generic, often used implementation of :ned:`IRadio` is the :ned:`Radio`
NED type. :ned:`Radio` is an active compound module, that is, it has an
associated C++ class that encapsulates the computations.

:ned:`Radio` contains its antenna, transmitter, receiver and energy
consumer models as submodules with parametric types:

.. code-block:: ned

   antenna: <default("IsotropicAntenna")> like IAntenna;
   transmitter: <> like ITransmitter;
   receiver: <> like IReceiver;
   energyConsumer: <default("")> like IEnergyConsumer
       if typename != "";

The following sections describe the parts of the radio model.

.. _ug:sec:phy:components-of-a-radio:

Components of a Radio
---------------------

.. _ug:sec:phy:antenna-models:

Antenna Models
~~~~~~~~~~~~~~

The antenna model describes the effects of the physical device which
converts electric signals into radio waves, and vice versa. This model
captures the antenna characteristics that heavily affect the quality of
the communication channel. For example, various antenna shapes, antenna
size and geometry, antenna arrays, and antenna orientation causes
different directional or frequency selectivity.

The antenna model provides a position and an orientation using a
mobility model that defaults to the mobility of the node. The main
purpose of this model is to compute the antenna gain based on the
specific antenna characteristics and the direction of the signal. The
signal direction is computed by the medium from the position and the
orientation of the transmitter and the receiver. The following list
provides some examples:

-  :ned:`IsotropicAntenna`: antenna gain is exactly 1 in any direction

-  :ned:`ConstantGainAntenna`: antenna gain is a constant determined by
   a parameter

-  :ned:`DipoleAntenna`: antenna gain depends on the direction according
   to the dipole antenna characteristics

-  :ned:`InterpolatingAntenna`: antenna gain is computed by linear
   interpolation according to a table indexed by the direction angles

.. _ug:sec:phy:transmitter-models:

Transmitter Models
~~~~~~~~~~~~~~~~~~

The transmitter model describes the physical process which converts
packets into electric signals. In other words, this model converts an L2
frame into a signal that is transmitted on the medium. The conversion
process and the representation of the signal depends on the level of
detail and the physical characteristics of the implemented protocol.

There are two main levels of detail (or modeling depths):

-  In the *flat model*, the transmitter model skips the symbol domain
   and the sample domain representations, and it directly creates the
   analog domain representation. The bit domain representation is
   reduced to the bit length of the packet, and the actual bits are
   ignored.

-  In the *layered model*, the conversion process involves various
   processing steps such as packet serialization, forward error
   correction encoding, scrambling, interleaving, and modulation. This
   transmitter model requires significantly more computation, but it
   produces accurate bit domain, symbol domain, and sample domain
   representations.

Some of the transmitter types available in INET:

-  :ned:`UnitDiskTransmitter`

-  :ned:`ApskScalarTransmitter`

-  :ned:`ApskDimensionalTransmitter`

-  :ned:`ApskLayeredTransmitter`

-  :ned:`Ieee80211ScalarTransmitter`

-  :ned:`Ieee80211DimensionalTransmitter`

.. _ug:sec:phy:receiver-models:

Receiver Models
~~~~~~~~~~~~~~~

The receiver model describes the physical process which converts
electric signals into packets. In other words, this model converts a
reception, along with an interference computed by the medium model, into
a MAC packet and a reception indication.

For a packet to be received successfully, reception must be *possible*
(based on reception power, bandwidth, modulation scheme and other
characteristics), it must be *attempted* (i.e. the receiver must
synchronize itself on the preamble and start receiving), and it must be
*successful* (as determined by the error model and the simulated part of
the signal decoding).

In the *flat model*, the receiver model skips the sample domain, the
symbol domain, and the bit domain representations, and it directly
creates the packet domain representation by copying the packet from the
transmission. It uses the error model to decide whether the reception is
successful.

In the *layered model*, the conversion process involves various
processing steps such as demodulation, descrambling, deinterleaving,
forward error correction decoding, and deserialization. This reception
model requires much more computation than the flat model, but it
produces accurate sample domain, symbol domain, and bit domain
representations.

Some of the receiver types available in INET:

-  :ned:`UnitDiskReceiver`

-  :ned:`ApskScalarReceiver`

-  :ned:`ApskDimensionalReceiver`

-  :ned:`ApskLayeredReceiver`

-  :ned:`Ieee80211ScalarReceiver`

-  :ned:`Ieee80211DimensionalReceiver`

.. _ug:sec:phy:error-models:

Error Models
~~~~~~~~~~~~

Determining reception errors is a crucial part of the reception process.
There are often several different statistical error models in the
literature even for a particular physical layer. In order to support
this diversity, the error model is a separate replaceable component of
the receiver.

The error model describes how the signal to noise ratio affects the
amount of errors at the receiver. The main purpose of this model is to
determine whether the received packet has errors or not. It also
computes various physical layer indications for higher layers such as
packet error rate, bit error rate, and symbol error rate. For the
layered reception model it needs to compute the erroneous bits, symbols,
or samples depending on the lowest simulated physical domain where the
real decoding starts. The error model is optional (if omitted, all
receptions are considered successful.)

The following list provides some examples:

-  :ned:`StochasticErrorModel`: simplistic error model with constant
   symbol/bit/packet error rates as parameters; suitable for testing.

-  :ned:`ApskErrorModel`

-  :ned:`Ieee80211NistErrorModel`, :ned:`Ieee80211YansErrorModel`,
   :ned:`Ieee80211BerTableErrorModel`: various error models for IEEE
   802.11 network interfaces.

.. _ug:sec:phy:power-consumption-models:

Power Consumption Models
~~~~~~~~~~~~~~~~~~~~~~~~

A substantial part of the energy consumption of communication devices
comes from transmitting and receiving signals. The energy consumer model
describes how the radio consumes energy depending on its activity. This
model is optional (if omitted, energy consumption is ignored.)

The following list provides some examples:

-  :ned:`StateBasedEpEnergyConsumer`: power consumption is determined by
   the radio state (a combination of radio mode, transmitter state and
   receiver state), and specified in parameters like
   :par:`receiverIdlePowerConsumption` and
   :par:`receiverReceivingDataPowerConsumption`, in watts.

-  :ned:`StateBasedCcEnergyConsumer`: similar to the previous one, but
   consumption is given in ampères.

.. _ug:sec:phy:layered-radio-models:

Layered Radio Models
--------------------

In layered radio models, the transmitter and receiver models are split
to several stages to allow more fine-grained modeling.

For transmission, processing steps such as packet serialization, forward
error correction (FEC) encoding, scrambling, interleaving, and
modulation are explicitly modeled. Reception involves the inverse
operations: demodulation, descrambling, deinterleaving, FEC decoding,
and deserialization.

In layered radio models, these processing steps are encapsulated in four
stages, represented as four submodules in both the transmitter and
receiver model:

#. *Encoding and Decoding* describe how the packet domain signal
   representation is converted into the bit domain, and vice versa.

#. *Modulation and Demodulation* describe how the bit domain signal
   representation is converted into the symbol domain, and vice versa.

#. *Pulse Shaping and Pulse Filtering* describe how the symbol domain
   signal representation is converted into the sample domain, and vice
   versa.

#. *Digital Analog and Analog Digital Conversion* describe how the
   sample domain signal representation is converted into the analog
   domain, and vice versa.

In layered radio transmitters and receivers such as
:ned:`ApskLayeredTransmitter` and :ned:`ApskLayeredReceiver`, these
submodules have parametric types to make them replaceable. This provides
immense freedom for experimentation.

.. _ug:sec:phy:notable-radio-models:

Notable Radio Models
--------------------

The :ned:`Radio` module has several specialized versions derived from
it, where certain submodule types and parameters are set to fixed
values. This section describes some of the frequently used ones.

The radio can be replaced in wireless network interfaces by setting the
:par:`typename` parameter of the radio submodule, like in the following ini
file fragment.

.. code-block:: ini

   **.wlan[*].radio.typename = "UnitDiskRadio"

However, be aware that not all MAC protocols can be used with all radio
models, and that some radio models require a matching transmission
medium module.

.. _ug:sec:phy:unitdiskradio:

UnitDiskRadio
~~~~~~~~~~~~~

:ned:`UnitDiskRadio` provides a very simple but fast and predictable
physical layer model. It is the implementation (with some extensions) of
the *Unit Disk Graph* model, which is widely used for the study of
wireless ad-hoc networks. :ned:`UnitDiskRadio` is applicable if network
nodes need to have a finite communication range, but physical effects of
signal propagation are to be ignored.

:ned:`UnitDiskRadio` allows three radii to be given as parameters,
instead of the usual one: communication range, interference range, and
detection range. One can also turn off interference modeling (meaning
that signals colliding at a receiver will all be received correctly),
which is sometimes a useful abstraction.

:ned:`UnitDiskRadio` needs to be used together with a special physical
medium model, :ned:`UnitDiskRadioMedium`.

The following ini file fragment shows an example configuration.

.. code-block:: ini

   *.radioMedium.typename = "UnitDiskRadioMedium"
   *.host[*].wlan[*].radio.typename = "UnitDiskRadio"
   *.host[*].wlan[*].radio.transmitter.bitrate = 2Mbps
   *.host[*].wlan[*].radio.transmitter.preambleDuration = 0s
   *.host[*].wlan[*].radio.transmitter.headerLength = 96b
   *.host[*].wlan[*].radio.transmitter.communicationRange = 100m
   *.host[*].wlan[*].radio.transmitter.interferenceRange = 0m
   *.host[*].wlan[*].radio.transmitter.detectionRange = 0m
   *.host[*].wlan[*].radio.receiver.ignoreInterference = true

As a side note, if modeling full connectivity and ignoring interference
is required, then :ned:`ShortcutInterface` provides an even simpler and
faster alternative.

.. _ug:sec:phy:apsk-radio:

APSK Radio
~~~~~~~~~~

APSK radio models provide a hypothetical radio that simulates one of the
well-known ASP, PSK and QAM modulations. (APSK stands for Amplitude and
Phase-Shift Keying.)

APSK radio has scalar/dimensional, and flat/layered variants. The flat
variants, :ned:`ApskScalarRadio` and :ned:`ApskDimensionalRadio` model
frame transmissons in the selected modulation scheme but without
utilizing other techniques such as forward error correction (FEC),
interleaving, spreading, etc. These radios require matching medium
models, :ned:`ApskScalarRadioMedium` and
:ned:`ApskDimensionalRadioMedium`.

The layered versions, :ned:`ApskLayeredScalarRadio` and
:ned:`ApskLayeredDimensionalRadio` can not only model the processing
steps missing from their simpler counterparts, they also feature
configurable level of detail: the transmitter and receiver modules have
:par:`levelOfDetail` parameters that control which domains are actually
simulated. These radio models must be used in conjuction with
:ned:`ApskLayeredScalarRadioMedium` and
:ned:`ApskLayeredDimensionalRadioMedium`, respectively.

.. [1]
   Wired network interfaces could similarly contain an explicit PHY
   model. The reason they do not is that wired links normally have very
   low error rates and simple observable behavior, and there is usually
   not much to be gained from modeling the physical layer in detail.
