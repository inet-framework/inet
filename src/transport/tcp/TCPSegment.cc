//
// Copyright (C) 2004 Andras Varga
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


#include "TCPSegment.h"

Register_Class(TCPSegment);


void TCPSegment::truncateData(unsigned int truncleft, unsigned int truncright)
{
    sequenceNo_var += truncleft;
    payloadLength_var -= truncleft + truncright;
}

void TCPSegment::truncateSegment(uint32 firstSeqNo, uint32 endSeqNo)
{
    unsigned int truncleft = 0;
    unsigned int truncright = 0;

    if (seqLess(sequenceNo_var, firstSeqNo))
    {
        truncleft = firstSeqNo - sequenceNo_var;
    }

    if (seqGreater(sequenceNo_var+payloadLength_var, endSeqNo))
    {
        truncright = sequenceNo_var + payloadLength_var - endSeqNo;
    }
    truncateData(truncleft, truncright);
}
