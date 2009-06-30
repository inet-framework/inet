// $Id$
//------------------------------------------------------------------------------
//	OpticalFrame.msg --
//
//	This file defines 'IpPacket', 'EthFrame' and 'HybridPonFrame' message
//	objects using new message subclassing schemes provided by OMNeT++ starting
//	from version 2.3b1.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


packet OpticalFrame
{
    int lambda;         // optical wavelength on which the frame is sent
}
