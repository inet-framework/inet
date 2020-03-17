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

import inet.common.TagBase;
import inet.common.Units;

namespace inet::physicallayer;

enum EtherPhyCommand {
        ETH_CMD_SEND = 101;                         // send SFD / SMD-E (these are equals); error, ha van küldés folyamatban
                                                    // param: -
        ETH_CMD_SEND_PREEMPTABLE = 102;             // send SMD-Sx / SMD-Cx (depends on isFirst); error, ha van küldés folyamatban
                                                    // param: fragId, fragCount, isFirst
        ETH_CMD_MODIFY_CURRENT_PREEMPTABLE = 103;   // módosítja az éppen küldés alatt álló csomagot
                                                    // error, ha a megadott bit offset változási pozíción már túljutott a küldés vagy ha nincs küldés folyamatban
                                                    // param: (fragId, fragCount, isFirst), firstChangedOffset
        ETH_CMD_SEND_PREEMPTION_VERIFY = 105;       // send SMD-Verify
                                                    // param: -
                                                    // error, ha van küldés folyamatban
        ETH_CMD_SEND_PREEMPTION_RESPONSE = 106;     // send SMD-Response
                                                    // param: -
                                                    // error, ha van küldés folyamatban
        // CMD_SEND_JAM,
        // CMD_SEND_FILLED_IFG,
};

enum EtherPhyIndCode {
        ETH_IND_RECV = 201;                         // received SFD / SMD-E (these are equals);
        ETH_IND_RECV_PREEMPTABLE = 202;             // received SMD-Sx / SMD-Cx
                                                    // info: fragId, fragCount, isFirst, isLast
        ETH_IND_RECV_PREEMPTION_VERIFY = 205;       // received SMD-Verify
        ETH_IND_RECV_PREEMPTION_RESPONSE = 206;     // received SMD-Response
        // ETH_IND_RECV_JAM,
        // ETH_IND_RECV_FILLED_IFG,
};

//
// This is an abstract base class that should not be directly added as a tag.
//
class PreemptionTagBase extends TagBase
{
    short fragId = 0;   // modulo 4
    short fragCount = 0;   // modulo 4
    bool isFirst = true;
}

//
class PreemptionReq extends PreemptionTagBase
{
}

class PreemptionModifyReq extends PreemptionReq
{
    B firstChangedOffset;
}

//
class PreemptionInd extends PreemptionTagBase
{
    bool isLast = true;
}

