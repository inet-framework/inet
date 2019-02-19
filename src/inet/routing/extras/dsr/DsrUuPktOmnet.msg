//
// Copyright (C) 2011 Zoltan Bojthe
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

import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

cplusplus {{
#include "DsrUuPktOmnet.h"
}}

namespace inet::inetmanet;

struct dsr_opt 
{
    @existingClass(true);
    @descriptor(readonly);
    uint8_t type;
    uint8_t length;
}

struct EtxCost
{
    @existingClass(true);
    @descriptor(readonly);
    L3Address address;
    double cost;
};

struct EtxList
{
    @existingClass(true);
    @descriptor(readonly);
    L3Address address;
    double delivery;// the simulation suppose that the code use a u_int32_t
};

cplusplus {{
     typedef  std::vector <dsr_opt*>  VectorOptions;
     typedef  std::vector <EtxCost>  VectorEtx;
      }} // C++ typedef
      
class VectorOptions { @existingClass; }
class VectorEtx { @existingClass; }

struct dsr_opt_hdr
{
    @existingClass(true);
    @descriptor(readonly);
    uint8_t nh;
    uint8_t res;
    uint8_t f;
    uint16_t p_len;
    VectorOptions option;
}

cplusplus {{
     typedef  std::vector <dsr_opt_hdr>  VectorOptionHeader;
      }} // C++ typedef

class VectorOptionHeader { @existingClass; }

class DSRPkt extends FieldsChunk
{
    @existingClass(true);
    @descriptor(readonly);
    VectorOptionHeader dsrOptions;
    L3Address previous @getter(getPrevAddress) @setter(setPrevAddress);
    L3Address next @getter(getNextAddress) @setter(setNextAddress);
    VectorEtx costVector;
    int dsr_ttl @getter(getTimeToLive) @setter(setTimeToLive);
    int encapProtocol = -1;
};

//cplusplus {{
//     typedef  EtxList *  EtxListPtr;
//      }} // C++ typedef
//
//class EtxListPtr { @existingClass; }

class DSRPktExt extends DSRPkt
{
    @existingClass(true);
    @descriptor(readonly);
    EtxList *extension  = nullptr;
    int size;
}
