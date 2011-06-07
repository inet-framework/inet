
// ***************************************************************************
// 
// HttpTools Project
//// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable 
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
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
// ***************************************************************************


#include "httptServerEvilB.h"

Define_Module(httptServerEvilB); 

void httptServerEvilB::initialize()
{
	httptServer::initialize();

	badLow  = par("minBadRequests");
	badHigh = par("maxBadRequests");

	EV_INFO << "Badguy " << wwwName << " was initialized to launch an attack on www.good.com" << endl;
	EV_INFO << "Minimum " << badLow << " and maximum " << badHigh << " bad requests for each hit." << endl;
}

std::string httptServerEvilB::generateBody()
{
	int numResources = badLow+(int)uniform(0,badHigh-badLow);
	double rndDelay;
	string result;

	char tempBuf[128];
	int refSize;
	for( int i=0; i<numResources; i++ )
	{		
		rndDelay = 10.0+uniform(0,2.0);
		refSize = (int)uniform(500,1000); // The random size represents a random reference string length
		sprintf(tempBuf, "TEXT%.4d.txt;%s;%f;%s;%d\n", i, "www.good.com", rndDelay, "TRUE", refSize);
		result.append(tempBuf);
	}

	return result;
}




