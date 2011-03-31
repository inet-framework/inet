// * **************************************************************************
// * file:        FreeSpaceModel.ned
// *
// * author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
// *
// * copyright:	  (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
// *
// * author:      Alfonso Ariza
// *
// * copyright:	  (c) 2009 Universidad de M�laga
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// ***************************************************************************
// ***************************************************************************

package inet.linklayer.radio;
simple PropagationModel like IRadio
{
    parameters:
        double TransmissionAntennaGainIndB @unit("dB") = default(0dB);  // Transmission Antenna Gain 
        double carrierFrequency @unit("Hz") = default(2.4GHz);
        double ReceiveAntennaGainIndB @unit("dB") = default(0dB);       // Receive Antenna Gain
        double SystemLossFactor @unit("dB") = default(0dB);             // System Loss of Hardware 
        double sigma=default(1); // LogNormalShadowingModel
        double nak_m =default(1); // NakagamiModel
        double KdB = default(8); // RiceModel
        double TransmiterAntennaHigh @unit("m") = default(1m);   // Transmitter Antenna High 
        double ReceiverAntennaHigh   @unit("m") = default(1m);	// Receiver Antenna High
   gates:
        input uppergateIn @labels(PhyControlInfo/down); // from higher layer protocol (MAC)
        output uppergateOut @labels(PhyControlInfo/up); // to higher layer protocol (MAC)
        input radioIn @labels(AirFrame); // to receive frames (AirFrame) on the radio channel
}