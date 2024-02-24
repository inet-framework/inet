TCP NewReno Congestion Control
==============================

Goals
-----

This showcase aims to demonstrate the fundamental operation of the TCP NewReno
congestion control algorithm, particularly focusing on its slow start and
congestion avoidance phases. We will use a simple network setup to illustrate
how TCP NewReno adjusts its congestion window (cwnd) in response to network
conditions to optimize throughput while avoiding congestion.

The Model
---------

Our simulation environment consists of two main nodes: a TCP
sender and a TCP receiver, connected by a router. This setup
helps to clearly observe the behavior of TCP NewReno without the complexities of
a larger network:

.. figure:: media/Network.png
   :align: center

Here is the network's NED definition:

.. literalinclude:: ../TcpCongestionShowcase.ned
   :start-at: TcpCongestionShowcase
   :language: ned

The bottleneck link between the router and the sink introduces controlled delay
and loss to mimic real-world network congestion. Also, the router's eggress
queue is limited to 10 packets. This setup allows us to observe how
TCP NewReno reacts to such network conditions. 

Here is the configuration in omnetpp.ini:

.. literalinclude:: ../omnetpp.ini
   :language: ini

.. Key Components:

.. TCP Sender: Initiates a data transfer using the TCP NewReno variant.
.. TCP Receiver: Acknowledges received packets, simulating a typical data reception
.. in a TCP connection. -> useful info: the sender uses the NewReno congestion control algorithm.

.. Bottleneck Link: Introduces controlled delay and loss to
   mimic real-world network congestion, allowing us to observe how TCP NewReno
   reacts to such conditions. 

TCP Congestion Control Phases
-----------------------------

Slow Start: Initially, TCP NewReno enters the slow start phase, where the cwnd
increases exponentially with each round-trip time (RTT) until it reaches the
slow start threshold (ssthresh) or a packet loss occurs. This phase is
characterized by rapid growth in the cwnd, as the algorithm probes the network
to find its capacity.

Congestion Avoidance: Once the cwnd reaches ssthresh or after a packet loss, TCP
NewReno transitions to the congestion avoidance phase. In this phase, the cwnd
grows linearly, increasing by one full-sized segment per RTT. This cautious
approach allows for efficient bandwidth utilization while minimizing the risk of
inducing further congestion.

Results
-------

After running the simulation, we plot the cwnd and ssthresh over time,
highlighting the transition from the exponential growth during the slow start
phase to the linear increase during congestion avoidance.

.. figure:: media/cwnd.png
   :align: center

.. Conclusion: Through this showcase, participants will gain insights into the
.. adaptive nature of TCP NewReno's congestion control mechanism. By observing the
.. cwnd plot, users can understand the algorithm's strategies for optimizing data
.. transmission over varying network conditions, ensuring reliable and efficient
.. communication. -> not needed as is; explain the chart instead?

Further Exploration: Users are encouraged to experiment with different network
parameters such as link bandwidth, delay, and packet loss rates to observe their
impact on TCP NewReno's performance. Additionally, comparisons with other TCP
variants can provide deeper insights into the evolution and diversity of
congestion control algorithms.