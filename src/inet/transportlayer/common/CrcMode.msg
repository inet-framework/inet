//
// Copyright (C) OpenSim Ltd.
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

namespace inet;

enum CrcMode
{
    CRC_MODE_UNDEFINED = -1; // mode is undefined, CRC is not yet set, not serializable
    CRC_DISABLED = 0; // CRC is disabled (not set), serializable
    CRC_DECLARED_CORRECT = 1; // CRC is correctly set without the actual value, not serializable
    CRC_DECLARED_INCORRECT = 2; // the CRC is incorrectly set without the actual value, not serializable
    CRC_COMPUTED = 3; // CRC is set to a value that is potentially incorrect, serializable
}


cplusplus {{
INET_API CrcMode parseCrcMode(const char *crcModeString, bool allowDisable);
}}
