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


cplusplus {{
#include "INETDefs.h"
}}


//
// Message class for generic request-reply style applications.
// The client sends a GenericAppMsg which contains the number of
// bytes the client wants the server to send back as reply.
//
// This way intelligence (behaviour specific to the modelled application,
// e.g. HTTP, SMB, database protocol) needs only to be present in
// the client, and the server model can be kept simple and dumb.
//
// @see TCPGenericSrvApp, TCPGenericCliAppBase (C++ only)
//
message GenericAppMsg
{
    properties:
        omitGetVerb = true;
    fields:
        int expectedReplyLength;  // in bytes
        double replyDelay;  // reply after this many seconds
        bool close;  // with TCP: if true, server should close the
                     // connection after sending the reply
};



