//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.applications.httptools.common;

//
// Interface for HTTP server and browser models that communicate directly,
// without lower layer protocols. This module is intended for use with the
// ~HttpDirectHost module and plugs in as an app.
//
// @see ~HttpServer, ~HttpDirectHost
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

