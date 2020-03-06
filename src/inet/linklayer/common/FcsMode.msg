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

enum FcsMode
{
    FCS_MODE_UNDEFINED = -1; // mode is undefined, FCS is not yet set, not serializable
    FCS_DISABLED = 0; // FCS is disabled (not set), serializable
    FCS_DECLARED_CORRECT = 1; // FCS is correctly set without the actual value, not serializable
    FCS_DECLARED_INCORRECT = 2; // the FCS is incorrectly set without the actual value, not serializable
    FCS_COMPUTED = 3; // FCS is set to a value that is potentially incorrect, serializable
}

cplusplus {{
INET_API FcsMode parseFcsMode(const char *fcsModeString);
}}
