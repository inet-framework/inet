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

package inet.applications.httptools.server;

import inet.applications.ITCPApp;


//
// A Web server module for OMNeT++ simulations. This module is intended for use with the
// INET StandardHost module and plugs in as a tcpApp. The INET TCP/IP stack modeling is
// used for transport. See the INET documentation for more details.
//
// The purpose of the HTTPServer component is to simulate a Web server operating on a single host.
// It operates in conjunction with the browser component to provide a realistic simulation of Web
// usage patterns.
//
// The activity of the server is characterized by the requests it receives. It responds to requests
// by serving documents or resources.
//
// A server receives a HTTP page request from a browser. If the browsers message is marked as bad,
// the server will respond with a 404:Not found message. Otherwise, the server will respond with
// a 200:OK HTML response. The body of the response is a list of resource references, as further
// described in HTTPMessage. The receiving browser is expected to issue requests for those
// resources, as discussed in HTTPBrowser. A server in responds to each resource request with a
// message consistent with the object requested. The size of the HTML page or resource sent is
// solely determined by the server receiving the corresponding request.
//
// The component has two modes of operation:
//
//  - Random document generation mode, in which the server responds to requests for HTML documents
//     by generating replies according to statistical distributions.
//  - Scripted mode, in which the pages and resources served are predetermined.
//
// <h1>Random document generation mode</h1>
//
// The goal for this mode of operation is for the server component to exhibit traffic patterns,
// which are statistically similar to ones derived from measurements of real systems. The component
// is initialized with a number of parameters, shown in table below, which define its behavior.
// The parameters are defined in a XML file and can be determined at run time by various random
// distributions, as was discussed for the browser component.
//
// Parameters:
// - pageSize:               The size of the generated HTML document
// - numResources:           The number of referenced resources per HTML page
// - textImageResourceRatio: The ratio of images to text resources on HTML pages
// - imageResourceSize:      The size of image resources
// - textResourceSize:       The size of text resources, e.g. CSS documents
// - replyDelay:             The delay in processing of each received HTML request before sending reply
//
// In this mode, the server responds to a HTML page request by assembling a response containing
// a random number of resources. The number of resource references and their type is determined
// by random distributions, according to the initialization parameters. Similarly, requests for
// resources are answered by messages of a size consistent with the size distributions for the
// object in question.
//
// Every server in a simulation can be configured with different parameters, although this would
// quickly become unwieldy in a large simulation. The flexibility to define groups or categories
// of servers will however prove useful in many cases.
//
// <h1>Scripted mode</h1>
//
// Repeatable response by a subset of servers in a simulation can be useful in a variety of
// situations, e.g. for testing purposes. This is provided by the scripted server mode. A Web
// site is defined in some detail using a site definition file. The definition file is specified
// by setting a scriptFile initialization parameter. The file format is:
//
// <pre>[HTML]
// [path/]{name}[.extension];{definition file}
// [RESOURCES]
// [path/]{name.extension};{size}</pre>
//
// The HTML section lists the hosted HTML pages and their associated definition files, one for
// each hosted HTML page. The page definition file is normally named {U\R\L}.def (the resource
// URL string, with~ replaced by), although any name can certainly be used. The format of the
// page definition file is the same as used for the generated HTML message body, a simple list
// of the referenced resources. A optional entry with URL as default can be used to specify a
// default page for the site, e.g. an error page.
//
// The RESOURCES section lists all resources available for the site, their type (image or text)
// and size in bytes.
//
// The server responds with a message containing the specified page or resource if the request
// can be satisfied (a 200:OK reply). Otherwise, a 404:Not found reply is issued.
//
// @see HttpServerDirect
// @see DirectHost
//
// @author  Kristjan V. Jonsson
//
simple HttpServer like ITCPApp
{
    parameters:
        string hostName = default("");                  // The domain name of the server
        int port = default(80);                         // The listening port number
        int httpProtocol = default(11);                 // The http protocol: 10 for http/1.0, 11 for http/1.1. Not used at the present time.
        string logFile = default("");                   // Name of server log file. Events are appended, allowing sharing of file for multiple servers.
        string siteDefinition = default("");            // The site script file. Blank to disable.
        double activationTime @unit("s") = default(0s); // The initial activation delay. Zero to disable.
        xml config;                                     // The XML configuration file for random sites
    gates:
        input tcpIn;
        output tcpOut;
}

