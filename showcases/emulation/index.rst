Emulation
=========

In INET, the word *emulation* is used in a broad sense to describe a system which
is partially implemented in the real world and partially in simulation.
Emulation is often used for testing and validating a simulation model with
its real-world counterparts. It may also be used out of necessity, because
some part of the system only exists in the real world or only as a simulation
model.

Developing a protocol, a protocol implementation, or an application that heavily
relies on network communication is often less expensive, more practical,
and safer using simulation than directly performing experiments in the real world.
However, there are potential pitfalls: porting simulation code to the target device
may be costly and error prone, and also, a model that performs well in simulation
does not necessarily work equally well when deployed in the real world.
INET helps reducing these risks by allowing the researcher to mix
simulation and real world in various ways, thereby reducing the need for porting,
and offering more possibilities for testing out the code.

INET provides modules that act as bridges between the
simulated and real domains, therefore it is possible to leave one part
of the simulation unchanged, while simply extracting the other into the
real world. Several setups are possible when one can take advantage of the emulation
capabilities of INET:

.. TODO: simulated protocol in a real network node
.. TODO: real application in a simulated network node
   etc.

- simulated node in a real network
- a simulated subnet in real network
- real-world node in simulated network
- simulated protocol in a real network node
- real application in a simulated network node
- etc.

For the simulation to act as a network emulator, two major problems need to be solved.
On one hand, the simulation must run in real time, or the real clock must be
configured according to the simulation time (synchronization). On the
other hand, the simulation must be able to communicate with the real
world (communication). This is achieved in INET as the following:

- Synchronization:

  - ``RealTimeScheduler:`` It is a socket-aware real-time
    scheduler class responsible for synchronization. Using this method, the
    simulation is run according to real time.

  - (``OppClock``: A hypothetical method; the clocks of real applications running on the host OS are synchronized to the clock of the simulation. Note that this is just an idea; it's not available yet.)

-  Communication:

   -  The interface between the real (an interface of the OS) and the
      simulated parts of the model are represented by `Ext` modules,
      with names beginning with ``Ext~`` prefix in the
      simulation (``ExtLowerUdp``, ``ExtUpperEthernetInterface``,
      etc.).

      Ext modules communicate both internally in the simulation and externally in the host OS.
      Packets sent to these modules in the simulation will be sent out on the host
      OS interface, and packets received by the host OS interface (or
      rather, the appropriate subset of them) will appear in the
      simulation as if received on an ``Ext~`` module.


      There are several possible ways to maintain the external communication:

      -  File
      -  Pipe
      -  Socket
      -  Shared memory
      -  TUN/TAP interfaces
      -  Message Passing Interface (MPI)

In order to run any emulation example, INET must be compiled with the
"Network Emulation" feature enabled. Enabling this feature can be done
by checking the
:menuselection:`IDE-->Project-->Properties-->OMNeT++/Project Features-->Network Emulation`
options.

In order to be able to send packets through raw sockets
applications require special permissions. There
are two ways to achieve this under linux.

The suggested solution is to use setcap to set the application
permissions:

.. code::

   $ sudo setcap cap_net_raw,cap_net_admin=eip /*fullpath*/opp_run
   $ sudo setcap cap_net_raw,cap_net_admin=eip /*fullpath*/opp_run_dbg
   $ sudo setcap cap_net_raw,cap_net_admin=eip /*fullpath*/opp_run_release

This solution makes running the examples from the IDE possible.
Alternatively, the application can be started with root privileges from
command line:


.. code::

   $ sudo `inet_dbg -p -u Cmdenv`

.. note:: In any case, it's generally a bad idea to start the IDE as superuser.
          Doing so may silently change the file ownership for certain IDE
          configuration files, and it may prevent the IDE to start up for the
          normal user afterwards.

The following showcases demonstrate several such emulation examples:

.. toctree::
   :maxdepth: 1

   basic/doc/index
   voip/doc/index
   babel/doc/index



.. todo::

   #1 Emulation concepts in INET; Connecting the Real and the Simulated World (two nodes one cable, every possible way to separate the simulation from the real world, one node pings the other)
   Real Video Streaming over Simulated Network
   Simulating VoIP Applications over the Real Network
   Real AODV Routing over Simulated Wireless Network
   Simulated GPSR Routing over Real Network
   Validating *something*

   Validation/Verification: maybe separate top level showcases? these are related to emulation

   see user guide for emulation
   simulation + real = emulation

   a system can be thought of being completely real or completely simulated or in between, the latter is called emulation
   the interface between the real and the simulated parts are represented by the Ext* modules in the simulation, in the real world the interface is often void or some configuration carried out on the OS

   synchronization:
       realtime scheduler: szimuláció valós időben fut, ha lemarad utoléri
       oppclock: real world óráját állítgatja az OS-en futó real applicationöknek
   communication between simulated and real world:
       filesystem
       pipes
       sockets
       shared memory
       tun/tap devices
       mpi
   modules representing the interface between the real and the simulated word:
       ExtLowerUdp, ExtIpv4, ExtEthernetInterface, Ext* ...

   optimization:
       disable optional features, compile static executable, release mode, etc. -> fast and small memory footprint makes it possible to use in embedded devices
       General Example: Optimizing Memory Footprint (or whatever)

   interprocess communication -> communication between the simulation and the real world

   - File: A record stored on disk, or a record synthesized on demand by a file server, which can be accessed by multiple processes.
   - Pipe: A unidirectional data channel. Data written to the write end of the pipe is buffered by the operating system until it is read from the read end of the pipe. Two-way data streams between processes can be achieved by creating two pipes utilizing standard input and output.
   - Socket: Data sent over a network interface, either to a different process on the same computer or to another computer on the network. Stream-oriented (TCP; data written through a socket requires formatting to preserve message boundaries) or more rarely message-oriented (UDP, SCTP).
   - Shared memory: Multiple processes are given access to the same block of memory which creates a shared buffer for the processes to communicate with each other.
   - TUN/TAP: TUN (namely network TUNnel) simulates a network layer device and it operates with layer 3 packets like IP packets. TAP (namely network tap) simulates a link layer device and it operates with layer 2 packets like Ethernet frames. TUN is used with routing, while TAP is used for creating a network bridge.
   - MPI: Message Passing Interface (MPI) is a standardized and portable message-passing standard.
