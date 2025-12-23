Step 9. Measuring RIP Recovery Time
===================================

Introduction
------------

In the previous steps, we've explored various mechanisms for preventing routing
loops and improving convergence in RIP networks, including split horizon, poison
reverse, triggered updates, and hold-down timers. Now, we'll focus on a critical
performance metric: recovery time.

Recovery time refers to how quickly a network can adapt to topology changes and
restore connectivity. In RIP networks, recovery time is influenced by various
factors, including the update interval, triggered updates, and the specific
mechanisms used to prevent routing loops. Understanding these factors can help
network administrators optimize RIP configurations for their specific
requirements.

Goals
-----

In this step, our goals are to:

1. Measure RIP recovery time under different configurations
2. Understand the impact of triggered updates on recovery time
3. Analyze the trade-offs between stability and recovery speed
4. Explore how different RIP features affect network performance

Network Configuration
---------------------

This step uses the same network topology as the previous steps, with a focus on
measuring recovery time after a link failure and subsequent recovery. We'll
compare different configurations to understand their impact on recovery time.

The basic configuration in ``omnetpp.ini`` is:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step9
   :end-before: ------

Key aspects of this configuration:

* We use UDP applications to generate continuous traffic between ``host0`` and ``host6``
* Split horizon is enabled for stability
* We test with both triggered updates enabled and disabled
* The simulation runs for a longer period (1500 seconds) to observe complete recovery

The scenario manager script controls the link failure and recovery:

.. literalinclude:: ../scenario7.xml
   :language: xml

This script:

1. Disconnects the link between ``router2`` and ``switch1`` at t=50s
2. Reconnects the link at t=400s (after both timeout and garbage-collection timers would have expired)

We also test an alternative configuration without netmask routes:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step9NoNetmaskRoutes
   :end-before: ------

Experiment: Measuring Recovery Time
-----------------------------------

In this experiment, we measure how long it takes for the network to recover
connectivity between ``host0`` and ``host6`` after a link failure and subsequent
recovery. We compare recovery times with and without triggered updates to
understand their impact.

The recovery process involves several phases:

1. **Failure Detection**: How quickly routers detect the link failure
2. **Route Invalidation**: How quickly invalid routes are marked as unreachable
3. **Route Propagation**: How quickly this information propagates through the network
4. **Alternate Path Discovery**: How quickly alternate paths are discovered
5. **Route Recovery**: How quickly routes are restored after the link recovers

.. TODO: Results of the experiment would be shown here, including graphs or tables comparing recovery times with and without triggered updates

Impact of Triggered Updates on Recovery Time
--------------------------------------------

Triggered updates can significantly impact recovery time in RIP networks:

**With Triggered Updates Enabled**:

1. Routers send immediate updates when their routing tables change
2. This accelerates the propagation of routing information after a topology change
3. The network can converge more quickly to a stable state
4. Recovery time is typically reduced, especially in larger networks

**Without Triggered Updates**:

1. Routers only send updates at regular intervals (typically every 30 seconds)
2. Routing information propagates more slowly through the network
3. Convergence takes longer, especially in networks with many hops
4. Recovery time is typically longer, but network overhead is reduced

.. TODO: Specific measurements from the simulation would be included here

Trade-offs Between Stability and Recovery Speed
-----------------------------------------------

When configuring RIP networks, administrators must balance stability and recovery speed:

**Faster Recovery**:

* Enables triggered updates
* Uses shorter update intervals
* Minimizes hold-down timers
* Prioritizes quick adaptation to changes

**Greater Stability**:

* Uses longer hold-down timers
* Implements split horizon and poison reverse
* May disable triggered updates in unstable networks
* Prioritizes preventing routing loops and oscillations

The optimal configuration depends on specific network requirements:

* Mission-critical networks may prioritize stability over recovery speed
* Real-time applications may require faster recovery at the cost of some stability
* Large networks may need longer timers to prevent excessive updates

Optimizing RIP Performance
--------------------------

Based on our experiments, we can make several recommendations for optimizing RIP performance:

1. **Enable Triggered Updates**: In most networks, triggered updates significantly improve recovery time without major drawbacks
2. **Use Split Horizon**: Always enable split horizon to prevent simple routing loops
3. **Consider Network Size**: Adjust timers based on network size and complexity
4. **Monitor Network Stability**: In unstable networks, consider longer hold-down timers and possibly disabling triggered updates
5. **Use Route Summarization**: Summarize routes where possible to reduce update size and limit the scope of routing changes

Conclusion and Next Steps
-------------------------

In this step, we've explored how different RIP configurations affect recovery
time after network changes. We've seen that triggered updates can significantly
improve recovery time, though there are trade-offs to consider regarding network
stability and overhead.

In the final step of this tutorial, we'll explore how to configure RIP
interfaces, including how to disable RIP on specific interfaces to optimize
protocol operation and reduce unnecessary routing traffic.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkC.ned <../RipNetworkC.ned>`,
:download:`scenario3.xml <../scenario3.xml>`,
:download:`scenario7.xml <../scenario7.xml>`
