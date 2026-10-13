//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.moduletest.lib;

//
// In addition to providing tcpdump-like functionality, it can delete,
// delay or duplicate \TCP segments, and inject new segments. Such segment
// manipulation can be controlled via a script.
//
// Script format:
//
// <i><segment><operation><args>; <segment><operation><args>; ...</i>
//
// e.g.:
//
// <tt>A2 delete; B3 delete; A3 delay 0.2; A4 copy 1.0,1.2; ...</tt>
//
// Where:
// - <i><segment></i>: A10 means 10th segment arriving from A
// - <i><operation></i> can be <tt>delete</tt>, <tt>delay</tt> or <tt>copy</tt>:
//    - <tt>delete</tt>: removes (doesn't copy) segment
//    - <tt>delay</tt> <i><delay></i>: forwards segment after a delay
//    - <tt>copy</tt> <i><delay1>,<delay2>,<delay3>,...</i>:
//      forwards copies of the segment after the given delays
//      (<tt>copy 0.5</tt> is the same as <tt>delay 0.5</tt>)
//    - <tt>ce</tt>: forwards the segment marked CE
//    - <tt>ect1</tt>: forwards the segment marked ECT(1), as from a sender that
//      uses ECT(1)
//    - <tt>ect0</tt>: forwards the segment marked ECT(0), as from a sender that
//      marks also a retransmission
//    - <tt>tsecr</tt> <i><value></i>: forwards the segment with this TSecr in its
//      timestamp option
//    - <tt>data</tt> <i><bytes></i> [<tt>fin</tt>]: forwards the segment with this
//      number of bytes added after its payload, and with the FIN bit set if
//      <tt>fin</tt> follows; <tt>fin</tt> alone sets only the FIN bit. It makes
//      a peer that sends data or a FIN where INET does not (on a SYN-ACK).
//    - <tt>strip</tt> <i><kind></i>: forwards the segment without its options of
//      this kind; it makes a peer that does not answer an option
//    - <tt>ace</tt> <i><value></i>: forwards the segment with its AE, CWR and ECE
//      bits set to the bits of the value (AE = 4, CWR = 2, ECE = 1); it makes a peer
//      or a path that changes them
//    - <tt>zeroaccecn</tt>: forwards the segment with the fields of its AccECN option
//      set to 0; it makes a path that zeroes them
//
// The parameter pathMtu makes a path that drops each packet larger than this
// MTU without an ICMP error, a black hole for RFC 4821: the length of a packet
// is the TCP segment and 20 bytes of IPv4 header. A packet that the script
// handles is not dropped.
//
simple TCPScriptableTester
{
    parameters:
        @class(::inet::tcp::TCPScriptableTester);
        string script;
        int pathMtu = default(-1); // drop each packet larger than this MTU, without an ICMP error; -1: no limit
        @display("i=device/router");
    gates:
        input in1;
        input in2;
        output out1;
        output out2;
}


//
// Complements TCPScriptableTester: instead of working from a script, it
// executes a "random script".
//
simple TCPRandomTester
{
    parameters:
        @class(::inet::tcp::TCPRandomTester);
        double pdelete; // probability of deleting a packet
        double pdelay;  // probability of delaying a packet
        double pcopy;   // probability of creating several copies of the packet
        volatile int numCopies;     // related to pcopy: number of copies
        volatile double delayValue;    // related to pdelay and pcopy: delay introduced
        @display("i=device/router");
    gates:
        input in1;
        input in2;
        output out1;
        output out2;
}


