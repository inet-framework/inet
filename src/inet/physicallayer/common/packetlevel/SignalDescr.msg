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
#include "inet/physicallayer/common/packetlevel/Signal.h"
}}

namespace inet::physicallayer;

cplusplus {{
typedef const ITransmission ConstITransmission;
typedef const IArrival ConstIArrival;
typedef const IListening ConstIListening;
typedef const IReception ConstIReception;
}}

class ConstITransmission { @existingClass(true); @descriptor(false); }
class ConstIArrival { @existingClass(true); @descriptor(false); }
class ConstIListening { @existingClass(true); @descriptor(false); }
class ConstIReception { @existingClass(true); @descriptor(false); }

packet Signal
{
    @existingClass(true);
    @descriptor(readonly);
    ConstITransmission *transmission;
    ConstIArrival *arrival;
    ConstIListening *listening;
    ConstIReception *reception;
}

