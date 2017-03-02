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

#ifndef __INET_HTTPBROWSERDIRECT_H
#define __INET_HTTPBROWSERDIRECT_H

#include "inet/applications/httptools/browser/HttpBrowserBase.h"

namespace inet {

namespace httptools {

/**
 * A simulated browser module for OMNeT++ simulations.
 *
 * This module implements direct message passing between modules.
 *
 * @see HttpBrowserBase
 * @see HttpBrowser
 *
 * @author Kristjan V. Jonsson (kristjanvj@gmail.com)
 */
class INET_API HttpBrowserDirect : public HttpBrowserBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;
    int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void sendRequestToServer(BrowseEvent be) override;
    virtual void sendRequestToServer(Packet *request) override;
    virtual void sendRequestToRandomServer() override;
    virtual void sendRequestsToServer(std::string www, HttpRequestQueue queue) override;
};

} // namespace httptools

} // namespace inet

#endif // ifndef __INET_HTTPBROWSERDIRECT_H

