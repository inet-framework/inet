//
// Copyright (C) 2011 Adriano (University of Pisa)
// Copyright (C) 2012 Opensim Ltd.
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

//
// Represents a voice packet used by SimpleVoIPSender / SimpleVoIPReceiver 
//
packet SimpleVoIPPacket
{
    unsigned int talkspurtID;          // identifies the talkspurt
    unsigned int talkspurtNumPackets;  // number of all packets of current talkspurt
    unsigned int packetID;             // 0..talkspurtNumPackets-1
    simtime_t voipTimestamp;            // "mouth" timestamp of begin of voice in this packet
    simtime_t voiceDuration;            // duration of voice in this packet
}

