//
// Protocol Test Framework for INET -- Phase 6: inline MITM packet tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// A man-in-the-middle tap spliced onto an existing link (e.g. between two hosts'
// Ethernet gates). It forwards every frame transparently in both directions, except
// frames selected by `matchExpression` (an INET PacketFilter expression evaluated over
// the dissected frame) and `occurrence`, to which it applies `action`:
//
//   pass   -- forward unchanged (the default; also applied to non-selected frames)
//   drop   -- discard the frame (fault injection: e.g. force a retransmission)
//   delay  -- forward after `delayTime`
//   mutate -- apply a mutation, then forward (the mutator is a C++ lambda, so this action
//             is only reachable via a test program's intercept(...).mutate(...), not params)
//
// Configurable two ways: from these parameters (self-contained), or programmatically by
// the ProtocolTester from a test program's intercept(...) clause. The ProtocolTester
// observes the resulting trace as usual.
//
package inet.protocoltest;

simple PacketTap
{
    parameters:
        string matchExpression = default(""); // PacketFilter expression selecting candidate frames ("" = all)
        int minPacketBytes = default(0);      // additionally require the (inner) frame to be at least this many bytes
        string action = default("pass");      // action on a selected frame: "drop" | "delay" | "pass" (mutate: via a test program)
        int occurrence = default(0);          // act on the Nth selected frame (1-based); 0 = every selected frame
        double delayTime @unit(s) = default(0s); // hold time for action == "delay"
        @networkNode(); // let the Ipv4 configurator traverse the tap (join both sides into one link)
        @display("i=block/tunnel");
        @class(PacketTap);
    gates:
        inout a;   // one side of the spliced link
        inout b;   // the other side
}
