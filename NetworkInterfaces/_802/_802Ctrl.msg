//
// Copyright (C) 2003 CTIE, Monash University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


cplusplus {{
#include "MACAddress.h"
}};

class noncobject MACAddress;

//
// Control structure for communication between LLC and higher layers
//
class _802Ctrl
{
    fields:
        MACAddress src;  // used with DATA
        MACAddress dest; // used with DATA
        int ssap;        // used with DATA
        int dsap;        // used with DATA
        int pauseUnits;  // used with PAUSE
};
