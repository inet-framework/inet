//
// Copyright 2010 Zoltan Bojthe
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPGenericApp.h"
#include "TCPSocket.h"
#include "TCPCommand_m.h"
#include "GenericAppMsg_m.h"


Define_Module(TCPGenericApp);

TCPGenericApp::TCPGenericApp(unsigned stacksize)
  : cSimpleModule(stacksize),
    tcpDataTransferMode(TCP_TRANSFER_UNDEFINED)
{
};

void TCPGenericApp::initialize()
{
}

void TCPGenericApp::readTransferModePar()
{
    const char *transferMode = par("TCPdataTransferMode");

    if(0 == transferMode || 0 == transferMode[0])
        opp_error("Missing/empty TCPdataTransferMode parameter at %s.", getFullPath().c_str());
    else if (0==strcmp(transferMode, "bytecount"))
        tcpDataTransferMode = TCP_TRANSFER_BYTECOUNT;
    else if (0 == strcmp(transferMode, "object"))
        tcpDataTransferMode = TCP_TRANSFER_OBJECT;
    else if (0 == strcmp(transferMode, "bytestream"))
        tcpDataTransferMode = TCP_TRANSFER_BYTESTREAM;
    else
        opp_error("Invalid '%s' TCPdataTransferMode parameter at %s.", transferMode, getFullPath().c_str());
}
