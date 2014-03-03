// * **************************************************************************
// * file:        SimplifiedRadio.ned
// *
// * author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
// *
// * copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
// *
// * author:      Alfonso Ariza
// *
// * copyright:   (c) 2009 Universidad de M�laga
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

package inet.physicallayer.radio.simplified;

import inet.physicallayer.radio.common.RadioBase;

//
// This is an abstract module that defines all parameters for radio modules
//
simple SimplifiedRadio extends RadioBase
{
    parameters:
        string powerSourceModule;
        string obstacleControlModule = default("");     // "obstacles"
        int channelNumber = default(0); // channel identifier this radio listens. Works only with simlified management module. Otherwise it scans all the channels as specified in ieee 80211
        double carrierFrequency @unit("Hz") = default(2.4GHz);
        double bitrate @unit("bps");
        double transmitterPower @unit("mW") = default(20mW); // power used for transmission of messages (in mW)
        double thermalNoise @unit("dBm") = default(-110dBm); // base noise level (dBm)
        double sensitivity @unit("dBm") = default(-85dBm); // received signals with power below sensitivity are ignored
        double maxDistance @unit("m") = default(-1m); // if the maxDistance is bigger of 0 the sensitivity is adjusted to a maximun reception to this value
        bool setReceptionThreshold = default(false);
        double receptionThreshold @unit("dBm") = default(-110dBm);
        double maxDistantReceptionThreshold @unit("m") = default(-1m);
        string radioModel;  // the radio model implementing the IRadioModel interface (C++). e.g. GenericRadioModel, Ieee80211RadioModel

        string NoiseGenerator = default("");
        // generic FreeSpace model parameters
        double pathLossAlpha = default(2); // used by the path loss calculation
        double TransmissionAntennaGainIndB @unit("dB") = default(0dB);  // Transmission Antenna Gain
        double ReceiveAntennaGainIndB @unit("dB") = default(0dB);       // Receive Antenna Gain
        double SystemLossFactor @unit("dB") = default(0dB);             // System Loss of Hardware
        // two ray model paramaeters
        double TransmiterAntennaHigh @unit("m") = default(1m);   // Transmitter Antenna High
        double ReceiverAntennaHigh @unit("m") = default(1m);   // Receiver Antenna High
        // LogNormalShadowingModel
        double sigma = default(1);
        // NakagamiModel
        double nak_m = default(1);
        // RiceModel
        double K @unit("dB") = default(8dB);
        // misc
        bool drawCoverage = default(true);  // draw the coverage area in Tkenv
        int refreshCoverageInterval @unit("s") = default(0s);  // update period in simulation time for coverage redraw

        xml SensitivityTable = default(xml("<NoSensitivityTable> </NoSensitivityTable>"));

        @class(SimplifiedRadio);
        @signal[bitrate](type="double"; unit=bps); // current transmit bitrate
        @signal[lossRate](type="double"); // 0..1; percentage of frames received incorrectly in the last period
        @statistic[bitrate](title="tx bitrate"; record=min,max,timeavg,vector; interpolationmode=sample-hold; unit=bps);
        @statistic[lossRate](title="packet loss rate"; record=min,max,timeavg,vector; interpolationmode=backward-sample-hold);
}
