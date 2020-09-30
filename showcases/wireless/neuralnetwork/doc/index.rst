:orphan:

Neural-Network-based Wireless Error Model
=========================================

.. - the current error model is not very accurate -> especially in corner cases, which are not rare
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

..  The default setting in radios is packet level simulation, with the analytical error models, such as the NIST error model in 802.11 or the APSK error model with APSKRadio.

.. The default analytical error models in INET are generally accurate, except for some corner cases, which can be abundant in simulations. Symbol-level simulations are very accurate even in corner cases where the analyitcal ones are not, but very computationally intensive. **TODO** confusing...error models or simulations?

.. Simulating wireless receptions with flat receiver models and analytical error models is fast and generally accurate, except for some corner cases, which can be abundant in simulations. Detailed, symbol-level simulations are very accurate even in corner cases where the flat/analyitcal models are not, but they are very computationally intensive.

.. *the goal: show we can do this; how its done; how users can do this with others;
   how users can use this; how good it is; how they can improve it;*

Simulating wireless receptions with analytical error models is fast and generally accurate, except for some corner cases, which can be abundant in some simulations. Detailed, symbol-level simulation is very accurate even in these corner cases, but they it is very computationally intensive.

.. Simulating the reception process in detail (on the symbol level) is very accurate even in corner cases where the analyitcal ones are not, but very computationally intensive.

  For more accuracy, detailed, symbol-level simulations can be used, which are accurate even in corner cases where the analytical ones are not, but very computationally intensive.

.. ------------------------------
  so very concisely:

  - the goals...and some intro
  - the commonly used analytical error models are garbage
  - the detailed ones are computationally intensive
  - we can make a better error model with deep learning
  ------------------------------

.. Neural-network-based error models aim to/can potentially achieve the accuracy of symbol level simulations, and the speed of analytical models.

.. A neural network can be trained on reception data from symbol level simulations, and used as accurate error models in packet level simulations

As a best-of-both-worlds approach, a neural network can be trained on reception characteristics of detailed symbol-level simulations, and used as an error model in packet-level simulations.
The resulting error model has the potential to be more accurate than packet-level analytical error models, but with comparable performance.

This showcase demonstrates the workflow of creating such a neural network based error model for 802.11 and :ned:`ApskRadio`, and compares their performance and accuracy to the baseline symbol-level simulation and those using analytical error models.

Overview
--------

.. *and concepts*

.. **some concept:**

.. **the reception process**

.. **V1** The reception process is converting a physical signal to a packet that can be sent to the MAC. The conversion process can be configured to be very detailed, simulating the reception of individual symbols and the decoding process (demodulation, deinterleaving, descrambling, forward error correction decoding), or more simple by jumping from the physical signal level to the packet level.
   In this case, the detailed reception and decoding process is replaced with a simple model, the analytical error model.

During the reception process, an analog signal is converted to a packet that can be sent up to the MAC. The conversion process goes through the analog, sample, symbol, bit, and packet domains. This conversion process can be configured to be very detailed, simulating the reception of individual symbols and the decoding process (i.e. demodulation, deinterleaving, descrambling, forward error correction decoding), or more simple by jumping from the analog signal domain to the packet domain.
In this case, the detailed reception and decoding process is replaced with a simple model, the error model.

.. **TODO** the other levels

.. The packet-level analytical error model (:ned:`Ieee80211NistErrorModel`) is used in many examples, showcases and tutorials in INET, it's a kind of informal default.

.. The default error models in scalar (all?) receivers are analytical.

.. Error models calculate whether the received frame has errors. It indicates this to the higher layers.


.. **what are error models?**

.. The error model is a submodule of the receiver, and calculates the amount of errors in a received frame. It calculates packet, bit or symbol error rate for the MAC layer. In layered mode, it indicates the erroneous bits or symbols as well.

  **TODO** in the most detailed configuration, the error model calculates symbol error rate for each symbol from per-symbol SNIR and decides which symbols to corrupt (replace with another) and how.

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

  Error models are basically a leap in the decoding process. We start with a signal at the analog level. Then there is symbol, bit and packet level (also sample level but its not implemented). We either simulate each level in the process -> receiving symbols, forward error correction, descrambling, deinterleaving, or *we jump a few levels with the error model. We replace the model of the whole or some of the decoding process with a more simple model, the error model.*

**V1** Error models describe how the signal-to-noise-plus-interference ratio (SNIR) affects the amount of errors in a received frame. They can calculate the packet error rate, bit error rate or symbol error rate of receptions, depending on the simulated level of detail. They are implemented as replaceable submodules of receivers.

**V2** Error models decide if a frame was correctly received or not. They can calculate error rates (depending on the configured level of detail, symbol-, bit-
and packet error rate) for higher layers. They can corrupt symbols (replace them with another one) or bits (invert them), or indicate that the frame is
incorrectly received. They are implemented as replaceable submodules of receivers.

