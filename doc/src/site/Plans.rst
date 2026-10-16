=====
Plans
=====

This page describes what's planned for INET in the future.

After INET 4.3
--------------

1. mobility models

   - separate initial positioning from positioning over time
   - make them composable sequentially and additively
   - extend interface with future movement predictions

2. physical layer

   - support capturing a stronger signal while receiving a weaker one
   - support aborting an ongoing transmission
   - add the simulation of synchronization process (pilot/preamble)
   - add missing Weibull and Jakes fading
   - add multipath support
   - add real CUDA and multi-core parallel execution

3. IPv6

   - IPv6 has not yet been through the same refactoring and improvements as the IPv4 model
   - after that, xMIPv6 needs to be tested and revised as well

4. NeuralNet based Error model

5. Documentation

   - review new and modified NED module and C++ class comments

6. Examples

   - review existing examples
   - add new more meaningful, more specific feature examples (e.g. crosstalk)
   - reduce redundancy in inifiles (e.g. lots of unrelated configuration entries) that makes understanding examples more difficult than necessary
   - review directory structure (e.g. more different wireless directories), make the structure more similar to the source directory structure
   - extend the wireless tutorial with additional steps

7. Tests

   - some examples are rather tests, they were usually created during development for quick testing
   - we definitely need more (some?) validation tests (e.g. comparison with other simulators, real-world measurements, or analytical results)

As for the not-so-near future (potentially in a later major version), we are considering some nontrivial changes:

1. synchronous sends

   - introducing synchronous (immediate) message sends to reduce communication overhead between protocol layers

2. Factor out the base infrastructure from INET so we would have a subset of components that could be separately supported.
   Protocol-specific code should be avoided in this codebase.

3. Based on the common infrastructure create a framework where generic network models could be built using only the
   combination of exsisting (very simple) modules. This could be useful for modeling abstract network concepts.
