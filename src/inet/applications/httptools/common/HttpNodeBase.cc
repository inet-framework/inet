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

#include "inet/applications/httptools/common/HttpNodeBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace httptools {

HttpNodeBase::HttpNodeBase()
{
    m_bDisplayMessage = false;
    m_bDisplayResponseContent = false;
}

void HttpNodeBase::initialize(int stage)
{
    EV_DEBUG << "Initializing base HTTP browser component -- stage " << stage << endl;

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
    }
}

void HttpNodeBase::sendDirectToModule(HttpNodeBase *receiver, cPacket *pckt, simtime_t constdelay, rdObject *rdDelay)
{
    if (pckt == nullptr)
        return;
    simtime_t delay = constdelay + transmissionDelay(pckt);
    if (rdDelay != nullptr)
        delay += rdDelay->draw();
    EV_DEBUG << "Sending " << pckt->getName() << " direct to " << getContainingNode(receiver)->getName() << " with a delay of " << delay << " s\n";
    sendDirect(pckt, receiver, "httpIn");
}

double HttpNodeBase::transmissionDelay(cPacket *pckt)
{
    if (linkSpeed == 0)
        return 0.0; // No delay if link speed unspecified
    return pckt->getBitLength() / ((double)linkSpeed);    // The link speed is in bit/s
}

void HttpNodeBase::logRequest(const HttpRequestMessage *httpRequest)
{
    if (!enableLogging)
        return;
    if (outputFormat == lf_short)
        logEntry(formatHttpRequestShort(httpRequest));
    else
        logEntry(formatHttpRequestLong(httpRequest));
    if (m_bDisplayMessage)
        EV_INFO << "Request:\n" << formatHttpRequestLong(httpRequest);
}

void HttpNodeBase::logResponse(const HttpReplyMessage *httpResponse)
{
    if (!enableLogging)
        return;
    if (outputFormat == lf_short)
        logEntry(formatHttpResponseShort(httpResponse));
    else
        logEntry(formatHttpResponseLong(httpResponse));
    if (m_bDisplayMessage)
        EV_INFO << "Response:\n" << formatHttpResponseLong(httpResponse);
}

void HttpNodeBase::logEntry(std::string line)
{
    if (!enableLogging || logFileName.empty())
        return;

    std::ofstream outfile;
    time_t curtime;
    time(&curtime);

    outfile.open(logFileName.c_str(), std::ios::app);
    if (outfile.tellp() == 0)
        outfile << "time;simtime;logging-node;sending-node;type;originator-url;target-url;protocol;keep-alive;serial;heading;bad-req;result-code;content-type" << endl;
    outfile << curtime << ";" << simTime() << ";" << host->getName();
    if (outputFormat == lf_short)
        outfile << ";";
    else
        outfile << endl;
    outfile << line << endl;
    outfile.close();
}

std::string HttpNodeBase::formatHttpRequestShort(const HttpRequestMessage *httpRequest)
{
    std::ostringstream str;

    std::string originatorStr = "";
    cModule *originator = findContainingNode(httpRequest->getSenderModule());
    if (originator != nullptr)
        originatorStr = originator->getFullName();

    str << originatorStr << ";";
    str << "REQ;" << httpRequest->originatorUrl() << ";" << httpRequest->targetUrl() << ";";
    str << httpRequest->protocol() << ";" << httpRequest->keepAlive() << ";" << httpRequest->serial() << ";";
    str << httpRequest->heading() << ";" << httpRequest->badRequest() << ";;";    // Skip the response specific fields

    return str.str();
}

std::string HttpNodeBase::formatHttpResponseShort(const HttpReplyMessage *httpResponse)
{
    std::ostringstream str;

    std::string originatorStr = "";
    cModule *originator = findContainingNode(httpResponse->getSenderModule());
    if (originator != nullptr)
        originatorStr = originator->getFullName();

    str << originatorStr << ";";
    str << "RESP;" << httpResponse->originatorUrl() << ";" << httpResponse->targetUrl() << ";";
    str << httpResponse->protocol() << ";" << httpResponse->keepAlive() << ";" << httpResponse->serial() << ";";
    str << httpResponse->heading() << ";;";    // Skip the request specific fields
    str << httpResponse->result() << ";" << httpResponse->contentType();

    return str.str();
}

std::string HttpNodeBase::formatHttpRequestLong(const HttpRequestMessage *httpRequest)
{
    std::ostringstream str;

    str << "REQUEST: " << httpRequest->getName() << " -- " << httpRequest->getByteLength() << " bytes\n";
    str << "Target URL:" << httpRequest->targetUrl() << "  Originator URL:" << httpRequest->originatorUrl() << endl;

    str << "PROTOCOL:";
    switch (httpRequest->protocol()) {    // MIGRATE40: kvj
        case 10:
            str << "HTTP/1.0";
            break;

        case 11:
            str << "HTTP/1.1";
            break;

        default:
            str << "UNKNOWN";
            break;
    }
    str << "  ";

    str << "KEEP-ALIVE:" << httpRequest->keepAlive() << "  ";
    str << "BAD-REQ:" << httpRequest->badRequest() << "  ";
    str << "SERIAL:" << httpRequest->serial() << "  " << endl;

    str << "REQUEST:" << httpRequest->heading() << endl;

    return str.str();
}

std::string HttpNodeBase::formatHttpResponseLong(const HttpReplyMessage *httpResponse)
{
    std::ostringstream str;

    str << "RESPONSE: " << httpResponse->getName() << " -- " << httpResponse->getByteLength() << " bytes\n";

    str << "Target URL:" << httpResponse->targetUrl() << "  Originator URL:" << httpResponse->originatorUrl() << endl;

    str << "PROTOCOL:";
    switch (httpResponse->protocol()) {
        case 10:
            str << "HTTP/1.0";
            break;

        case 11:
            str << "HTTP/1.1";
            break;

        default:
            str << "UNKNOWN";
            break;
    }
    str << "  ";

    str << "RESULT:" << httpResponse->result() << "  ";
    str << "KEEP-ALIVE:" << httpResponse->keepAlive() << "  ";
    str << "SERIAL:" << httpResponse->serial() << "  " << endl;

    str << "RESPONSE: " << httpResponse->heading() << endl;

    str << "CONTENT-TYPE:";
    switch (httpResponse->contentType()) {
        case CT_HTML:
            str << "HTML DOC";
            break;

        case CT_TEXT:
            str << "Text/HTML RES";
            break;

        case CT_IMAGE:
            str << "IMG RES";
            break;

        default:
            str << "UNKNOWN";
            break;
    }
    str << endl;

    if (m_bDisplayResponseContent) {
        str << "CONTENT:" << endl;
        str << httpResponse->payload() << endl;
    }

    return str.str();
}

} // namespace httptools

} // namespace inet

