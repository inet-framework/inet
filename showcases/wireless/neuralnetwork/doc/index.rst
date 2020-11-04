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

Simulating wireless receptions with analytical error models is fast and generally accurate, except for some corner cases, which can be abundant in some simulations. Detailed, symbol-level simulation is very accurate even in these corner cases, but it is very computationally intensive.

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

As a best-of-both-worlds approach, a neural network can be trained on the reception characteristics of detailed symbol-level simulations, and used as an error model in packet-level simulations.
The resulting error model has the potential to be more accurate than packet-level analytical error models, but with comparable performance.

This showcase demonstrates the workflow of creating such a neural-network-based error model for 802.11 and compares its performance and accuracy to the baseline symbol-level simulation and those using analytical error models.

Overview
--------

.. *and concepts*

.. **some concept:**

.. **the reception process**

.. **V1** The reception process is converting a physical signal to a packet that can be sent to the MAC. The conversion process can be configured to be very detailed, simulating the reception of individual symbols and the decoding process (demodulation, deinterleaving, descrambling, forward error correction decoding), or more simple by jumping from the physical signal level to the packet level.
   In this case, the detailed reception and decoding process is replaced with a simple model, the analytical error model.

During the reception process, an analog signal is converted to a packet that can be sent up to the MAC. During the conversion process, the received frame goes through the analog, sample, symbol, bit, and packet domains. This conversion process can be configured to be very detailed, simulating the reception of individual symbols and the decoding process (i.e. demodulation, deinterleaving, descrambling, forward error correction decoding), or simpler by jumping from the analog signal domain to the packet domain.
In the latter case, the detailed reception and decoding process is replaced with a simple model, the error model.

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

.. **V1** Error models describe how the signal-to-noise-plus-interference ratio (SNIR) affects the amount of errors in a received frame. They can calculate the packet error rate, bit error rate or symbol error rate of receptions, depending on the simulated level of detail. They are implemented as replaceable submodules of receivers.

Error models decide if a frame was correctly received or not. They can calculate error rates (depending on the configured level of detail, symbol-, bit-
and packet error rate) for higher layers. They can corrupt symbols (replace them with another one) or bits (invert them), or indicate that the frame is
incorrectly received. They are implemented as replaceable submodules of receivers.

.. **about the default error models**

.. TODO

The most commonly used packet level radio models in INET examples and showcases feature analytical error models, such as the NIST error model in 802.11. The packet-level radio model is an informal default because it's fast and accurate enough most of the time.

..  or the APSK error model with :ned:`ApskRadio`

.. for performance reasons

.. **TODO** flat implies packet level

.. **TODO** somewhere -> in the detailed case there is an error model for calculating the symbol error rate from the per-symbol SNIR

.. **TODO** analytical error models and the closed formula based on statistics and empirical evidence

.. **about analytical error models**

.. The packet-level analytical error models calculate a packet-error rate based on the reception SNIR.

Motivation
----------

The packet-level analytical error models use a single scalar Signal-to-noise-plus-interference radio (SNIR) value for the whole frame to calculate packet error rate with closed formulas for the different wireless technologies. These error models either use the minimum or the mean of the SNIR during reception. Both methods can lead to unrealistic reception probabilities in corner cases. For example, when using the minimum SNIR, a short spike in an interfering signal can ruin a reception unnecessarily; with mean SNIR, an interfering signal overlapping with a transmission to a large extent (in frequency or time) can still result in a correctly received transmission unnecessarily.

.. TODO scalar SNIR

.. **about symbol level simulation**

**V1** Symbol level simulation can model these corner cases more accurately, as per-symbol-SNIR can be used, and a symbol-error-rate can be calculated with a modulation-specific closed formula. This method is less sensitive to the corner cases mentioned above. However, the symbol-level simulation is very computationally intensive because it needs to do the complete decoding process.

**V2** Symbol-level simulations use error models which calculate a symbol-error-rate from the per-symbol-SNIR with a modulation-specific closed formula. This method is less sensitive to the corner cases mentioned above. However, the symbol-level simulation is very computationally intensive because it needs to do the complete decoding process.

