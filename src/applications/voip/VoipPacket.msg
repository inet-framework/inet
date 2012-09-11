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

//
// TODO generated message class
//
// FIXME: this name is almost the same as in voiptool (except case), so it is more than confusing
packet VoipPacket
{
    unsigned int talkID;
    // FIXME: the naming convention uses numFrames. anyway this is probably rather numSamples, what would number of frames mean?
	unsigned int nframes;
    unsigned int frameID;
    // FIXME: this name is pretty meaningless. does it contain the sending time or what? be more specific
	simtime_t voipTimestamp;
    // FIXME: is this a duration? if so, then call it duration
    simtime_t timeLength;

    // next field used by receiver:
    // FIXME: this name is also quite meaningless. is it the time when the packet was played at the receiver? why do we put it in the packet then? it looks like as if it would have been sent over the wire
	simtime_t playoutTime;
}
