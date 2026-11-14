.. _ug:cha:scenario-scripting:

Scenario Scripting
==================

.. _ug:sec:scenario:overview:

Overview
--------

The INET Framework contains scripting support to help scenarios that
cannot be adequately described using static configuration.
Actions can be scheduled to be carried out at specified
simulation times, such as changing a parameter value, changing the
bit error rate of a connection, removing or adding connections, removing
or adding routes in a routing table, shutting down or crashing routers,
etc. The aim is usually to observe transient behavior caused by the
changes.

INET supports the following built-in actions:

-  Create or delete module

-  Create or delete connection

-  Set module or channel parameter

-  Initiate lifecycle operation (startup, shutdown, crash) on a network
   node or part of it

.. _ug:sec:scenario:scenariomanager:

ScenarioManager
---------------

The :ned:`ScenarioManager` module type is used for setting up and controlling
simulation experiments. In typical usage, it has only one instance in
the network:

.. code-block:: ned

   network Test {
       submodules:
           scenarioManager: ScenarioManager;
           ...
   }

:ned:`ScenarioManager` executes a script specified in XML. It has a few
built-in commands, while other commands are dispatched (in C++) to be
carried out by other simple modules.

An example script:

.. code-block:: xml

   <scenario>
       <set-param t="10" module="host[1].mobility" par="speed" value="5"/>
       <set-param t="20" module="host[1].mobility" par="speed" value="30"/>
       <at t="50">
           <set-param module="host[2].mobility" par="speed" value="10"/>
           <set-param module="host[3].mobility" par="speed" value="10"/>
           <connect src-module="host[2]" src-gate="ppp[0]"
                    dest-module="host[1]" dest-gate="ppp[0]"
                    channel-type="ned.DatarateChannel">
               <param name="datarate" value="10Mbps" />
               <param name="delay" value="0.1us" />
           </connect>
       </at>
       <at t="60">
           <disconnect src-module="host[2]" src-gate="ppp[0]" />
       </at>
   </scenario>

The above script probably does not need much explanation.

The built-in commands of :ned:`ScenarioManager` are: ``<connect>``,
``<disconnect>``, ``<create-module>``, ``<delete-module>``,
``<initiate>``, ``<shutdown>``, ``<startup>``, ``<crash>``,
``<set-param>``, ``<set-channel-attr>``, ``<at>``.

All commands have a ``t`` attribute which carries the simulation time
at which the command has to be carried out. Several commands can be grouped
to be carried out at the same simulation time using
``<at>``, and then only the ``<at>`` command needs to have a
``t`` attribute.

More information can be found in the :ned:`ScenarioManager`
documentation.

The script is usually placed in a separate file and specified like
this:

.. code-block:: ini

   *.scenarioManager.script = xmldoc("scenario.xml")

Short scripts can also be written inline:

.. code-block:: ini

   *.scenarioManager.script = xml("<x><shutdown t='2s' module='Router2'/></x>")

Changing a mesh peer's power mode
---------------------------------

An enabled mesh power policy accepts ``set-mesh-power-mode`` through
``ScenarioManager``. Point ``module`` at the policy submodule, and give
``peer`` as the established peer's individual MAC address. For example,
after assigning the peer interface this address in the configuration:

.. code-block:: ini

   *.peer.wlan[0].typename = "Ieee80211MeshInterface"
   *.relay.wlan[0].typename = "Ieee80211MeshInterface"
   *.peer.wlan[0].address = "0A-AA-00-00-00-02"
   *.peer.wlan[0].powerSave = true
   *.relay.wlan[0].powerSave = true
   *.scenarioManager.script = xmldoc("scenario.xml")

.. code-block:: xml

   <scenario>
       <at t="1s">
           <set-mesh-power-mode module="relay.wlan[0].powerSavePolicy"
                                peer="0A-AA-00-00-00-02" mode="light-sleep"/>
       </at>
   </scenario>

The mode is exactly ``active``, ``light-sleep``, or ``deep-sleep``. The
command requests a mode using the policy's existing peer transition: a
lower-activity mode commits when its indication is acknowledged; a request
for higher activity commits locally immediately. The interface's
``powerSave`` parameter defaults to false. Enable it on peers that must honor
the power-save exchanges, including peers that remain in ACTIVE mode. The
command reports an error if
the policy is disabled or down, the peer is not established, or either
attribute is missing or invalid. It does not retain requests for later peers.
