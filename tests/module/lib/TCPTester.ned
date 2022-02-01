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
//
simple TCPScriptableTester
{
    parameters:
        @class(::inet::tcp::TCPScriptableTester);
        string script;
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


