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
    tcpDataTransferMode(TCP_TRANS_VIRTUALBYTES)
{
};

void TCPGenericApp::initialize()
{
    const char *transferMode = par("TCPdataTransferMode");
    tcpDataTransferMode = 0;

    if(0 == transferMode || 0 == transferMode[0] || 0==strcmp(transferMode, "virtualBytes"))
        tcpDataTransferMode = TCP_TRANS_VIRTUALBYTES;
    else if (0 == strcmp(transferMode, "msgBased"))
        tcpDataTransferMode = TCP_TRANS_MSGBASED;
    else if (0 == strcmp(transferMode, "byteStream"))
        tcpDataTransferMode = TCP_TRANS_BYTESTREAM;
    else
        opp_error("Invalid '%s' TCPdataTransferMode parameter at %s.", transferMode, getFullPath().c_str());
}
