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

cplusplus {{
#include "inet/common/packet/FieldsChunk.h"
#include "inet/common/packet/SequenceChunk.h"
}}

namespace inet;

class FieldsChunk extends cObject;
class SequenceChunk extends cObject;

class CompoundHeader extends SequenceChunk
{
    @customize();
}

class TlvHeader extends FieldsChunk
{
    int8_t type = -1;
    int64_t chunkLength = -1;
}

class TlvHeaderBool extends TlvHeader
{
    type = 1;
    chunkLength = 3;
    bool boolValue = false;
}

class TlvHeaderInt extends TlvHeader
{
    type = 2;
    chunkLength = 4;
    int16_t int16Value = -1;
}
