//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#ifndef __INET_HTTPSERVERDIRECTEVILB_H
#define __INET_HTTPSERVERDIRECTEVILB_H

#include <string>

#include "inet/applications/httptools/server/HttpServerDirect.h"

namespace inet {

namespace httptools {

/**
 * An evil attacker server demonstration - type B.
 *
 * Demonstrates subclassing the server to create a custom site. This site is an attacker -- a puppetmaster --
 * which serves HTML pages containing attack code. In this case, we are simulating JavaScript attack code which prompts
 * the unsuspecting browser to issue a number of requests for non-existing resources (random URLs) to the victim site.
 * Delays are specified to simulate hiding the attack from the browser user by use of JavaScript timeouts or similar mechanisms.
 * The generateBody virtual function is redefined to create a page containing the attack code.
 *
 * @see HttpServerDirect
 *
 * @author  Kristjan V. Jonsson
 */
class INET_API HttpServerDirectEvilB : public HttpServerDirect
{
  private:
    int badLow = 0;
    int badHigh = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual std::string generateBody() override;
};

} // namespace httptools

} // namespace inet

#endif

