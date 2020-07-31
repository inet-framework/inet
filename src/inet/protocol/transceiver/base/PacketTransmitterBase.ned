//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.protocol.transceiver.base;

import inet.queueing.base.PacketProcessorBase;

simple PacketTransmitterBase extends PacketProcessorBase
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional 
        volatile double datarate @unit(bps);
        @class(PacketTransmitterBase);
        @display("i=block/wtx");
        @signal[transmissionStarted](type=inet::physicallayer::Signal);
        @signal[transmissionEnded](type=inet::physicallayer::Signal);
    gates:
        input in;
        output out;
}
