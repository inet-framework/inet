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

The default analytical error models in INET are generally accurate, except for some corner cases, which can be abundant in simulations. Symbol-level simulations are very accurate even in corner cases where the analyitcal ones are not, but very computationally intensive.

.. Neural-network-based error models aim to/can potentially achieve the accuracy of symbol level simulations, and the speed of analytical models.

.. A neural network can be trained on reception data from symbol level simulations, and used as accurate error models in packet level simulations

A neural network can be trained on reception characteristics/data/properties/measurements/features of symbol-level simulations, and used as an error model in packet-level simulations.
The resulting error model has the potential to be more accurate than packet-level analytical error models, but with comparable performance.

This showcase demonstrates the workflow of creating such a neural network based error model for 802.11, and compares it..

Motivation
----------

.. The packet-level analytical error model (:ned:`Ieee80211NistErrorModel`) is used in many examples, showcases and tutorials in INET, it's a kind of informal default.

.. The default error models in scalar (all?) receivers are analytical.

The packet-level analytical error models calculate a packet-error rate based on the reception SNIR. They either use the minimum or the mean of the SNIR during reception. Both methods can lead to unrealistic reception probabilities in corner cases. For example, when using the minimum SNIR, a short spike in an interfering signal can ruin a reception; with mean SNIR, an interfering signal overlapping with a transmission to a large extent (in frequency or time) can still result in a correctly receivable transmission.

Using symbol level simulation can accurately model these corner cases, as per-symbol-SNIR can be used, and an accurate symbol-error-rate can be calculated. However, the symbol-level simulations are very computationally intensive.

**TODO** what are error models ?
