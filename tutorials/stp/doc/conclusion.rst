Conclusion
==========

Congratulations! You have completed the STP/RSTP tutorial for the INET Framework.

You have learned:

- Why loops in bridged Ethernet networks are harmful (broadcast storms)
- How STP builds a loop-free spanning tree through root bridge election, port role assignment, and the Blocking/Listening/Learning/Forwarding state machine
- How to control the root bridge selection using bridge priorities
- Why STP convergence takes approximately 50 s and what the port states mean
- How RSTP achieves convergence in approximately 6 s using edge ports and the proposal/agreement mechanism
- How both protocols handle switch failures and link reconnects, with RSTP recovering much faster

In practice, RSTP (or its successor Multiple Spanning Tree Protocol, MSTP) is
preferred over the original STP in all modern networks due to its significantly
faster convergence. The INET :ned:`Rstp` module implements IEEE 802.1D-2004 and
provides a complete simulation model for studying these behaviors.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