.. - this is better because the error model is applied to a smaller phenomena, as opposed to the analytical which tries to estimate the error which is a complex thing from just the snir

  - symbol level simulations use error models as well to calculate a symbol error rate from the per-symbol SNIR
  - these error models are analytical in the sense that there is a closed formula for the symbol error rate vs per-symbol SNIR
  - but the after that, the decoding process is simulated, and this yields either a correctly received packet or an incorrectly received (and dropped) one
  - so its technically analytical, actually, its more like not
  - so its not analytical, because the analytical error models only use a closed formula for the whole reception process (not simulating it but just giving an estimate for the PER)

  Symbol-level simulations use error models which calculate a symbol-error-rate from the per-symbol-SNIR with a modulation-specific closed formula. Based on the symbol error rate, some symbols are corrupted (replaced with another symbol) and the symbols undergo the decoding process.

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

The neural network based error model is a good compromise between these two methods.
The idea is to create a large training dataset containing the channel parameters and outcomes of many receptions using symbol-level simulation, and use it to train the neural network. We use this neural network as the error model, which gives an estimation of the packet error rate for similar channel conditions in packet level simulations.

.. .. note:: With this process, a neural network error model can be created for any wireless technology which has a symbol-level simulation model.

.. **TODO** its used instead of the packet level

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

.. The Model
   ---------

Generating Training Data
------------------------

We create a large training dataset by running thousands of simulations. The simulations cover a broad range of channel conditions, to prepare the neural network for generally any channel condition.

.. The simulations are as detailed as possible, to make the resulting neural network based error model as accurate as possible.

.. To do that,

To make the neural-network-based error model as accurate as possible, we use symbol level of detail and dimensional analog signal representation, i.e. the transmission and reception of each symbol of each subcarrier is simulated. Also, the complete coding and decoding process is simulated, i.e. scrambling, interleaving and forward error correction (and the inverse process in the receiver). We use the :ned:`Ieee80211OfdmErrorModel`, which calculates a symbol error rate from the modulation, spectrum, and per-symbol SNIR at reception. Based on the calculated symbol error rate (SER), it corrupts the received symbol when necessary, i.e. it replaces it with another symbol. Then the (potentially altered) symbols of the signal undergo the decoding process. The higher layers can either correct the errors and receive the packet correctly or find it erroneous and drop the packet; this reception outcome is recorded. (The reception process only makes use of random numbers when deciding how to corrupt symbols based on the SER.)

.. **V1** After the decoding process, the outcome of the reception is either successful or failed.)

.. **V2** The decoding process after that is not stochastic.

.. **V3** After the decoding process, the received frame is either verified as correct or found erroneous and dropped.

.. After that, the decoding process doesn't use random numbers, and the output is a reception outcome - successful or failed).**TODO**

