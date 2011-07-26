package inet.linklayer.ieee802154.phyLayer;

import inet.linklayer.radio.Radio;


simple Ieee802154Phy extends Radio
{
    parameters:
        @class(Ieee802154Phy);

        bool debug;
        bool NoBitError = default(true);

        // power used for transmission of messages (mW)
        transmitterPower @unit("mW") = default(1.0mW);

        double shadowingDeviation @unit("dB") = default(0dB); // used by the shadowing model calculation

        // used by Ieee802154RadioModel
        double snirThreshold @unit("dB") = default(4dB);

        // parameters for energy usage
        refreshCoverageInterval @unit("s") = default(5s);

        radioModel @enum("Ieee802154RadioModel") = default("Ieee802154RadioModel");

        // Unused param:
        bitrate = 0;
}
