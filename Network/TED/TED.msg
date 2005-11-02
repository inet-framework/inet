//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


cplusplus {{
#include "IPAddress.h"

#define	LINK_STATE_MESSAGE		1
#define	LINK_NOTIFY_MESSAGE		2

typedef std::vector<struct TELinkStateInfo> TELinkStateInfoVector;
}}

class noncobject IPAddress;

struct TELinkStateInfo
{
  fields:
    IPAddress advrouter;
    IPAddress linkid;
    IPAddress local;
    IPAddress remote;
    double metric;
    double MaxBandwidth;
    double UnResvBandwidth[8];
    
	double timestamp;
	unsigned int sourceId;
	unsigned int messageId;
    bool state;
};

struct TELink
{
  fields:
  	IPAddress advrouter;
	IPAddress linkid;
};

message TEDMsg 
{
	fields:
		int command = 0;
};

message LinkStateMsg extends TEDMsg
{
	fields:
		TELinkStateInfo linkInfo[];	
		
		bool request = false;
		bool ack = false;
	
		int command = LINK_STATE_MESSAGE;
};

message LinkNotifyMsg extends TEDMsg
{
  fields:
  	TELink link[];
  	
	int command = LINK_NOTIFY_MESSAGE;
};
