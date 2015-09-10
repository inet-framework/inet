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

package inet.physicallayer.communicationcache;

import inet.physicallayer.contract.packetlevel.ICommunicationCache;

//
// This communication cache model allows validating other implementations. It
// can be used to provide a reference for communication logs, simulation
// fingerprints, and statistical results that can be compared with results of
// other communication cache implementations.
//
module ReferenceCommunicationCache like ICommunicationCache
{
    parameters:
        @display("i=block/table2");
        @class(ReferenceCommunicationCache);
}
