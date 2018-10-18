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

import inet.common.INETDefs;
import inet.linklayer.common.MacAddress;

namespace inet;

//
// Message kind values used with in communication between L3 and IEEE 802 L2
//
enum Ieee802MessageKind
{
    IEEE802CTRL_DATA = 2003;  // data to/from higher layer
    IEEE802CTRL_SENDPAUSE = 2006;  // higher layer wants MAC to send PAUSE frame
}

// used with IEEE802CTRL_SENDPAUSE
class Ieee802PauseCommand extends cObject
{
    MacAddress destinationAddress;
    int pauseUnits;
}
