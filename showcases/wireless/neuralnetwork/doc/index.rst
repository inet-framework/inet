:orphan:

Neural Network Error Model
==========================

- the current error model is not very accurate -> especially in corner cases, which are not rare
- it uses just the SNIR to calculate a PER probability...either the min or the mean
- with min, a short spike could ruin a reception
- with mean, a longer (more overlapping in either frequency or time) could not
- the symbol level bit correct (is that the expression?) can simulate it accurately, but it's slow
- the idea is to use a neural network, train it with symbol level training data of receptions
- and use it in the simulation to calculate PER faster than the symbol level simulation
- comparable to the current analytical error model
- but just or almost as accurate than the symbol level

this is the motivation

how does it actually work

- create training dataset
- train network
- use it in the simulation

- check against the analytical and the baseline sybol level (accuracy and performance)

what can be done with this ?

- can create neural network based error model for any model that has symbol level simulation
- the neural network structure is up for optimization

Goals
-----

.. Analytical error models are accurate to some extent, however, they are not well suited to corner cases

Error models are implemented as submodules of receiver modules, and compute the packet error rate, bit error rate or symbol error rate of receptions.

The default setting in radios is packet level simulation, with the analytical error models, such as the NIST error model in 802.11 or the APSK error model with APSKRadio.

The default analytical error models in INET are generally accurate, except for some corner cases, which can be abundant in simulations. Symbol-level simulations are very accurate even in corner cases where the analyitcal ones are not, but very computationally intensive. **TODO** confusing...error models or simulations?

The default analytical error models in INET are generally accurate, except for some corner cases, which can be abundant in simulations. Symbol-level simulations are very accurate even in corner cases where the analyitcal ones are not, but very computationally intensive. **TODO** confusing...error models or simulations?

.. Neural-network-based error models aim to/can potentially achieve the accuracy of symbol level simulations, and the speed of analytical models.

.. A neural network can be trained on reception data from symbol level simulations, and used as accurate error models in packet level simulations

A neural network can be trained on reception characteristics/data/properties/measurements/features of symbol-level simulations, and used as an error model in packet-level simulations.
The resulting error model has the potential to be more accurate than packet-level analytical error models, but with comparable performance.

This showcase demonstrates the workflow of creating such a neural network based error model for 802.11, and compares it..

Motivation
----------

.. The packet-level analytical error model (:ned:`Ieee80211NistErrorModel`) is used in many examples, showcases and tutorials in INET, it's a kind of informal default.

.. The default error models in scalar (all?) receivers are analytical.

.. Error models calculate whether the received frame has errors. It indicates this to the higher layers.


**what are error models?**

The error model is a submodule of the receiver, and calculates the amount of errors in a received frame. It calculates packet, bit or symbol error rate for the MAC layer. In layered mode, it indicates the erroneous bits or symbols as well.

  - meghatároz az error rateek minden szinten ha lehet
  - eldönti h a csomag hibás e vagy sem
  - hogy hol a hiba
  - el is ronthatja a csomagot bit szinten
  - legegyszerubb kiszamol error rate es eldonti h hibas e

  error model egy ugras hogy kihagysz a folyamatban egy csomo lepes -> a fizikai jellemzok dekodolasanak a processet atugorja
  es reprezentalja -> analog domain -> packet domain (a legegyszerubb)

  TODO kell egy rész a limitationokről

  -> most még nem drop in replacement a default error modelre

so

- the main purpose of error models is to decide if there was an error in receiving a frame
- during the reception, the error model determines whether the reception had errors
- depending the type of the error model, and the detail level of the simulation,
it might indicate that the packet has errors, or that certain bits are erroneous...
- it calculates error rates for the higher layers such as packet error rate bit error rate or symbol error rate,
depending on the detail level of the simulation

Error models are basically a leap in the decoding process. We start with a signal at the analog level. Then there is symbol, bit and packet level (also sample level but its not implemented). We either simulate each level in the process -> receiving symbols, forward error correction, descrambling, deinterleaving, or we jump a few levels with the error model. We replace the model of the whole or some of the decoding process with a more simple model, the error model.

**about the default error models**

TODO

**about analytical error models**

