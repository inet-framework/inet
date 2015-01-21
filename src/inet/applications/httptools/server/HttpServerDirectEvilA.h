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

#ifndef __INET_HTTPSERVERDIRECTEVILA_H
#define __INET_HTTPSERVERDIRECTEVILA_H

#include <string>
#include "inet/applications/httptools/server/HttpServerDirect.h"

namespace inet {

namespace httptools {

/**
 * An evil attacker server demonstration - type A.
 *
 * Demonstrates subclassing the server to create a custom site. This site is an attacker -- a puppetmaster --
 * which serves HTML pages containing attack code. In this case, we are simulating JavaScript attack code which prompts
 * the unsuspecting browser to issue a number of requests for non-existing resources to the victim site.
 * Delays are specified to simulate hiding the attack from the browser user by use of JavaScript timeouts or similar mechanisms.
 * The generateBody virtual function is redefined to create a page containing the attack code.
 *
 * @see HttpServerDirect
 *
 * @author  Kristjan V. Jonsson
 */
class INET_API HttpServerDirectEvilA : public HttpServerDirect
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

#endif // ifndef __INET_HTTPSERVERDIRECTEVILA_H

