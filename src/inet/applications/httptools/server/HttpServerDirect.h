//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
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

