Automatic Gate Schedule Configuration
=====================================

Automatic gate schedule configurators generate and install TAS gate schedules
based on network topology and traffic requirements. The following showcases
demonstrate three configurators that INET provides: an eager configurator that
takes a straightforward greedy approach that often fails and therefore is mostly
useful as an example; a more sophisticated one that employs a SAT solver (Z3) to
be able to satisfy the latency and jitter requirements; and a third one that
uses an external tool (TSNsched) for generating the schedule.

The following showcases demonstrate these configurators:

.. toctree::
   :maxdepth: 1

   eager/doc/index
   sat/doc/index
   tsnsched/doc/index
