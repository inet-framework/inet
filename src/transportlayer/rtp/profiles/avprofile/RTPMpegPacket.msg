//
// Copyright (C) 2008 Andras Varga
// Copyright (C) 2001 Matthias Oppitz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

//
// An ~RTPMpegPacket is intended to be capsulated into an ~RTPPacket when
// transmitting mpeg data with rtp under the rtp audio/video profile.
// It stores information about the mpeg data as described in rfc 2250.
// This implementation doesn't transport real mpeg data. It is intended
// to simulate storing mpeg data by adding length.
// Currently only one header field (picture type) is filled with the right
// value. The values for the other header fields can't be determined by
// reading the gdf file.
//
// @see ~RTPAVProfilePayload32Sender
// @see ~RTPAVProfilePayload32Receiver
//
packet RTPMpegPacket
{
    int headerLength = 4;
    int payloadLength; // size of the mpeg data
    int pictureType;   // picture type of the frame the data in this packet belongs to.

    // currenrtly unused fields:
    //int mzb;
    //int two;
    //int temporalReference;
    //int activeN;
    //int newPictureHeader;
    //int sequenceHeaderPresent;
    //int beginningOfSlice;
    //int endOfSlice;
    //int fbv;
    //int bfc;
    //int ffv;
    //int ffc;
}


