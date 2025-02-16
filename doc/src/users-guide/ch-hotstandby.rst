.. role:: raw-latex(raw)
   :format: latex
..

.. _ug:cha:hotstandby:

Hot Standby
===========

.. _ug:sec:hotstandby:overview:

Overview
--------

In time-sensitive networking (TSN), precise and uninterrupted time synchronization
is fundamental to ensuring deterministic network behavior. Many real-time applications,
such as industrial automation, financial trading, and vehicular communication, depend on
synchronized clocks with minimal jitter and drift. However, conventional network simulations
often assume a globally shared clock, which does not account for real-world clock
discrepancies and failures.

In practice, hardware clocks are prone to drift, and failures in the primary time source can
introduce significant disruptions. To mitigate these risks, the concept of a hot-standby master
clock is introduced. This approach enhances the robustness of time synchronization mechanisms by
ensuring that a backup master clock seamlessly takes over in the event of a failure, minimizing
synchronization loss and preventing cascading system failures.

The gPTP Hot-Standby model aims to simulate such a resilient setup within a communication network,
demonstrating how redundancy in master clocks ensures continued synchronization even under failure
conditions.

The gPTP Hot-Standby model simulates a redundant time synchronization architecture comprising the
following components:

- Primary Master Clock: The primary source of time synchronization for all connected slave clocks.
- Hot-Standby Clock: A backup master clock that takes over in the event of a failure in the
  primary master clock.
- Slave Clocks: Devices that synchronize their clocks with the master clock, ensuring a consistent
  time reference across the network.

**Key Features**:
- **Primary and Hot-Standby Master Clocks**: The system operates with two master clocks, where the
  standby clock takes over when the primary fails.
- **Automatic Failover Mechanism**: The switch to the standby master occurs seamlessly, ensuring
  continuous time synchronization.
- **Multi-domain Synchronization**: Supports multiple time domains, ensuring redundancy in network timing.

HotStandby Users
----------------

The easiest way to use the HotStandby module is to add a :ned:`HotStandby` module to the network node
where redundancy is required and set the :bool:`hasHotStandby` parameter to `true`. This allows the node
to function as a backup master clock, automatically taking over time synchronization in the event of a primary
master failure.

Adjusting Clock Failure Scenario in HotStandby
----------------------------------------------

The :ned:`HotStandby` module is instantiated in network nodes where redundancy is required. The module follows the
INET time synchronization framework and integrates seamlessly with the gPTP implementation. In the event of a failure,
the HotStandby module automatically switches to the standby master clock, ensuring uninterrupted synchronization.
It supports creating a failure scenario with the following command:

.. code-block:: xml

   <set-channel-param t="3.1s" src-module="tsnClock1" dest-module="tsnSwitch1" par="disabled" value="true"/>
   <set-channel-param t="7.1s" src-module="tsnClock1" dest-module="tsnSwitch1" par="disabled" value="false"/>

The above command disables the channel between the primary master clock and the switch at 3.1 seconds and re-enables
it at 7.1 seconds, simulating a failure and recovery scenario.