.. **about the default error models**

.. TODO

The most commonly used packet level radio models in INET examples and showcases feature analytical error models, such as the NIST error model in 802.11 or the APSK error model with APSKRadio. The packet-level radio model is an informal default because it's fast and accurate enough most of the time.

.. for performance reasons

.. **TODO** flat implies packet level

.. **TODO** somewhere -> in the detailed case there is an error model for calculating the symbol error rate from the per-symbol SNIR

.. **TODO** analytical error models and the closed formula based on statistics and empirical evidence

.. **about analytical error models**

.. The packet-level analytical error models calculate a packet-error rate based on the reception SNIR.

Motivation
----------

The packet-level analytical error models use a scalar SNIR value to calculate packet error rate with closed formulas for the different wireless technologies. The error models either use the minimum or the mean of the SNIR during reception. Both methods can lead to unrealistic reception probabilities in corner cases. For example, when using the minimum SNIR, a short spike in an interfering signal can ruin a reception unecessarily; with mean SNIR, an interfering signal overlapping with a transmission to a large extent (in frequency or time) can still result in a correctly received transmission unecessarily.

.. TODO scalar SNIR

.. **about symbol level simulation**

Symbol level simulation can model these corner cases more accurately, as per-symbol-SNIR can be used, and a symbol-error-rate can be calculated with a modulation-specific closed formula. This method is less sensitive to the corner cases mentioned above. However, the symbol-level simulation is very computationally intensive because it needs to do the complete decoding process.

.. **TODO** layered, the whole reception process is modeled

.. **TODO** what are error models ?

.. **the idea**

.. detailed description

.. *A neural network error model can potentially be more accurate than the analytical error models, but with comparable performance. The reception probability of a symbol can be calculated analytically in symbol-level bit-correct simulations. We create a big training dataset from accurate, symbol-level simulations, with multiple noise sources, and variable noise spectrum, duration, and power. We record these parameters, along with the reception center frequency, bandwidth, modulation, and SNIR for every symbol.
.. **TODO** contains every property of the channel; it contains the success/failure of the frame (not probability). When we have lots of data it becomes probability. -> the training dataset -> packet error rate in the different conditions (cos we wanna replace the default analytical packet-level error model with this)
.. Then we train the neural network on this training dataset.*

.. We use this neural network as the error model, which gives estimation of packet error rate for similar parameters.
.. -> relatively good estimation

.. so

  - some introduction to the next section/conclusion to this one
  - create large traning dataset
  - train network
  - use it in simulations as error model

.. A neural network trained on the reception data of a lot of detailed symbol-level simulations can be used as an error model, and it is potentially more accurate than analytical error models, with comparable performance.

.. A neural network error model can potentially be more accurate than the analytical error models, but with comparable performance.

.. TODO neural network jo comprimise a ketto kozott

The neural network error model is a good compromise between these two methods.
The idea is to create a large training dataset containing the channel parameters and outcomes of many receptions using symbol-level simulation, and use it to train the neural network. We use this neural network as the error model, which gives an estimation of the packet error rate for similar channel conditions.

.. -> relatively good estimation

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

.. structure:

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

The Model
---------

We create a large training dataset by running thousands of simulations. The simulations cover a broad range of channel conditions/reception conditions, to prepare the neural network for generally any channel condition. The simulations are as detailed as possible, to make the resulting neural network error model as accurate as possible.

