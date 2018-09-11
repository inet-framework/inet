//
// Copyright (C) Opensim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.transportlayer.rtp;

//
// Interface for RTP Payload Receivers.
//
// These modules are internal to the ~Rtp module; instances are created
// and connected dynamically by ~RtpProfile.
//
moduleinterface IRTPPayloadReceiver
{
    parameters:
        string outputFileName;  // use an empty string to disable this logging
        string outputLogFileName;   // use an empty string to disable this logging

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpPayloadReceiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");
        @signal[rcvdPkRtpTimestamp](type=simtime_t; unit=s);
        @statistic[rcvdPkRtpTimestamp](title="received packet RTP timestamp"; record=vector);

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfilePayload10Receiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RtpAvProfilePayload32Receiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

simple RTPAVProfileSampleBasedAudioReceiver like IRTPPayloadReceiver
{
    parameters:
        string outputFileName;
        string outputLogFileName = default("outputLogLoss%d.log");

    gates:
        input profileIn @labels(RtpInnerPacket);
        output profileOut @labels(RtpInnerPacket);
}

