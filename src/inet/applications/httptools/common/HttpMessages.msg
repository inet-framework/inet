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
//
// @file HttpMessages.msg
//
// HTTP message definitions for the HttpTools components.
//
// @author  Kristjan V. Jonsson
//
// ----------------------------------------------------------------------------
// <h1>HTTP Messages</h1>
//
// <strong>HttpRequestMessage</strong> and <strong>HttpReplyMessage</strong> messages are used by
// the browser and server components to simulate HTTP requests and replies. Their fields are shown
// in the table below:
//
// <table border="1">
//   <tr><th>message type</th><th>field</th><th>Description</th></tr>
//   <tr><td>Common</td><td>targetUrl</td><td>URL of the intended recipient</td></tr>
//   <tr><td>Common</td><td>originatorUrl</td><td>URL of the originator. Only applicable if the originator is a server</td></tr>
//   <tr><td>Common</td><td>protocol</td><td>The HTTP protocol version, http/1.0 or http/1.1</td></tr>
//   <tr><td>Common</td><td>keepAlive</td><td>True if the keep-alive header is simulated as set</td></tr>
//   <tr><td>Common</td><td>serial</td><td>Convenience field, which allows resources requested and sent to be serially tagged</td></tr>
//   <tr><td>Common</td><td>payload</td><td>The message body as a string</td></tr>
//   <tr><td>Request</td><td>requestString</td><td>The request line. Emulates the HTTP request, e.g. GET {resource} {protocol}</td></tr>
//   <tr><td>Request</td><td>bad</td><td>Indicates that the browser is issuing an invalid request. The server responds with a 404:Not found.</td></tr>
//   <tr><td>Response</td><td>resultCode</td><td>The numerical result code, e.g. 200 for OK or 404 for not found</td></tr>
//   <tr><td>Response</td><td>payloadType</td><td>The type of the returned object, page, image or text resource, as an integer</td></tr>
// </table>
//
// The two messages, request and reply, are subclassed from a common base message type,
// HttpBaseMessage.
//
// Servers generate replies containing simplified HTML bodies as payload, either according to the
// random parameters or site definition scripts, as discussed in HttpServer.
// The  body is a list of the form:
// <pre>{resource}[;{site};{delay};{bad}]</pre>
//  - <strong>resource</strong> is the only required field and contains a reference to a resource
//    object, by default simulated as stored locally.
//  - <strong>site</strong> is by default omitted, signifying that the resource is local.
//    This field is only used when referring to resources hosted by third party sites.
//  - <strong>delay</strong> is a optional parameter which causes the receiving browser to insert
//    a delay before requesting the resource.
//  - <strong>bad</strong> is a optional parameter used as a convenience when modeling anomalous
//    messages. The receiving browser will generate a request to the target site, marked as bad
//    (see the table above). The target site will then automatically answer with a
//    <strong>404:Not found</strong>.
//    This feature can be used to simulate usage errors or malicious behavior, e.g. DDoS attacks.
//


namespace inet::httptools;

//
// Base class for HTTP messages
//
packet HttpBaseMessage
{
    @omitGetVerb(true);
    string targetUrl;               // The target URL - request set for browsers, echoed by servers.
    string originatorUrl = "";      // The originator URL. Only applicable for servers.
    int protocol = 11;              // 10 for http/1.0, 11 for http/1.1.
    bool keepAlive = true;          // The keep-alive header
    int serial = 0;                 // Convenience field which allows resource requests to be serially tagged for ease of analysis.
    string heading = "";                // The message heading - request string for requests, response for replies
    string payload = "";                // The payload field
}

//
// Message class for HTTP requests
//
packet HttpRequestMessage extends HttpBaseMessage
{
    @omitGetVerb(true);         // Dont append get to generated getter methods.
    bool badRequest = false;    // Set to true to simulate a bad request. Server answers w. 404.
}


//
// Content type of HTTP responses.
//
enum HttpContentType
{
    CT_UNKNOWN = 0;
    CT_HTML = 1;
    CT_IMAGE = 2;
    CT_TEXT = 3;
};


//
// Message class for HTTP replies
//
packet HttpReplyMessage extends HttpBaseMessage
{
    @omitGetVerb(true);
    int result = 0;      // e.g. 200 for OK, 404 for NOT FOUND.
    int contentType @enum(HttpContentType) = CT_UNKNOWN;
}
