//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common.scenario;

//
// ~ScenarioManager is for setting up and controlling simulation experiments.
// You can schedule certain events to take place at specified times,
// like changing a parameter value, changing the bit error rate of
// a connection, removing or adding connections, removing or
// adding routes in a routing table, etc, so that you can observe the
// transient behaviour.
//
// ~ScenarioManager executes a script specified in XML. It has a few
// built-in commands, while other commands are dispatched to be carried out
// by given simple modules. (The C++ class of these simple modules' needs
// to implement the ~IScriptable interface, and the processCommand() method
// must be redefined accordingly).
//
// An example script:
//
// <pre>
// <scenario>
//     <set-param t="10" module="host[1].mobility" par="speed" value="5"/>
//     <set-param t="20" module="host[1].mobility" par="speed" expr="20+10"/>
//     <set-param t="30" module="host[1].mobility" par="displayStringTextFormat" value='p: %p\nv: %v'/>
//     <set-param t="30" module="host[1].mobility" par="displayStringTextFormat" expr='"p: %p\nv: %v"'/>
//     <at t="50">
//         <set-param module="host[2].mobility" par="speed" value="10"/>
//         <set-param module="host[3].mobility" par="speed" value="10"/>
//         <connect src-module="host[2]" src-gate="ppp[0]"
//                  dest-module="host[1]" dest-gate="ppp[0]"
//                  channel-type="ned.DatarateChannel">
//             <param name="datarate" value="10Mbps" />
//             <param name="delay" value="0.1us" />
//         </connect>
//     </at>
//     <at t="60">
//         <disconnect src-module="host[2]" src-gate="ppp[0]" />
//     </at>
//     <at t="2s">
//         <initiate module="Router2" operation="shutdown"/>
//         <shutdown module="Router2"/>
//         <start module="Router2"/>
//         <crash module="Router2"/>
//     </at>
// </scenario>
// </pre>
//
// Built-in commands: <at>, <set-param>, <set-channel-param>,
// <create-module>, <delete-module>, <connect>, <disconnect>,
// <initiate>, <startup>, <shutdown>, <crash>
//
// All commands have a t attribute which carries the simulation time
// at which the command has to be carried out. You can group several commands
// to be carried out at the same simulation time using <at>, and
// then only the <at> command is needed to have a t attribute.
//
// Supported attributes:
// - <set-param>: module, par, value|expr.
//       module: path to module (required)
//       par: name of settable parameter of module (required)
//       value: new value for parameter
//       expr: expression for calculate new value of parameter
//       Note: value and expr are exclusive, required one
// - <set-channel-param>: src-module, src-gate|dest-module, par, value|expr.
//       src-module: path to source module (required)
//       src-gate: name of source gate
//       dest-module: path to destination module
//       par: name of settable parameter of module (required)
//       value: new value for parameter
//       expr: expression for calculate new value of parameter
//       Note: src-gate and dest-module are exclusive, required one;
//             value and expr are exclusive, required one;
//             if dest-module is present, there must be exactly one connection
//             between src-module and dest-module (which may be bidirectional);
//             if src-gate references an inout gate, the parameter will be set
//             on the channel of both directions.
// - <connect>: src-module, src-gate, dest-module, dest-gate, channel-type
//       src-module: path to source module (required)
//       src-gate: name of source gate (required)
//       dest-module: path to destination module (required)
//       dest-gate: name of destination gate (required)
//       channel-type: full qualified ned type of connection channel (required), e.g. "ned.DatarateChannel"
//     - <param>: name, value|expr
//       name: name of settable parameter of channel (required)
//       value: new value for parameter
//       expr: expression for calculate new value of parameter
//       Note: value and expr are exclusive, required one
// - <disconnect>: src-module, src-gate|dest-module
//       src-module: path to source module (required)
//       src-gate: name of source gate
//       dest-module: path to destination module
//       Note: src-gate and dest-module are exclusive, required one;
//       if dest-module is present, there must be exactly one connection
//       between src-module and dest-module (which may be bidirectional);
//       if src-gate references an inout gate, both directions will be disconnected.
// - <create-module>: parent, submodule, type, vector
//       parent: parent module path (required)
//       submodule: name of created module (required)
//       type: NED type of created module (required), e.g. "inet.applications.pingapp.PingApp"
//       vector: boolean, if true, the module created as vector element with next index,
//               if false, the module created as a single module,
//               if missing, detects existing vector by submodule name
// - <delete-module>: module
//       module: path to module (required)
//
simple ScenarioManager
{
    parameters:
        xml script = default(xml("<script></script>"));
        @display("i=block/control");
        @labels(node,mpls-node);
}

