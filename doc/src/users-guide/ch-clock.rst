.. role:: raw-latex(raw)
   :format: latex
..

.. _ug:cha:clock:

Clock Model
===========

.. _ug:sec:clock:overview:

Overview
--------

In most communication network simulations, time is simply modeled as a global
quantity. All components of the network share the same time throughout the
simulation independently of where they are physically located or how they are
logically connected to the network.

In contast, in time sensitive networking, the bookkeeping of time is an essential
part, which should be explicitly simulated independently of the underlying global
time. The reason is that the differences among the local time of the communication
network components significantly affects the simulation results.

In such simulations, hardware clocks are simulated on their own, and communication
protocols don't rely on the global value of simulation time, which is in fact
unknown in reality, but on the value of their own clocks. With having hardware
clocks modeled, it's also often required to use various time synhronization
protocols, because clocks tend to drift over time and communication protocols
rely on the precision of the clocks they are using.

In INET, the clock model is a completely optional feature, which has no effect
on the simulation performance when disabled. Even if the feature is enabled,
the usage of clock modules by communication protocols and applications is still
optional, and enabling the feature has negligible performance hit when not in
use. For testing that the mere usage of a clock has no effect on the simulation
results, INET also includes an ideal clock mechanism.

Clocks
------

Clocks are implemented as modules, and are used by other modules via direct C++
method calls. Clock modules implement the :ned:`IClock` module interface and
the corresponding :cpp:`IClock` C++ interface.

The C++ interface provides an API similar to the standard OMNeT++ simulation time
based scheduling mechanism, but it relies on the underlying clock implementation
for (re)scheduling events according to the clock. These events are transparently
scheduled for the client module, and they will be delivered to it when the clock
timers expire.

The clock API uses the clock time instead of the simulation time as arguments and
return values. The interface contains functions such as :fun:`getClockTime()`,
:fun:`scheduleClockEventAt()`, :fun:`scheduleClockEventAfter()`,
:fun:`cancelClockEvent()`.
 
INET contains optional clock modules (not used by default) at the network node
and the network interface levels. The following clock models are available:

-  :ned:`IdealClock`: clock time is identical to the simulation time.
-  :ned:`OscillatorBasedClock`: clock time is the number of oscillator ticks
   multiplied by the nominal tick length.
-  :ned:`SettableClock`: a clock which can be set to a different clock time. 

Clock Time
----------

In order to avoid confusing the simulation time (which is basically unknown to
communication protocols and hardware elements) with the clock time maintained
by hardware clocks, INET introduces a new C++ type called the :cpp:`ClockTime`.

This type is pretty much the same as the default :cpp:`SimTime`, but the two
types cannot be implicitly converted into each other. This approach prevents
accidentally using clock time where simulation time is needed, and vice versa.
Simlarly to how :cpp:`simtime_t` is an alias for :cpp:`SimTime`, INET also
introduces the :cpp:`clocktime_t` alias for :cpp:`ClockTime` type.

For the explicit conversion between clock time and simulation time, one can use
the :cpp:`CLOCKTIME_AS_SIMTIME` and the :cpp:`SIMTIME_AS_CLOCKTIME` C++ macros.
Note that these macros don't change the numerical value, they simply convert
between the C++ types.

When the actual clock time is used by a clock, the value may be rounded according
to the clock granularity and rounding mode (e.g. :ned:`OscillatorBasedClock`). For
example, when a clock with a us granularity is instructed to wait for 100 ns,
while its oscillator is right in the middle of its ticking period, it may actually
wait for the next tick to happen to start the timer, and wait another tick to
happen to account for the requested wait time interval.

Oscillators
-----------

The clock interface is quite general in the sense that it allows many different
ways to implement it. Nevertheless, the most common way is to use an oscillator
based clock model.

An oscillator efficiently models the periodic generation of ticks that are usually
counted by a clock module. The tick period is not necessarily constant, it can
change over time. Oscillators implement the :ned:`IOscillator` module interface
and the corresponding :cpp:`IOscillator` C++ interface.

The following oscillator models are available:

-  :ned:`IdealOscillator`: generates ticks periodically with a constant length
   (mostly useful for testing).
-  :ned:`ConstantDriftOscillator`: tick length changes proportional to the elapsed
   simulation time (clock drift).
-  :ned:`RandomDriftOscillator`: updates clock drift with a random walk process.

Clock Users
-----------

The easiest way to use a clock in applications and communication protocols is
to add a `clockModule` parameter that specifies where the clock module can be
found. Then the C++ user module should be simply derived from either
:cpp:`ClockUserModuleBase` or the parameterizable :cpp:`ClockUserModuleMixin`
base classes. The clock can be used via the inherited clock related methods
or through the methods of the :cpp:`IClock` C++ interface on the inherited
clock field.

Clock Events
------------

The clock model requires the use of a specific C++ class called :cpp:`ClockEvent`
to schedule clock timers. It's also allowed to derive new C++ classes from
:cpp:`ClockEvent` if necessary. In any case, clock events must be scheduled and
canceled via the :cpp:`IClock` C++ interface to operate properly.

Controlling Clocks According to a Scenario
------------------------------------------

In order to support the simulation of specific scenarios, where the clock time
or the oscillator drift must be changed according to a predefined script, INET
provides clocks and oscillators that implement the interface required by the
:ned:`ScenarioManager` module. This allows the user to update the clock and
oscillator state from the :ned:`ScenarioManager` XML script and to also mix
these operations with many other supported operations.

For example, the :ned:`SettableClock` model supports setting the clock time and
also to optionally reset the oscillator at a specific moment of simulation time
as follows:

.. code-block:: xml

   <set-clock at="10 s" module="server.clock" time="1.2 s" reset-oscillator="true"/>

The above example means that the clock time of the server node's clock will be
set to 1.2 seconds when the simulation time reaches 10 seconds, and the clock's
oscillator will restart its duty cycle.

For another example, the :ned:`ConstantDriftOscillator` supports changing the
state of the oscillator with the following command:

.. code-block:: xml

   <set-oscillator at="10 us" module="server.clock.oscillator" drift-rate="42 ppm" tick-offset="1 us"/>

This example simultaneously changes the drift rate and the tick offset of the
oscillator in the server node's clock.

