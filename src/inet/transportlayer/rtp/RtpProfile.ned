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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.transportlayer.rtp;

//
// Generic RTP Profile. ~RtpProfile is internal to the ~Rtp module;
// instances are created and connected dynamically by ~Rtp.
//
simple RtpProfile
{
    parameters:
        bool autoOutputFileNames;

    gates:
        input rtpIn;
        input payloadSenderIn;
        input payloadReceiverIn[];
        output rtpOut @labels(RtpInnerPacket);
        output payloadSenderOut;
        output payloadReceiverOut[];
}

//
// Audio-Video RTP Profile. ~RtpAvProfile is internal to the ~Rtp module;
// instances are created and connected dynamically by ~Rtp.
//
simple RtpAvProfile
{
    parameters:
        bool autoOutputFileNames;

    gates:
        input rtpIn;
        input payloadSenderIn;
        input payloadReceiverIn[];
        output rtpOut;
        output payloadSenderOut;
        output payloadReceiverOut[];
}

