//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
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

package inet.applications.httptools.common;


//
// Interface for HTTP server and browser models that communicate directly,
// without lower layer protocols. This module is intended for use with the
// DirectHost module and plugs in as a tcpApp.
//
// @see HttpServerBase
// @see HttpServer
// @see DirectHost
//
// @author  Kristjan V. Jonsson
//
moduleinterface IHttpDirectApp
{
    parameters:
        double linkSpeed @unit(bps);  // Used to model transmission delays.
    gates:
        input httpIn @directIn;
}

