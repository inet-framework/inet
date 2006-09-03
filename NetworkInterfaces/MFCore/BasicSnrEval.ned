//****************************************************************************
// * file:        BasicSnrEval.h
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
// ***************************************************************************/


//
// The basic class for all snrEval modules
//
// The BasicSnrEval module provides functionality like en- and
// decapsulation of messages. If you use the standard message formats
// everythng should work fine. Before a packet is sent some
// information, e.g. transmission power, can be written to the
// AirFrame header. If you write your own snrEval, just subclass and
// redefine the handleUpperMsg function (see description of the
// function). After receiving a message it can be processed in
// handleLowerMsgStart. Then it is buffered for the time the
// transmission would last in reality, and then can be handled
// again. Again you can redefine the 1. handleLowerMsgStart and
// 2. handleLowerMsgEnd for your own needs (see description). So, the
// call of these functions represent the following events: 1. received
// a message (i.e. transmission startet) 2. message will be handed on
// to the upper layer (i.e. transmission time is over)
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @see SnrEval
// @author Marc Loebbers
//
simple BasicSnrEval
    parameters:
        coreDebug: numeric const, // debug switch for the core framework
        transmitterPower: numeric, // power used for transmission of messages (in mW)
        bitrate: numeric, // bitrate (in bits/sec)
        headerLength: numeric const; // header length of the AirFrame (in bits)
    gates:
        in: uppergateIn; // from the MAC layer
        out: uppergateOut; // to the decider sub-module
        in: radioIn; // to receive AirFrames
endsimple

