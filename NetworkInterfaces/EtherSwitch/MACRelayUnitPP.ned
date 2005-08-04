//
// Copyright (C) 2003 CTIE, Monash University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//



//
// A MACRelayUnit implementation which models one CPU assigned to each
// incoming port, working with shared memory but separate queues.
//
// It also models fixed delay for precessing each frame.
// Finite memory is taken into account by dropping frames if
// total number of bits enqueued exceed a given limit.
//
// A simple scheme for sending PAUSE frames is built in (although
// users will probably change it). When the buffer level goes
// above a high watermark, PAUSE frames are sent on all ports.
// The watermark and the pause time is configurable; use zero
// values to disable the PAUSE feature.
//
simple MACRelayUnitPP
    parameters:
        addressTableFile : string,  // see MACRelayUnit
        addressTableSize : numeric, // see MACRelayUnit
        agingTime : numeric,        // see MACRelayUnit
        processingTime : numeric,   // processing time of one frame
        bufferSize : numeric,       // memory (bytes)
        highWatermark : numeric,    // buffer usage threshold to send PAUSE frame (bytes)
        pauseUnits : numeric,       // time to put in PAUSE frames (in units of 512 bit times)
        writeScalars: bool;         // enable/disable recording statistics in omnetpp.sca

    gates:
        in:  lowerLayerIn[];       // see MACRelayUnit
        out: lowerLayerOut[];      // see MACRelayUnit
endsimple

