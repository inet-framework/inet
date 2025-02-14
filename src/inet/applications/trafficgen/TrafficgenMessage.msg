//
// Copyright (C) 2015 Felix Weinrank
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

cplusplus {{
#include "inet/common/INETDefs.h"
}}

namespace inet;

enum TrafficGenControlType {
	TRAFFICGEN_UPDATE_BITRATE = 1;
	TRAFFICGEN_START_SENDING = 2;
	TRAFFICGEN_STOP_SENDING = 3;
}

enum TrafficgenMessagetype {
    TRAFFICGEN_MSG_INFO = 1;
    TRAFFICGEN_MSG_DATA = 2;
    TRAFFICGEN_MSG_CONTROL = 3;
};

enum TrafficgenInfotype {
    TRAFFICGEN_INFO_INIT = 1;
    TRAFFICGEN_INFO_FINISH = 2;
};

packet Trafficgen extends cPacket {
    uint32_t id;
}

packet TrafficgenData extends Trafficgen {
    uint32_t priority;
    bool reliable = true;
    bool ordered = true;
    simtime_t creation_time = simTime(); 
}

packet TrafficgenInfo extends Trafficgen {
    uint32_t type;
    uint32_t priority;
    
}

packet TrafficgenControl extends Trafficgen {
    uint8_t	controlMessageType @enum(TrafficGenControlType);
    uint32_t targetBitrate;
}
