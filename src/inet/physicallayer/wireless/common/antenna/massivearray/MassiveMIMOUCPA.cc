
#include "inet/physicallayer/wireless/common/antenna/massivearray/MassiveMIMOUCPA.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

#include "inet/common/ModuleAccess.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>
#include <vector>
#include <complex>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <mutex>
namespace inet {

namespace physicallayer {
using std::cout;

static double overall_sum(EulerAngles direction);
Define_Module(MassiveMIMOUCPA);

MassiveMIMOUCPA::MassiveMIMOUCPA() : MassiveArray()
{
}

void MassiveMIMOUCPA::initialize(int stage) {
    MassiveArray::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        auto length = m(par("length"));
        double freq = (par("freq"));
        double distance = (par("distance"));
        double phiz = (par("phiz"));
        if (std::isnan(M))
            M = (par("M"));

        cout << "Posizione: " << getMobility()->getCurrentPosition() << endl;
        auto it = risValuesUcpa.find(M);
        if (it == risValuesUcpa.end()) {
            risInt =  computeIntegral();
            risValuesUcpa[M] = risInt;
        }
        else
            risInt = it->second;
        cModule *radioModule = getParentModule();
        IRadio * radio = check_and_cast<IRadio *>(radioModule);
        gain = makeShared<AntennaGain>(length, M, phiz, freq, distance, risInt, this, radio);

/*
        const char *energySourceModule = par("energySourceModule");

        energySource = dynamic_cast<IEnergySource *>(getModuleByPath(energySourceModule));
        if (energySource)
            energyConsumerId = energySource->addEnergyConsumer(this);
*/

    }
}

static double getFunc (double x, double y, int M, int N)
{

    if (x == 0)
            return 0;
        if (y == 0)
            return 0;
        if (y == M_PI/2 || y == M_PI || y == 3/2 * M_PI || y == 2 * M_PI)
            return 0;

        std::complex <double> sum1 = 0 ;
        std::complex <double> term = 1 ;

        int emme = M;
        const std::complex<double> i(0, 1);


        for ( int m=1; m<=emme ; ++m ){
            int val = 6*m;
            for ( int n=1; n<= val; ++n ){

                            double aux2 =  (double)((M_PI*n)/(3*m));
                            double aux =  (double) (sin(x)* cos (y- aux2));
                            sum1 += std::exp(-i* (M_PI * m * aux));
            }

        }

  double third = sin(x);

  double afmod = (double) std::norm(term+sum1);
  return afmod*third;
}

double MassiveMIMOUCPA::AntennaGain::getMaxGain() const {
    double maxG;
    int numel = ourpa->getNumAntennas();
    double numer = 4 * M_PI * numel*numel;
    maxG= 20 * log10 (numer/risInt);
    return maxG;
}


double MassiveMIMOUCPA::AntennaGain::computeGain(const Quaternion &direction) const {

    double gain;
    double maxGain = getMaxGain();
    IRadio *radio = check_and_cast<IRadio *>(ourpa->getParentModule());
    if (phiz == -1) {
        // omni
        return 1;
    }
    int numel = ourpa->getNumAntennas(); //without energy control
    cout << "ACTIVE ARRAY ELEMENTS:" << numel << endl;
    double phizero = phiz * (M_PI / 180);
    double heading = direction.toEulerAngles().alpha.get();
    double elevation = direction.toEulerAngles().beta.get();
    double currentangle = heading * 180.0 / M_PI;
    if (currentangle == phiz)
        gain = maxGain;

    cout << "Thetazero:" << phiz << endl;
    cout << "CurrentAngle(degree):" << currentangle << endl;

    std::complex<double> sum1 = 0;
    std::complex<double> term = 0;
    int emme = M;
    const std::complex<double> i(0, 1);
    for (int m = 1; m <= emme; ++m) {
        int val = 6 * m;
        for (int n = 1; n <= val; ++n) {
            double aux2 = (double) ((M_PI * n) / (3 * m)); //phin=2pi*n/N
            double betax = sin(phizero) * cos(elevation - aux2);
            double auxprova = (double) (sin(heading) * cos(elevation - aux2)
                    + betax);
            sum1 += std::exp(-i * (M_PI * m * (auxprova)));
        }
    }
    double afmodu = std::norm(term + sum1);

    double nume = 4 * M_PI * afmodu;
    gain = 20 * log10(std::abs(nume) / risInt);
    //if (phiz==0)gain=1;
    if (gain < 0)
        gain = 1;
    if (gain > maxGain)
        gain = maxGain;
    Ieee80211ScalarReceiver * rec =
            dynamic_cast<Ieee80211ScalarReceiver *>(const_cast<IReceiver *>(radio->getReceiver()));
    if (rec != nullptr) {
        // is of type Ieee80211ScalarReceiver
        if (phiz >= 0)
            gain = computeRecGain(direction.toEulerAngles().alpha - rad(M_PI));
        else
            gain = computeRecGain(direction.toEulerAngles().alpha + rad(M_PI));
    }

    cout << "Gain (dB) at angle (degree): " << currentangle << " is: " << gain
            << endl;

    return gain;

}

double MassiveMIMOUCPA::AntennaGain::computeRecGain(
        const rad &direction) const {

    double gain;
    double maxGain = getMaxGain();

    if (phiz == -1) {
        // omni
        return 1;
    }

    int numel = ourpa->getNumAntennas(); //without energy control
    cout << "ACTIVE ARRAY ELEMENTS:" << numel << endl;

    double phizero = phiz * (M_PI / 180);

    double heading = direction.get();
    double elevation = 0;
    double currentangle = heading * 180.0 / M_PI;

    if (currentangle == phiz)
        gain = maxGain;

    cout << "Thetazero:" << phiz << endl;
    cout << "CurrentAngle(degree):" << currentangle << endl;
    std::complex<double> sum1 = 0;
    std::complex<double> term = 0;
    int emme = M;
    const std::complex<double> i(0, 1);

    for (int m = 1; m <= emme; ++m) {
        int val = 6 * m;
        for (int n = 1; n <= val; ++n) {

            double aux2 = (double) ((M_PI * n) / (3 * m)); //phin=2pi*n/N
            double betax = sin(phizero) * cos(elevation - aux2);
            double auxprova = (double) (sin(heading) * cos(elevation - aux2)
                    + betax);
            sum1 += std::exp(-i * (M_PI * m * (auxprova)));
        }

    }
    double afmodu = std::norm(term + sum1);

    double nume = 4 * M_PI * afmodu;
    gain = 20 * log10(std::abs(nume) / risInt);

    //if (phiz==0)gain=1;
    if (gain < 0)
        gain = 1;
    if (gain > maxGain)
        gain = maxGain;

    cout << "Gain (dB) at angle (degree): " << currentangle << " is: " << gain
            << endl;

    return gain;

}

double MassiveMIMOUCPA::AntennaGain::getAngolo(Coord p1, Coord p2) const {
    double angolo;
    double cangl;
    double x1, y1, x2, y2;
    double dx, dy;

    x1 = p1.x;
    y1 = p1.y;
    x2 = p2.x;
    y2 = p2.y;
    dx = x2 - x1;
    dy = y2 - y1;
    cangl = dy / dx;
    angolo = atan(cangl) * (180 / 3.14);
    return angolo;

}

std::ostream& MassiveMIMOUCPA::printToStream(std::ostream& stream,  int level, int evFlags) const {
    stream << "MassiveMIMOUCPA";
    if (level >= PRINT_LEVEL_DETAIL) {

        stream << ", length = " << gain->getLength();

        cout << getFullPath().substr(13, 8) << " Posizione: "
                << getMobility()->getCurrentPosition() << endl;

        //  std::vector<Coord>positions = this->getMempos()->getListaPos();
        //  cout<<getAngolo(Coord(2,0,0),Coord(1,2,0))<<endl;
        //  cout<<getAngolo(positions[0],positions[1])<<endl;
    }
    return AntennaBase::printToStream(stream, level, evFlags);
}

void MassiveMIMOUCPA::receiveSignal(cComponent *source, simsignal_t signalID,
        double val, cObject *details) {
    if (signalID == MassiveArrayConfigureChange) {
        auto radio =  check_and_cast<IRadio *>(getParentModule());
        if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER
                && radio->getTransmissionState()
                        == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            // pending
            pendingConfiguration = true;
            nextValue = val;
            return;
        }
        nextValue = NaN;
        pendingConfiguration = false;
        if (val >= -180 && val <= 180)
            gain->setPhizero(val);
        else if (val == 360)
            gain->setPhizero(360);
    }
    else {
        if (pendingConfiguration) {
            auto radio =  check_and_cast<IRadio *>(getParentModule());

            if (radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER
                    || (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER
                            && radio->getTransmissionState()
                            != IRadio::TRANSMISSION_STATE_TRANSMITTING)) {
                if (std::isnan(nextValue))
                    throw cRuntimeError("next value is nan");
                if (nextValue >= -180 && nextValue <= 180)
                    gain->setPhizero(nextValue);
                else if (nextValue == 360)
                    gain->setPhizero(360);
                nextValue = NaN;
                pendingConfiguration = false;
            }
        }
    }
}

