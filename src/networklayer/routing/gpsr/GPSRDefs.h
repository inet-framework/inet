//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_GPSRDEFS_H_
#define __INET_GPSRDEFS_H_

#define GPSR_UDP_PORT 269

enum GPSRForwardingMode {
    GPSR_GREEDY_ROUTING,
    GPSR_PERIMETER_ROUTING
};

enum GPSRPlanarizationMode {
    GPSR_GG_PLANARIZATION,
    GPSR_RNG_PLANARIZATION
};

#endif
