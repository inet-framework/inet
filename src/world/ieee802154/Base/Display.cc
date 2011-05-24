
#include "Display.h"

Define_Module( Display );


/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ===================================
/**
 * Initialization routine
 */
void Display::initialize(int aStage)
{
    cSimpleModule::initialize(aStage);

    if (0 == aStage)
    {
        ev << "initializing Display" << endl;

        // calculate communication radius
        double interference_distance = calcInterfDist();
        ev << "interference_distance: " << interference_distance << endl;

        // display communication radius
        cDisplayString *dispStr = &getParentModule()->getDisplayString();

        char buf[20];
        sprintf(buf, "%f", interference_distance);
        dispStr->setTagArg("r",0,buf);          // radius of the circle
        dispStr->setTagArg("r",3,"1");          // line width
    }
    else if (1 == aStage)
    {

    }
}

//============================= OPERATIONS ===================================
/**
 * @short calculates the interference distance of a host based on
 *  physical parameters
 * @return the interference distance
 */
double Display::calcInterfDist()
{
    double speed_of_light_m_s = 300000000.0; // in m/s
    double interference_distance;

    //the carrier frequency used
    double carrier_frequency = par("carrierFrequency"); //in 1/s

    //maximum transmission power possible
    double max_trans_power   = par("transmitterPower"); // in mW

    //signal attenuation threshold (= minimum receive power in dBm)
    double sat               = par("sensitivity");

    //path loss coefficient
    double alpha             = par("alpha");

    double wave_length_m     = (speed_of_light_m_s / carrier_frequency); // in m

    //minimum power level to be able to physically receive a signal in mW
    double min_receive_power = pow(10.0, sat / 10.0);

    interference_distance    = pow(wave_length_m * wave_length_m * max_trans_power /
                                   (16.0 * M_PI * M_PI * min_receive_power), 1.0 / alpha);

    return interference_distance;
}
