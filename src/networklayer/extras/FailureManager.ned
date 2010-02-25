//
// Copyright (C) 2005 Vojtech Janota
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.networklayer.extras;

//
// Module for managing node failure and recovery in the network. If used,
// it should have exactly one instance in the whole network.
//
// FailureManager does not send or receive messages, and has no parameters
// either. Instead, it is invoked from ScenarioManager scripts, for example:
//
// <pre><nohtml>
// <scenario>
//   <at t="2">
//     <shutdown module="failureManager" target="LSR2"/>
//   </at>
//   <at t="10">
//     <startup module="failureManager" target="LSR2"/>
//   </at>
// </scenario>
// </nohtml></pre>
//
// It is currently only used with MPLS simulations, and supports the following
// commands:
//
// - <code>shutdown</code>: replaces a router module named by the <tt>target</tt> attribute
//   with another module that represents a failed router. It currently replaces
//   RSVP_LSR with RSVP_FAILED, LDP_LSR with LDP_FAILED, and QuaggaRouter with
//   FailedRouter.
// - <code>startup</code>: the inverse of <tt>shutdown</tt>: replaces a <i>failed</i>
//   router model named by the <tt>target</tt> attribute with an operational host/router
//   module type.
//
// The operation of FailureManager is likely to get refined and generalized in next
// versions.
//
simple FailureManager
{
    parameters:
        @labels(node,mpls-node);
        @display("i=block/control_s");
}