.. and it is indicated whether the signal was correctly received/the frame was corrupt **TODO**.
   (Actually, reception process only makes use of random numbers when deciding how to corrupt symbols based on the SER. After that, the decoding process doesn't use random numbers, and the output is a reception outcome - successful or failed).**TODO**

.. The MAC decides if the packet had errors, and drops it when necessary.

We iterate over a wide range of signal and channel parameters, such as the number, duration, power and spectrum of interfering signals, power and spectrum of background noise, etc.
We simulate reception in a given iteration many times. A packet error rate is calculated from the many reception outcomes.
The input of the neural network is the per-symbol SNIR and the estimated packet error rate.

.. **V1** When using the neural network as the error model, the network estimates a packet error rate from the per-symbol SNIR vector (or the SNIR in the time intervals corresponding to symbols). This works with dimensional and scalar analog models as well, though the dimensional is more accurate.

When using the neural network as the error model, the network estimates a packet error rate from the per-symbol SNIR vector with the dimensional analog model, or the SNIR in the time intervals corresponding to symbols with the scalar analog model.

.. The result is a training dataset with channel conditions represented by per-symbol SNIR values and corresponding PER.

.. **TODO** random; evaluate acronyms;

..  keywords

  - thousands of channel parameters/ thousands of signal parameters
  - iterate over a wide range on thousands of channel/signal parameters
  - reception outcome

.. TODO

  2 resz

  - tanitas
  - using

.. - use the trainingDatasetGenerator module
   - use the script
   - the result files (log files)

We use the :ned:`NeuralNetworkErrorModelTrainingDatasetGenerator` module to create the dataset. The dataset generation is specified in the ``generate-training-dataset.ini`` file, with the network defined in :download:`GenerateNeuralNetworkErrorModelTrainingDataset.ned <../GenerateNeuralNetworkErrorModelTrainingDataset.ned>`:

.. literalinclude:: ../GenerateNeuralNetworkErrorModelTrainingDataset.ned
   :start-at: GenerateNeuralNetworkErrorModelTrainingDataset
   :end-before: connections
   :language: ned

The :ned:`NeuralNetworkErrorModelTrainingDatasetGenerator` uses the radio module in the network to simulate receptions. It has parameters for packet length, the number of interfering signals, the mean and stddev of the interfering signals, background noise power, and the repeat count.

.. Here are the two configurations in generate-training-dataset.ini for generating the training data for :ned:`ApskRadio` and 802.11:

.. The simulations that generate the training data are defined in generate-training-dataset.ini.

.. There are two configurations in :download:`generate-training-dataset.ini <../generate-training-dataset.ini>`, ``ApskRadio`` and ``Ieee802.11Radio``. The following are some excerpts from these configurations.

The following are some excerpts from the ``Ieee802.11Radio`` configuration in :download:`generate-training-dataset.ini <../generate-training-dataset.ini>`:

.. The following are some excerpts from generate-training-dataset.ini for the 802.11 configuration.

.. .. literalinclude:: ../generate-training-dataset.ini
   :start-at: Config ApskRadio
   :language: ini

.. Configuring the trainingDatasetGenerator module in the ``Ieee80211Radio`` configuration:

The training dataset generator is configured to create file names which contain the class name, bitrate, modulation, subcarrier modulation, center frequency, bandwidth and repetition number. Each reception is simulated 1000 times, with 64-Byte packets; the distribution of the background noise and interfering signal power is specified:

.. the subcarrier modulation is added to the filename; additionally, interfering signals are added.

.. .. literalinclude:: ../generate-training-dataset.ini
   :start-at: trainingDatasetGenerator.filenameFormat
   :end-at: backgroundNoisePowerStddev
   :language: ini

.. literalinclude:: ../generate-training-dataset.ini
   :start-after: error model of Ieee80211Radio
   :end-at: interferingSignalNoisePowerStddev
   :language: ini

.. Configuring the radio:

.. The radio is configured to use the dimensional analog model and symbol level of detail, in addition to the center frequency, bandwidth, modulation, etc; forward error correction is enabled:

The radio is configured to use the dimensional analog model and symbol level of detail. The :ned:`Ieee80211LayeredOfdmReceiver` module doesn't have a default error model, so it is selected. **TODO** why?

.. .. literalinclude:: ../generate-training-dataset.ini
   :start-at: ApskLayeredDimensionalRadio"
   :end-at: bandwidth
   :language: ini

.. literalinclude:: ../generate-training-dataset.ini
   :start-at: Ieee80211OfdmRadio
   :end-at: bandwidth
   :language: ini

.. literalinclude:: ../generate-training-dataset.ini
   :start-at: Ieee80211LayeredOfdmTransmitter
   :end-at: power
   :language: ini

.. literalinclude:: ../generate-training-dataset.ini
   :start-at: Ieee80211OfdmErrorModel
   :end-at: Ieee80211OfdmErrorModel
   :language: ini

The layered 802.11 model simulates the coding and decoding process by default (forward error correction, scrambling, interleaving).

.. This radio uses the :ned:`ApskLayeredErrorModel` by default.

.. In the ``Ieee80211Radio`` configuration, the training dataset generator is configured similarly to the APSK Radio configuration, but the subcarrier modulation is added to the filename; additionally, interfering signals are added.

  .. literalinclude:: ../generate-training-dataset.ini
     :start-at: 36Mbps
     :end-at: fecEncoder.typename
     :language: ini

  .. literalinclude:: ../generate-training-dataset.ini
     :start-after: error model of Ieee80211Radio
     :end-at: interferingSignalNoisePowerStddev
     :language: ini

  The radio is configured to use the dimensional analog model and symbol level of detail. The Ieee80211LayeredOfdmReceiver module doesn't have a default error model, so it is selected. **TODO** why?

  .. literalinclude:: ../generate-training-dataset.ini
     :start-at: Ieee80211OfdmRadio
     :end-at: bandwidth
     :language: ini

  .. literalinclude:: ../generate-training-dataset.ini
     :start-at: Ieee80211OfdmErrorModel
     :end-at: Ieee80211OfdmErrorModel
     :language: ini

.. **TODO** not sure its needed -> describe and some excerpt ?

.. - apsk config
   - 802.11 config

The :ned:`NeuralNetworkErrorModelTrainingDatasetGenerator` module creates log files which contain the iteration variables, the per-symbol SNIR and the packet error rate. Here is the header and the first line of a log file (each line describes a reception):

.. **TODO** this is the training dataset

.. :download:`log <../log>`

.. literalinclude:: ../log
   :lines: 1,2
   :append: ...

.. - Our approach is to create a neural network model for each modulation, bit rate, channel and bandwidth used in IEEE 802.11g, so the models are less complex, smaller, easier to train and run
   - One could create just one model which can be used for all modulations, bit rates, channels and bandwidth
   - For this showcase, we just created the 24Mbps QAM-16 20MHz BW 2.412GHz center frequency
   - Now, it works with fixed packet sizes (1000B)

Our approach is to create a neural network model for each modulation, bit rate, Wifi channel and bandwidth used in IEEE 802.11g. (One could also create just one, more generic model). We chose the multiple models approach because these models are less complex, smaller, and easier to train and run, compared to using just one model (the error model module will automatically choose the neural network needed for the given modulation when used in the simulation, based on the filename). For this showcase, we only created the model for 802.11g 24Mbps QAM-16 20MHz bandwidth 2.412GHz center frequency version.

.. which can be used for all modulations, bit rates, channels and bandwidth

.. **TODO** the error model will automatically choose the neural network needed when used in the simulation

.. **TODO** - Now, it works with fixed packet sizes (1000B)

.. note:: Our model currently only works with a fixed packet size of 64B

Training the neural network
---------------------------

.. how to train

  - using keras
  - the neural network model (its up for optimization)
  - the script which builds the neural network model and trains it
  - the result is model and h5 files

We use Keras to build and train the neural network. The network used in this showcase has the following structure:

.. **TODO** better figures

.. figure:: graph.svg
   :width: 90%
   :align: center

.. figure:: legend.svg
   :width: 90%
   :align: center

.. It uses 64 + 32 + 1 neurons in the dense layers TODO.

.. Here it is in Keras' model summary function:

.. .. code-block:: text

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

The ``train-neural-network.py`` script creates the Keras model, and trains it on the training datasets.

.. **TODO** repeat = 8 so there are 8 log files

.. **V1** Specify the log files as an argument to the script (note that there are eight log files hence the ``*``):

When running the script, specify the list of log files as an argument (note that there are eight log files hence the ``*``):

.. - For ApskRadio:

  .. code-block:: bash

     ./train-neural-network.py ApskRadio_36Mbps_Bpsk_2.4GHz_20MHz_*.log

.. - For 802.11:

.. code-block:: bash

   ./train-neural-network.py Ieee80211OfdmRadio_24Mbps_Ieee80211Ofdm_Qam16_2.412GHz_20MHz_*.log

After the training process is completed, the structure and weights of the neural network model are saved to a ``.model`` file, e.g. ``Ieee80211Radio_24Mbps_Ieee80211Ofdm_Qam16_2.412GHz_20MHz.model``, which can be used by INET's integrated inference library (keras2cpp) at runtime.

.. **TODO** which contains the TODO in the filename, more on this in the next section

Using the neural network
------------------------

.. technical details

  - creating training dataset
  - training the network
  - using it as an error model

.. The :ned:`NeuralNetworkErrorModel` module can use the trained neural networks as error models in simulations. The module takes the neural network file names as a parameter, and by default chooses the neural network model required for each reception based on class name, modulation, bit rate, center frequency, and bandwidth contained in the filenames. The selection can be specified with the :par:`modelNameFormat` parameter.

The :ned:`NeuralNetworkErrorModel` module can use the trained neural networks as error models in simulations. The module takes the neural network file names as a parameter (:par:`neuralNetworkModelFilenames`). The module chooses the neural network model required for a given reception based on these filenames. By default, the module expects the filename to contain the class name (``%c``), modulation (``%m``), bit rate (``%r``), center frequency (``%c``), and bandwidth (``%b``). This selection can be specified by the :par:`modelNameFormat` parameter.

For example, in this showcase the error model for 802.11 is configured as follows:

.. literalinclude:: ../compare-ieee80211radio-error-models.ini
   :start-at: errorModel.typename = "NeuralNetworkErrorModel"
   :end-at: neuralNetworkModelFilenames
   :language: ini

.. **TODO** it also uses the subcarrier modulation

Note that the model name format also includes the subcarrier modulation (``%M``).

.. .. note:: With this process, a neural network error model can be created for any wireless technology which has a symbol-level simulation model.

.. And the same for :ned:`ApskRadio`:

  .. literalinclude:: ../compare-apskradio-error-models.ini
     :start-at: errorModel.typename = "NeuralNetworkErrorModel"
     :end-at: neuralNetworkModelFilenames
     :language: ini

.. **TODO** ini files and scripts

Comparison
----------

.. so

   - compare the accuracy and the performance of the packet level analytical error models and the neural network error models with the baseline symbol-level simulations

.. in terms of accuracy and performance

The ``compare-error-models.py`` script can be used to generate results for the three cases (symbol-level, packet-level analytical, packet-level neural network), which then can be compared with the analysis tool of the IDE. The script takes an ini file as argument, and runs simulations of receptions for the three cases.
The simulations are defined in :download:`compare-ieee80211radio-error-models.ini <../compare-ieee80211radio-error-models.ini>`.

.. note:: The simulations take several minutes to finish.

..  (it runs in Cmdenv)

.. **TODO** include models and results in the repo? (because it takes long)

``CompareIeee80211RadioErrorModels.anf`` contains the charts for Received packets (%) vs Number of noise sources, for the different values of noise duration and power. The displayed noise duration and power values can be changed with sliders interactively. Some of the charts are included in the following sections.

.. Here are some of the charts:

.. ApskRadio
   ~~~~~~~~~

.. Ieee80211Radio
   ~~~~~~~~~~~~~~

.. The ``CompareIeee80211RadioErrorModels.anf`` file contains the charts for Received packets (%) vs Number of noise sources, for the different values of noise duration and power. The displayed noise duration and power values can be changed with sliders interactively. Here are some of the charts:

At low power and duration, both the analytical and the neural network based packet level error models match the baseline symbol level simulation fairly well. At high number of noise sources, the analytical model has around 10% difference from the baseline:

itt rövid zajok kicsi a power igy az analytical se rossz

.. figure:: media/1_2.png
   :align: center
   :width: 100%

At longer duration, the neural network based error model gives a better estimation than the analytical. The difference from the baseline gets increasingly larger for the analytical model; at 20 noise sources, the difference is substantial, ~95% vs ~1% (100x difference). However, this difference for the neural network is less than 50% at most in the whole range (2x difference):

miért van ez? -> ha a packet levelben sok noise van es idoben osszeernek akkor tobb lesz a power -> ? we dont know

.. figure:: media/1_8.png
   :align: center
   :width: 100%

At even higher duration, the neural network is still a better match, because the analytical model gives incorrect results in most of the range:

.. figure:: media/1_16.png
   :align: center
   :width: 100%

At higher power, the analytical error models estimation is mostly incorrect, except when no packets can be received due to the high number of noise sources. The neural network still follows the baseline better when packets can be received:

1 noise sourcenal a packet level ugy veszi hogy vegig az a snir -> minSnir-t néz

.. - at high number of noise sources, the analytical follows the baseline better. At lower number of noise sources, the relative difference between the analytical and the baseline is large (up to 1000x), for the neural network at most 2x. However,

.. **TODO**

.. figure:: media/5_10.png
   :align: center
   :width: 100%

At high power the neural network based error model's estimation is has around 2x difference. The analytical error model gives incorrect results in the first half of the range:

.. figure:: media/10_6.png
   :align: center
   :width: 100%

.. At the highest power and duration, all three match when no packets can be received (yet the neuralnetwork matches the baseline more:

At the highest power and duration, the analytical model drops to zero, while the neural network model follows the baseline:

.. figure:: media/10_16.png
   :align: center
   :width: 100%


Limitations and further research
--------------------------------

.. so

  - this showcase is basically a proof-of-concept
  - not final
  - further research is needed
  - the structure of the neural network can be optimized
  - need to be validated for more packet sizes
  - one network which contains each modulation vs many smaller networks for each modulation

  limitations:

  - was tested with fixed packet sizes
  - generally better than analytical, but 2x slower (as opposed to 10x slower for the symbol-level)

  - can be created for any wireless technology which has symbol-level error models/accurate symbol-level model

The neural network based error model used in this showcase is a proof-of-concept, not a final version. Currently, it has several limitations, such as that it has been only tested with a fixed packet size. It is generally better than the analytical error models, but runs two times slower (as opposed to the symbol-level simulation which runs ten times slower). Here are some of the limitations:

- It only works with fixed packet sizes
- There is no reference model; this is just a showcase

.. - There is no reference neural network models for any technologies
   - Nem adunk referencia modelt, ez csak egy showcase
   (szélesebb körű training data kéne)

.. - Neural networks for more modulations, bit rates and technologies can be created

Further research is needed, e.g. in the following areas:

- The structure of the neural network can be fine-tuned to the problem
- Use one network for all modulations, or several smaller networks for each modulation

This showcases demonstrated a 802.11 neural network based error model, but this method can create neural network based wireless error models for any wireless technology which has symbol-level simulation.
