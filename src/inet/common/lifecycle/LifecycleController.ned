//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
//

package inet.common.lifecycle;

//
// Manages operations like shutdown/restart, suspend/resume, crash/recover
// and similar operations for nodes (routers, hosts, etc), interfaces, and
// protocols.
//
// This module usually has only one instance in a network, and has no
// parameters -- operations can be initiated programmatically (from C++),
// or via ~ScenarioManager scripts. An example script:
//
// <pre>
// <scenario>
//   <at t="2s">
//     <tell module="lifecycleController" target="Router2" operation="NodeShutdownOperation"/>
//   </at>
//   <at t="10s">
//     <tell module="lifecycleController" target="Router2" operation="NodeStartOperation"/>
//   </at>
// </scenario>
// </pre>
//
// The <tt>module</tt> attribute should point to the LifecycleController
// module instance.
//
// The <tt>target</tt> attribute should point to the module (host, router,
// network interface, protocol, etc) to be operated on.
//
// The <tt>operation</tt> attribute contains the operation to perform. Some
// known operations names are: NodeShutdownOperation, NodeCrashOperation,
// NodeStartOperation. To see all available operations,
// find the names of C++ classes subclassed from LifecycleOperation.
//
// The details of the mechanism can be found in the C++ documentation of this
// module's class.
//
// @see ScenarioManager
//
simple LifecycleController
{
    @display("i=block/cogwheel_s");
}
