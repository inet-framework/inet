//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.protocol.transceiver;

import inet.protocol.transceiver.base.PacketReceiverBase;
import inet.protocol.transceiver.contract.IPacketReceiver;

//
// This module receives signals from the transmission medium as a stream and
// also sends packets to the upper layer as a stream. The stream start and stream
// end are sent seperately allowing the preemption of signals.
//
simple StreamThroughReceiver extends PacketReceiverBase like IPacketReceiver
{
    parameters:
        double datarate @unit(bps);
        @class(StreamThroughReceiver);
        @signal[receptionStarted](type=inet::physicallayer::Signal);
        @signal[receptionEnded](type=inet::physicallayer::Signal);
}