To do that, we use symbol level of detail dimensional analog signal representation, i.e. the transmission and reception of each symbol of each subcarrier is simulated. Also, the complete coding and decoding process is simulated, i.e. scrambling, interleaving and forward error correction (and the inverse process in the receiver). We use the :ned:`Ieee80211OfdmErrorModel`, which calculates a symbol error rate from the modulation, spectrum, and per-symbol SNIR at reception. Based on the calculated SER, it corrupts the received symbol when necessary, i.e. it replaces it with another symbol. Then the symbols of the signal undergo the decoding process, and it is indicated whether the signal was correctly received/the frame was corrupt **TODO**.
(Actually, reception process only makes use of random numbers when deciding how to corrupt symbols based on the SER. After that, the decoding process doesn't use random numbers, and the output is a reception outcome - successful or failed).**TODO**

.. The MAC decides if the packet had errors, and drops it when necessary.

We iterate over a wide range of signal and channel parameters, such as the number, duration, power and spectrum of interfering signals, power and spectrum of background noise, etc.
We simulate reception in a given iteration many times. A packet error rate is calculated from the many reception outcomes.
The input of the neural network is the per-symbol SNIR and the estimated packet error rate.

When using the neural network as the error model, the network estimates a packet error rate from the per-symbol SNIR vector (or the SNIR in the time intervals corresponding to symbols). This works with dimensional and scalar analog models as well, though the dimensional is more accurate.

.. The result is a training dataset with channel conditions represented by per-symbol SNIR values and corresponding PER.

**TODO** random; evaluate acronyms;

..  keywords

  - thousands of channel parameters/ thousands of signal parameters
  - iterate over a wide range on thousands of channel/signal parameters
  - reception outcome

.. TODO

  2 resz

  - tanitas
  - using

- use the trainingDatasetGenerator module
- use the script
- the result files (log files)

We use the :ned:`NeuralNetworkErrorModelTrainingDatasetGenerator` module for this purpose. The dataset generation is specified in the generate-training-dataset.ini file, with the network defined in GenerateNeuralNetworkErrorModelTrainingDataset.ned:

.. literalinclude:: ../GenerateNeuralNetworkErrorModelTrainingDataset.ned
   :start-at: GenerateNeuralNetworkErrorModelTrainingDataset
   :end-before: connections
   :language: ned

The :ned:`NeuralNetworkErrorModelTrainingDatasetGenerator` uses the radio module in the network to simulate receptions. It has parameters for packet length, the number of interfering signals, the mean and stddev of the interfering signal and background noise power, the repeat count.

Here are the two configurations in omnetpp.ini for generating the training data for :ned:`ApskRadio` and 802.11:

.. literalinclude:: ../generate-training-dataset.ini
   :start-at: Config ApskRadio
   :language: ini

**TODO** not sure its needed -> describe and some excerpt ?

.. - apsk config
   - 802.11 config

The results are log files which contain the iteration variables (TODO), the per-symbol SNIR and the packet error rate. Here is the header and the first line of a log file (each line describes a reception):

.. :download:`log <../log>`

.. literalinclude:: ../log
   :lines: 1,2
   :append: ...

.. - Our approach is to create a neural network model for each modulation, bit rate, channel and bandwidth used in IEEE 802.11g, so the models are less complex, smaller, easier to train and run
   - One could create just one model which can be used for all modulations, bit rates, channels and bandwidth
   - For this showcase, we just created the 24Mbps QAM-16 20MHz BW 2.412GHz center frequency
   - Now, it works with fixed packet sizes (1000B)

Our approach is to create a neural network model for each modulation, bit rate, Wifi channel and bandwidth used in IEEE 802.11g. (One could also create just one model which can be used for all modulations, bit rates, channels and bandwidth). We chose the multiple models approach because these models are less complex, smaller, and easier to train and run, compared to using just one model. For this showcase, we only created the model for 802.11g 24Mbps QAM-16 20MHz bandwidth 2.412GHz center frequency version,
and the 36Mbps BPSK 20MHz bandwidth 2.4GHz center frequency version for :ned:`ApskRadio`.

.. **TODO** - Now, it works with fixed packet sizes (1000B)

.. note:: Our model currently only works with fixed packet sizes of 1000B

Training the neural network
---------------------------

how to train

- using keras
- the neural network model (its up for optimization)
- the script which builds the neural network model and trains it
- the result is model and h5 files

We use Keras to build and train the neural network. The network used in this showcase has the following structure:

.. figure:: graph.svg
   :width: 90%
   :align: center

.. figure:: legend.svg
   :width: 90%
   :align: center

It uses 64 + 32 + 1 neurons in the dense layers TODO.

Here it is in Keras' model summary function:

.. code-block:: text

  Model: "sequential"
  _________________________________________________________________
  Layer (type)                 Output Shape              Param #
  =================================================================
  conv1d (Conv1D)              (None, None, 32)          256
  _________________________________________________________________
  average_pooling1d (AveragePo (None, None, 32)          0
  _________________________________________________________________
  conv1d_1 (Conv1D)            (None, None, 8)           776
  _________________________________________________________________
  lstm (LSTM)                  (None, None, 4)           208
  _________________________________________________________________
  global_average_pooling1d (Gl (None, 4)                 0
  _________________________________________________________________
  dense (Dense)                (None, 32)                160
  _________________________________________________________________
  dense_1 (Dense)              (None, 16)                528
  _________________________________________________________________
  dense_2 (Dense)              (None, 1)                 17
  =================================================================
  Total params: 1,945
  Trainable params: 1,945
  Non-trainable params: 0
  _________________________________________________________________

The TODO script creates the keras model, and trains it on the training datasets.

**TODO** repeat = 8 so there are 8 log files

For ApskRadio:

.. code-block:: bash

   ./train-neural-network.py ApskRadio_36Mbps_Bpsk_2.4GHz_20MHz_*.log

For 802.11:

.. code-block:: bash

   ./train-neural-network.py Ieee80211OfdmRadio_24Mbps_Ieee80211Ofdm_Qam16_2.412GHz_20MHz_0.log

Using the neural network
------------------------

.. technical details

  - creating training dataset
  - training the network
  - using it as an error model
