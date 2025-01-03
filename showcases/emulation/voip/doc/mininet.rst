Using Mininet to Set Up the Virtual Network
===========================================

Goals
-----

This showcase builds upon the previous one, :doc:`index`, and demonstrates how
setting up the virtual network can be vastly simplified using Mininet.

| INET version: ``4.5``
| Source files location: `inet/showcases/emulation/voip <https://github.com/inet-framework/inet/tree/master/showcases/emulation/voip>`__


Introduction
------------

`Mininet <https://mininet.org>`__ is a software package on Linux for setting up a virtual
network on the local computer. Using Mininet, one can create a virtual network
topology with just a few commands. Each host (and switch, router, etc) has its
own virtual network stack, and one can run commands on these virtual hosts.

Mininet utilizes network namespaces and virtual network interfaces of the Linux
kernel under the hood. One just needs to specify the network topology (the
number of virtual hosts and switches, and connections between them), then
Mininet handles the creation of network namespaces, virtual network interfaces,
and routes.

Mininet scenarios can be run using its command line tool ``mn``, or via Python
scripts. This showcase uses the Python API.


Installing Mininet
------------------

On Debian and derivatives like Ubuntu, install with the following command:

.. code-block:: bash

   $ sudo apt install mininet openvswitch-testcontroller

The Setup
---------

To recap, the goal of this showcase is to simulate transmission of a VoIP stream
over a virtual network, where both the VoIP sender and receiver are simulations
that are run in real-time. We use Mininet to set up a similar, but slightly extended
network topology than in the previous showcase: two hosts connected via a
switch. The simulations that run the VoIP sender and receiver are the same as in
the previous showcase.

The setup is the following:

.. figure:: media/setup4.png
   :align: center
   :width: 40%

We use the following Python script in the ``mininet-veth.py`` file:

.. literalinclude:: ../mininet-veth.py
   :start-at: net = Mininet

The Python file can be run with the ``/run_mininet.sh`` shell script:

.. literalinclude:: ../run_mininet.sh

The Python script should be fairly self-explanatory with the ``addhost()``,
``addSwitch()``, and ``addLink()`` calls, and the two ``cmd()`` calls that
start the simulated apps on the virtual hosts. It is easy to see how this
example can be extended to create more complex topologies. Nevertheless, some
explanations are in order.

-  The ``TCIntf`` interface type is used to configure the virtual network
   interfaces. This allows the use of Traffic Control (TC) features of the Linux
   kernel to be used for the links, such as configuring the link bandwidth, packet
   loss, delay, jitter, etc.

-  Note how the IP addresses of the hosts match in the Python script and in the
   ``omnetpp.ini`` file.

-  Note the use of ``sudo`` at several places, and the use of the ``SAVED_USER``
   and ``SAVED_PATH`` environment variables. ``sudo`` is needed because Mininet
   needs to run as root (it can only do system-wide changes like creating network
   namespaces if it has the necessary permissions), but we want to run the
   simulations as a normal user. Once in Mininet, the ``SAVED_USER`` variable is
   used to restore the current user for the simulations. The ``SAVED_PATH``
   variable is used to restore the ``PATH`` environment variable that ``sudo``
   overwrites.

-  The ``CLI()`` call opens a Mininet prompt. Its purpose is to suspend the
   execution of the Python script until the user, after having finished with the
   experiment, presses Ctrl-D. Without it, the Python script would immediately
   continue to the end of the script where the virtual network infrastructure is
   deleted.

To run the simulation, use the ``run_mininet.sh`` script:

.. code-block:: bash

  $ ./run_mininet.sh

.. note:: When starting Mininet, the following error might occur:
   
   .. code-block:: shell
   
      Exception: Please shut down the controller which is running on port 6653:
      Active Internet connections (servers and established)
      tcp        0      0 0.0.0.0:6653            0.0.0.0:*               LISTEN      1335/ovs-testcontro

   If that happens, kill the controller process with ``sudo kill -9 <PID>``. In this case, the PID is 1335.


When the simulations in Qtenv are loaded, make sure to start the receiver one
first, and run both in Fast or Express mode. After the simulations are finished,
exit the Mininet prompt with Ctrl-D to delete the virtual network
infrastructure.

Results
-------

Here are the results of the transfer via the lossy link. As a reference, you can
listen to the original audio file by clicking the play button below:

.. raw:: html

   <p><audio controls> <source src="media/Beatify_Dabei_cut.mp3" type="audio/mpeg">Your browser does not support the audio tag.</audio></p>

Here is the received audio file:

.. raw:: html

   <p><audio controls> <source src="media/received-mininet.wav" type="audio/wav">Your browser does not support the audio tag.</audio></p>

The quality of the received sound is degraded compared to the original, due to the loss and jitter of the link.

Sources: :download:`voipsender.ini <../voipsender.ini>`,
:download:`voipreceiver.ini <../voipreceiver.ini>`,
:download:`AppContainer.ned <../AppContainer.ned>`,
:download:`run_mininet.py <../run_mininet.sh>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/emulation/voip`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-latest --options=inet:full
    $ cd inet-workspace
    $ opp_env shell

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and opens an interactive shell. (The
``--options=inet:full`` argument is required to enable the Emulation feature in
opp_env.)

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

To experiment with the emulation examples, navigate to the
``inet/showcases/emulation/voip`` directory. From there, you can execute the
commands outlined in the previous sections.

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ in
the GitHub issue tracker for commenting on this showcase.
