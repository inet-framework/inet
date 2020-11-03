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

#ifndef __INET_HTTPSERVERDIRECT_H
#define __INET_HTTPSERVERDIRECT_H

#include "inet/applications/httptools/server/HttpServerBase.h"

namespace inet {

namespace httptools {

/**
 * Server module for direct message passing.
 *
 * This module implements a flexible Web server for direct message passing. It is part of the HttpTools project
 * and should be used in conjunction with a number of clients running the HttpBrowserDirect.
 * The module plugs into the HttpDirectHost module.
 *
 * @see HttpServerBase
 * @see HttpServer
 *
 * @author  Kristjan V. Jonsson
 */
class INET_API HttpServerDirect : public HttpServerBase
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace httptools

} // namespace inet

#endif