void MassiveMIMOUCPA::receiveSignal(cComponent *source, simsignal_t signalID,
        long val, cObject *details) {
    if (signalID != MassiveArrayConfigureChange) {
        // Radio signals
        if (pendingConfiguration) {
            auto radio =  check_and_cast<IRadio *>(getParentModule());
            if (radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER
                    || (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER
                            && radio->getTransmissionState()
                                    != IRadio::TRANSMISSION_STATE_TRANSMITTING)) {
                if (std::isnan(nextValue))
                    throw cRuntimeError("next value is nan");
                if (nextValue >= -180 && nextValue <= 180)
                    gain->setPhizero(nextValue);
                else if (nextValue == 360)
                    gain->setPhizero(360);
                nextValue = NaN;
                pendingConfiguration = false;
            }
        }
    }
}

void MassiveMIMOUCPA::setDirection(const double &val)
{
    auto radio =  check_and_cast<IRadio *>(getParentModule());
    if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER
            && radio->getTransmissionState()
                    == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
        // pending
        pendingConfiguration = true;
        nextValue = val;
        return;
    }

    nextValue = NaN;
    if (val >= -180 && val <= 180)
        gain->setPhizero(val);
    else if (val == 360)
        gain->setPhizero(360);
    pendingConfiguration = false;
}

double MassiveMIMOUCPA::computeIntegral() {

    Simpson2D::Mat matrix;
    Simpson2D::limits limitX, limitY;
    limitX.setLower(0);
    limitX.setUpper(M_PI);
    limitY.setLower(0);
    limitY.setUpper(2 * M_PI);
    return Simpson2D::Integral(getFunc, limitX, limitY, 1999, M, N);
}

} // namespace physicallayer

} // namespace inet

