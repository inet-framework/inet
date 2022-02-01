//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
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

#endif