The packet-level analytical error models calculate a packet-error rate based on the reception SNIR. They either use the minimum or the mean of the SNIR during reception. Both methods can lead to unrealistic reception probabilities in corner cases. For example, when using the minimum SNIR, a short spike in an interfering signal can ruin a reception; with mean SNIR, an interfering signal overlapping with a transmission to a large extent (in frequency or time) can still result in a correctly receivable transmission.

**about symbol level simulation**

Using symbol level simulation can accurately model these corner cases, as per-symbol-SNIR can be used, and an accurate symbol-error-rate can be calculated. However, the symbol-level simulations are very computationally intensive.

.. **TODO** what are error models ?

**the idea**

detailed description

A neural network error model can potentially be more accurate than the analytical error models, but with comparable performance. The reception probability of a symbol can be calculated analytically in symbol-level bit-correct simulations. We create a big training dataset from accurate, symbol-level simulations, with multiple noise sources, and variable noise spectrum, duration, and power. We record these parameters, along with the reception center frequency, bandwidth, modulation, and SNIR for every symbol.
**TODO** contains every property of the channel; it contains the success/failure of the frame (not probability). When we have lots of data it becomes probability. -> the training dataset -> packet error rate in the different conditions (cos we wanna replace the default analytical packet-level error model with this)
Then we train the neural network on this training dataset.

We use this neural network as the error model, which gives estimation of packet error rate for similar parameters.
-> relatively good estimation

.. We create a large training dataset by running symbol-level simulations. The training data needs to cover a wide range of parameters...

.. We create a large training dataset, by running lots of simulations. The simulations use layered dimensional radios, and symbol level of detail. The complete decoding process is simulated, i.e. scrambling, interleaving and forward forward error correction.
  The Ieee80211OfdmErrorModel calculates a symbol error rate from the per-symbol SNIR, and corrupts the symbol (replaces it with another one) if needed.

.. Then the symbol goes through the decoding process. The MAC indicates if it's incorrectly received.

.. so

  - we train the neural network on the SNIR of the various symbols,

  - we run a symbol-level accurate simulation
  - there is a closed figure ? closed formula for the symbol error rate depending on the per-symbol SNIR
  - run a lot of simulations, with varying conditions, such as background noise power, number of interfering signals, power of interfering signals, etc.
  - to goal is for the dataset to contain a broad range of situations/variability of the reception environment
  - there is a log file -> the training dataset

  - the training dataset generation is symbol level, with scrambling, interleaving and FEC
  - layered dimensional transmitter and receiver
  - the error model calculates the symbol error rate for each symbol
  - the decoding process is simulated
  - the symbol error rate has random, but the decoding process doesn't
  - after the decoding process, we have a packet reception success/failure
  - do this a 100 times, and we get a packet error rate
  - for this iteration of the variables
  - the neural network is trained on the per symbol SNIR and the packet error rate
  - for a given bitrate, modulation, center frequency and bandwidth
  - for others we create a different neural network model
  - could include the modulation in the training data

  - iteration variables to create a broad range of reception properties/circumstances/situations
  - such as number of noise sources, their power, their duration, etc
  - repetitions of a set of iteration variables to get packet error rate
  - train neural network on packet error rate and per symbol SNIR

.. We create a large training dataset. We run many simulations

.. We create a large training dataset by running many simulation. The simulations cover a broad range of reception scenarios. The simulations need to be as accurate as possible; they are symbol level, and use layered dimensional radios

.. so

  - we create a large training dataset by running many simulations
  - the simulations cover a broad range of reception scenarios/circumstances, with iteration variables such as number, duration and power of interfering signals
  - what actually happens is that we simulate lots of receptions, with noise and interference, in great detail
  - they are as accurate as possible; they use symbol level of detail, dimensional layered radios
  - i.e. each symbol of each subcarrier is simulated
  - the complete coding and decoding process is simulated
  - i.e. scrambling, interleaving, and forward error correction in the transmitter (and the inverse process in the receiver)
  - during the reception process, the TODO error model calculates a symbol error rate from the modulation, frequency, bandwidth and per-symbol SNIR
  - based on this SER, it corrups the received symbol (replaces with another one)
  - the symbols then undergo the decoding process, the MAC might detect errors and drop the frame
  - so we have many receptions, each either successful or failed
  - from this we calculate a packet error rate, corresponding to the set of parameter/variable values with which we ran the simulations
  - the neural networks input is the SNIR at the time intervals in the reception process corresponding to the symbols
  (even tho we dont use symbol level of detail when we use the neural network error model) TODO
  - the neural network's input is the per-symbol SNIR and the packet error rate
  - and its actually done for a certain

