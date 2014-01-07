//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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



package inet.linklayer;

//
// Prototype for modules providing Ethernet switch functionality.
// These modules handle the mapping between ports and MAC addresses,
// and forward frames (~EtherFrame) to appropriate ports.
//
// Functions:
// - relays frames based on their dest MAC addresses (dest-->port mapping)
// - models finite buffer size (drops) and finite processing power (latency, queueing)
//
// Address caching is modeled by a fixed-size lookup table.
// When the table is full, the oldest address is kicked out of the cache.
//
// Entries are also deleted if their age exceeds a certain limit.
//
// If needed, address tables can be pre-loaded from text files at the beginning
// of the simulation; this controlled by the addressTableFile module parameter.
//
// This module is not a concrete implementation, it just defines gates and
// parameters a ~IMACRelayUnit should have. Concrete inplementations add
// capacity and performance aspects to the model (number of frames processed
// per second, amount of memory available in the switch, etc.)
// C++ implementations can subclass from the class <tt>MACRelayUnitBase</tt>.
//
// Known implementations are ~MACRelayUnit and ~Ieee8021dRelay.
//
moduleinterface IMACRelayUnit
{
    parameters:
        @display("i=block/switch");
    gates:
        input ifIn[] @labels(EtherFrame);
        output ifOut[] @labels(EtherFrame);
        input stpIn @loose;
        output stpOut @loose;
}

simple MACRelayUnit_None like IMACRelayUnit
{
    parameters:
        @display("i=old/x_noentry");
        @class("UnimplementedModule");
    gates:
        input ifIn[] @labels(EtherFrame);
        output ifOut[] @labels(EtherFrame);
        input stpIn @loose;
        output stpOut @loose;
}

