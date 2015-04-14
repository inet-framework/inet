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


#ifndef __INET_SERIALIZERUTIL_H_
#define __INET_SERIALIZERUTIL_H_

// Includes sockets.h for htonl, ntohl, etc. on Windows.
// This header needs to be included before <omnetpp.h> (i.e. practically before any other header)

#include <platdep/sockets.h>  //TODO should be <omnetpp/platdep/sockets.h> when OMNeT++ 4.x compatibility is no longer needed

#endif  // __INET_SERIALIZERUTIL_H_

