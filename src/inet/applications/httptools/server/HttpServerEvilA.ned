//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

package inet.applications.httptools.server;

//
// Demonstrates subclassing the server to create a custom site. This site is an attacker -- a puppetmaster --
// which serves HTML pages containing attack code. In this case, we are simulating JavaScript attack code which prompts
// the unsuspecting browser to request a number of images from a victim site. Delays are specified to simulate hiding
// the attack from the browser user by use of JavaScript timeouts or similar mechanisms.
// The generateBody virtual function is redefined to create a page containing the attack code.
//
// @author  Kristjan V. Jonsson
//
simple HttpServerEvilA extends HttpServer
{
    parameters:
        @class(HttpServerEvilA);
        int minBadRequests;     // Specifies the lower bound of bad requests to be sent to the victim by the browser.
        int maxBadRequests;     // Specifies the upper bound of bad requests to be sent to the victim by the browser.
}

