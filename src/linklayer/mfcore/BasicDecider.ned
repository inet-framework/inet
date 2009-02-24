//***************************************************************************
// * file:        BasicDecider.ned
// *
// * author:      Marc Loebbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/


package inet.linklayer.mfcore;

//
// Module to decide whether a frame is received correctly or is
// lost due to bit errors, interference...
//
// The decider module only handles messages from lower layers. All
// messages from upper layers are directly passed to the snrEval layer
// and cannot be processed in the decider module
//
// This is the basic decider module which does not really decide
// anything. It only provides the basic functionality which all
// decider modules should have, namely message de- &encapsulation
// (For further information about the functionality of the physical
// layer modules and the formats used for communication in between
// them have a look at the manual
//
// Every own decider module class should be derived from this class
// and only the handle*Msg functions may be redefined for your own
// needs. The other functions should usually NOT be changed.
//
// All decider modules should assume bits as a unit for the length
// fields.
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @author Marc Löbbers, Daniel Willkomm
//
simple BasicDecider
{
    parameters:
        bool coreDebug = default(false); // debug switch for the core framework
        @display("i=block/process_s");
    gates:
        output uppergateOut; // to MAC layer
        input lowergateIn; // from snrEval sub-module
}
