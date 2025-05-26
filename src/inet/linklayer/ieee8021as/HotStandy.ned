//
// @authors: Lucas Haug
//           Lun-Yu Yuan
//           Chunzhi Guo
//           University of Stuttgart, Deterministic6G

package inet.linklayer.ieee8021as;

import inet.common.SimpleModule;


// This module is a basic implementation of the IEEE 802.1ASdm HotStandby amendment.
// It is part of the MultiDomainGptp module and keeps track of the synchronization state of contained
// gPTP modules.
//
// In case one gPTP module loses synchronization, it will switch the activeClockIndex of the multiClock
// to the clock of a gPTP instance still in sync.
//
// Note: This module is not a complete implementation of the HotStandby amendment and only provides
// the basic functionality of switching the active clock if one gPTP instance loses sync.
// For example, a smooth transition when switching between synchronization instances is not implemented.
simple HotStandby extends SimpleModule
{
    parameters:
        @display("i=block/blackboard");
        @class(HotStandby);
        @selfMessageKinds(inet::GptpSelfMsgKind);
        string clockModule = default(""); // Path to a multiClock
}
