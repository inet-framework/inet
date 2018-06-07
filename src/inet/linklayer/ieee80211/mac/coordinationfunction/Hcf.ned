//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
// 

package inet.linklayer.ieee80211.mac.coordinationfunction;

import inet.linklayer.ieee80211.mac.blockack.OriginatorBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.blockack.RecipientBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.channelaccess.Edca;
import inet.linklayer.ieee80211.mac.channelaccess.Hcca;
import inet.linklayer.ieee80211.mac.contract.IRateControl;
import inet.linklayer.ieee80211.mac.contract.IHcf;
import inet.linklayer.ieee80211.mac.originator.NonQosRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.OriginatorQosAckPolicy;
import inet.linklayer.ieee80211.mac.originator.OriginatorQosMacDataService;
import inet.linklayer.ieee80211.mac.originator.QosRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.QosRtsPolicy;
import inet.linklayer.ieee80211.mac.originator.TxopProcedure;
import inet.linklayer.ieee80211.mac.protectionmechanism.SingleProtectionMechanism;
import inet.linklayer.ieee80211.mac.rateselection.QosRateSelection;
import inet.linklayer.ieee80211.mac.recipient.QosCtsPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientQosAckPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientQosMacDataService;


//
// HCF -- Hybrid Coordination Function
//
module Hcf like IHcf
{
    parameters:
        int numEdcafs = default(4);
        int maxQueueSize = default(100); // -1 means unlimited
        bool isBlockAckSupported = default(false);
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)

        string rxModule;
        string txModule;

        *.rateSelectionModule = "^.rateSelection";
        *.rxModule = "^." + rxModule;

        @class(Hcf);
        @display("i=block/queue");
        @signal[linkBroken](type=Packet);
        @signal[packetDropped](type=Packet);

    submodules:
        edca: Edca {
            parameters:
                numEdcafs = numEdcafs;
                @display("p=150,100");
        }
        hcca: Hcca {
            parameters:
                @display("p=150,200");
        }
        originatorMacDataService: OriginatorQosMacDataService {
            parameters:
                @display("p=150,300");
        }
        recipientMacDataService: RecipientQosMacDataService {
            parameters:
                @display("p=150,400");
        }
        rateSelection: QosRateSelection {
            parameters:
                rateControlModule = "^.rateControl";
                @display("p=350,100");
        }
        rateControl: <default("")> like IRateControl if typename != "" {
            parameters:
                @display("p=350,200");
        }
        singleProtectionMechanism: SingleProtectionMechanism {
            parameters:
                @display("p=350,300");
        }
        edcaMgmtAndNonQoSRecoveryProcedure: NonQosRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^.edca.edcaf[1]"; // best effort
                rtsPolicyModule = "^.rtsPolicy";
                @display("p=550,300");
        }
        edcaDataRecoveryProcedures[numEdcafs]: QosRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^.edca.edcaf[" + string(index) + "]";
                rtsPolicyModule = "^.rtsPolicy";
                @display("p=550,400,column,100");
        }
        edcaTxopProcedures[numEdcafs]: TxopProcedure {
            parameters:
                @display("p=350,400,column,100");
        }
        originatorAckPolicy: OriginatorQosAckPolicy {
            parameters:
                @display("p=750,100");
        }
        recipientAckPolicy: RecipientQosAckPolicy {
            parameters:
                @display("p=750,200");
        }
        rtsPolicy: QosRtsPolicy {
            parameters:
                @display("p=750,300");
        }
        ctsPolicy: QosCtsPolicy {
            parameters:
                @display("p=750,400");
        }
        originatorBlockAckAgreementPolicy: OriginatorBlockAckAgreementPolicy if isBlockAckSupported {
            parameters:
                originatorAckPolicyModule = "^.originatorAckPolicy";
                @display("p=750,500");
        }
        recipientBlockAckAgreementPolicy: RecipientBlockAckAgreementPolicy if isBlockAckSupported {
            parameters:
                @display("p=750,600");
        }
}
