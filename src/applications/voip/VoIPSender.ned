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
package lte.apps.voip;

import inet.applications.IUDPApp;


simple VoIPSender like IUDPApp
{
    parameters:
        int localPort = default(-1);
        int destPort = default(3000);
        string destAddress;
        int PacketSize = default(40);
        double shape_talk = default(0.824);
        double scale_talk = default(1.423);
        double shape_sil = default(1.089);
        double scale_sil = default(0.899);
        bool is_talk = default(true);
        double sampling_time = default(0.02);
        double startTime @unit("s") = default(0);

        @display("i=block/source");
    gates:
        output udpOut;
        input udpIn;
}