structure:

  - (we create a) large traning dataset (why is it important? to "prepare" the neural network for generally any channel conditions)
  - (by using) detailed simulation (why is it important? so that the result is as accurate as possible)
  - how we create that?
  	- so iteration variables -> parameter space
  	- in each iteration, multiple simulations -> PER
  	- per symbol SNIR + PER -> neural network input
  	- when using: per symbol SNIR (or where that would be during the reception process) -> estimate PER

.. We create a large training dataset by running thousands of simulations. The simulations cover a broad range of channel conditions/reception conditions, to prepare the neural network for generally any channel condition. The simulations are as detailed as possible, to make the resulting neural network error model as accurate as possible. To do that:

  - we use symbol level of detail, and layered dimensional radios, i.e. the transmission and reception of each symbol of each subcarrier is simulated. The complete coding and decoding process is simulated, i.e. scrambling, interleaving and forward error correction (and the inverse process in the receiver). We use the IeeeOfdmErrorModel, which calculates a symbol error rate from the modulation, spectrum, and per-symbol SNIR at reception. Based on the calculated SER, it corrupts the received symbol when necessary, i.e. it replaces it with another symbol. Then the symbol undergoes the decoding process, and the packet is passed on to the MAC. The MAC decides if the packet had errors, and drops it when necessary.
  - we iterate over a wide range of signal and channel parameters, such as the number, duration, power and spectrum of interfering signals, power and spectrum of background noise, etc./The iteration variables are a wide range of signal and channel parameters
  - we simulate reception in a given iteration many times. A packet error rate is calculated from the many reception outcomes.
  - The input of the neural network is the per-symbol SNIR and the packet error rate
  - When using the neural network as the error model, the network estimates a packet error rate from the per-symbol SNIR (or the SNIR in the time intervals corresponding to symbols). This works with dimensional and scalar analog models as well, tho the dimensional is more accurate.
  - The result is training dataset with channel conditions represented by per-symbol SNIR values and corresponding PER.

---------------------------------------------

We create a large training dataset by running thousands of simulations. The simulations cover a broad range of channel conditions/reception conditions, to prepare the neural network for generally any channel condition. The simulations are as detailed as possible, to make the resulting neural network error model as accurate as possible.

To do that, we use symbol level of detail, and layered dimensional radios, i.e. the transmission and reception of each symbol of each subcarrier is simulated. Also, the complete coding and decoding process is simulated, i.e. scrambling, interleaving and forward error correction (and the inverse process in the receiver). We use the :ned:`Ieee80211OfdmErrorModel`, which calculates a symbol error rate from the modulation, spectrum, and per-symbol SNIR at reception. Based on the calculated SER, it corrupts the received symbol when necessary, i.e. it replaces it with another symbol. Then the symbol undergoes the decoding process, and the packet is passed on to the MAC. The MAC decides if the packet had errors, and drops it when necessary.

We iterate over a wide range of signal and channel parameters, such as the number, duration, power and spectrum of interfering signals, power and spectrum of background noise, etc./The iteration variables are a wide range of signal and channel parameters
We simulate reception in a given iteration many times. A packet error rate is calculated from the many reception outcomes.
The input of the neural network is the per-symbol SNIR and the packet error rate.

When using the neural network as the error model, the network estimates a packet error rate from the per-symbol SNIR (or the SNIR in the time intervals corresponding to symbols). This works with dimensional and scalar analog models as well, tho the dimensional is more accurate.
The result is a training dataset with channel conditions represented by per-symbol SNIR values and corresponding PER.

**TODO** random; evaluate acronyms;

---------------------------------------------

  keywords

  - thousands of channel parameters/ thousands of signal parameters
  - iterate over a wide range on thousands of channel/signal parameters
  - reception outcome

The Model
---------

The Process
~~~~~~~~~~~

- creating training dataset
- training the network
- using it as an error model
