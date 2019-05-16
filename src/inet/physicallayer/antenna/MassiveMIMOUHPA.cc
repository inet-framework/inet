
#include "inet/physicallayer/antenna/MassiveMIMOUHPA.h"
#include "inet/physicallayer/base/packetlevel/AntennaBase.h"

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

double MassiveMIMOUHPA::risInt = -1;


int MassiveMIMOUHPA::M = NaN;
simsignal_t MassiveMIMOUHPA::MassiveMIMOUHPAConfigureChange = registerSignal(
        "MassiveMIMOUHPAConfigureChange");

static double overall_sum(EulerAngles direction);

Define_Module(MassiveMIMOUHPA);

MassiveMIMOUHPA::MassiveMIMOUHPA() :
        AntennaBase(), length(NaN),
        freq(NaN),
        distance (NaN),
        phiz(NaN)


{



}

void MassiveMIMOUHPA::initialize(int stage) {
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        length = m(par("length"));
        freq = (par("freq"));
        distance = (par("distance"));
        phiz = (par("phiz"));
        M = (par("M"));
#ifdef REGISTER_IN_NODE
		// this code register in the node module
		cModule *mod = getContainingNode(this);
		mod->subscribe(MassiveMIMOUHPAConfigureChange, this);
#else
		// register in the interface module
		getParentModule()->getParentModule()->subscribe(MassiveMIMOUHPAConfigureChange, this);
		
#endif
        // cout << "Posizione: " << getMobility()->getCurrentPosition() << endl;

		if (risInt < 0)
		      risInt =  calcolaInt();
        cModule *radioModule = getParentModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receivedSignalPartChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

/*
        const char *energySourceModule = par("energySourceModule");

        energySource = dynamic_cast<IEnergySource *>(getModuleByPath(energySourceModule));
        if (energySource)
            energyConsumerId = energySource->addEnergyConsumer(this);
*/

    }
}

static double getFunc (double x, double y)
{

    if (x == 0)
            return 0;
        if (y == 0)
            return 0;
        if (y == M_PI/2 || y == M_PI || y == 3/2 * M_PI || y == 2 * M_PI)
            return 0;

        std::complex <double> sum1 = 0 ;
        std::complex <double> sum2 = 0 ;

        int emme = MassiveMIMOUHPA::M;
        const std::complex<double> i(0, 1);


        for ( int m=-emme; m<=emme ; ++m ){
            int val = (2*emme) - abs(m);
            sum2 = 0;
            for ( int n=0; n<= val; ++n ){
                double aux = n * sin (x)* cos(y);
                std::complex<double> aux2 = exp(i* M_PI * aux);
                sum2 += aux2;
            }
            double vx = sin(x) * cos(y);
            double vy = sin(x) * sin(y);
            double aux = (m * vy) - ((double)((emme*2.0) - abs(m))/2.0 * vx) - (vx * (double)m/2.0);
            sum1 += std::exp(i* M_PI *aux) * sum2;
            //sum1 += exp((i* M_PI *((m * sin (x)* sin(y))))-((0.5*((2* emme) - abs(m))) * sin (x)* cos(y))-((0.5*m * sin (x)* cos(y))))*sum2;
        }

        return std::norm(sum1) * sin(x);

  double third = sin(x);

  double afmod = std::norm(sum1);
  return afmod*third;
}

double MassiveMIMOUHPA::getMaxGain() const
{
       IRadio *radio= check_and_cast<IRadio *>(getParentModule());
       IRadioMedium *ra =  const_cast< IRadioMedium *> (radio->getMedium());
       RadioMedium *rm=dynamic_cast< RadioMedium *>(ra);
       double maxG;
       int numel = getNumAntennas();
       double numer = 4 * M_PI * numel*numel;
       maxG= 20 * log10 (numer/risInt);
       return maxG;
}

double MassiveMIMOUHPA::computeGain(const EulerAngles direction) const {

    double gain;
    double maxGain = getMaxGain();
    IRadio *radio = check_and_cast<IRadio *>(getParentModule());
    IRadioMedium *ra = const_cast<IRadioMedium *>(radio->getMedium());
    RadioMedium *rm = dynamic_cast<RadioMedium *>(ra);

    if (phiz == -1) {
        // omni
        return 1;
    }

    int numel = getNumAntennas(); //without energy control
    cout << "ACTIVE ARRAY ELEMENTS:" << numel << endl;

    double c = 300000 * 10 * 10 * 10;
    double lambda = c / freq;
    double d = distance * lambda;

    double k = (2 * M_PI) / lambda;
    double phizero = phiz * (M_PI / 180);

    double heading = direction.alpha;
    double elevation = direction.beta;
    double currentangle = heading * 180.0 / M_PI;

    if (currentangle == phiz)
        gain = maxGain;
    cout << "PHI:" << phiz << endl;
    cout << "CurrentAngle(degree):" << currentangle << endl;

    std::complex<double> sum1 = 0;
    std::complex<double> sum2 = 0;
    int emme = MassiveMIMOUHPA::M;
    const std::complex<double> i(0, 1);

    double betax = sin(phizero) * cos(elevation);
    double betay = (sin(phizero) * sin(phizero));

    for (int m = -emme; m <= emme; ++m) {
        int val = (2 * emme) - abs(m);
        sum2 = 0;
        for (int n = 0; n <= val; ++n) {

            double aux = n * (sin(heading) * cos(elevation));
            double auxprova = n
                    * (double) (sin(heading) * cos(elevation) + betax);

            std::complex<double> aux2 = exp(i * M_PI * auxprova);
            sum2 += aux2;
        }

        double vx = (sin(heading) * cos(elevation));
        double vy = (sin(heading) * sin(elevation));
        double aux = (m * vy) - ((double) ((emme * 2.0) - abs(m)) / 2.0 * vx)
                - (vx * (double) m / 2.0);
        sum1 += std::exp(i * M_PI * aux) * sum2;

    }

    double afmodu = std::norm(sum1);

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
            gain = computeRecGain(direction.alpha - M_PI);
        else
            gain = computeRecGain(direction.alpha + M_PI);
    }

    cout << "Gain (dB) at angle (degree): " << currentangle << " is: " << gain
            << endl;

    return gain;

}

double MassiveMIMOUHPA::computeRecGain(const EulerAngles &direction) const {

    double gain;
    double maxGain = getMaxGain();
    IRadio *radio = check_and_cast<IRadio *>(getParentModule());
    IRadioMedium *ra = const_cast<IRadioMedium *>(radio->getMedium());
    RadioMedium *rm = dynamic_cast<RadioMedium *>(ra);
    if (phiz == -1) {
        // omni
        return 1;
    }

    int numel = getNumAntennas(); //without energy control
    cout << "ACTIVE ARRAY ELEMENTS:" << numel << endl;

    double c = 300000 * 10 * 10 * 10;
    double lambda = c / freq;
    double d = distance * lambda;

    double k = (2 * M_PI) / lambda;
    double phizero = phiz * (M_PI / 180);

    double heading = direction.alpha;
    double elevation = direction.beta;
    double currentangle = heading * 180.0 / M_PI;

    if (currentangle == phiz)
        gain = maxGain;

    std::complex<double> sum1 = 0;
    std::complex<double> sum2 = 0;
    int emme = MassiveMIMOUHPA::M;
    const std::complex<double> i(0, 1);
    double betax = sin(phizero) * cos(elevation);
    double betay = (sin(phizero) * sin(phizero));
    for (int m = -emme; m <= emme; ++m) {
        int val = (2 * emme) - abs(m);
        sum2 = 0;
        for (int n = 0; n <= val; ++n) {

            double aux = n * (sin(heading) * cos(elevation));
            double auxprova = (double) n
                    * (sin(heading) * cos(elevation) + betax);

            std::complex<double> aux2 = exp(i * M_PI * auxprova);
            sum2 += aux2;
        }

        double vx = (sin(heading) * cos(elevation));
        double vy = (sin(heading) * sin(elevation));
        double aux = (m * vy) - ((double) ((emme * 2.0) - abs(m)) / 2.0 * vx)
                - (vx * (double) m / 2.0);
        sum1 += std::exp(i * M_PI * aux) * sum2;
        //sum1 += exp((i* M_PI *((m * sin (x)* sin(y))))-((0.5*((2* emme) - abs(m))) * sin (x)* cos(y))-((0.5*m * sin (x)* cos(y))))*sum2;
    }

    double afmodu = std::norm(sum1);

    double nume = 4 * M_PI * afmodu;
    gain = 20 * log10(std::abs(nume) / risInt);

    //if (phiz==0)gain=1;
    if (gain < 0)
        gain = 1;
    if (gain > maxGain)
        gain = maxGain;

    return gain;

}

double MassiveMIMOUHPA::getAngolo(Coord p1, Coord p2) const {
    double angolo;
    double cangl, intercept;
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

std::ostream& MassiveMIMOUHPA::printToStream(std::ostream& stream,
        int level) const {
    stream << "MassiveMIMOUHPA";
    if (level >= PRINT_LEVEL_DETAIL) {

        stream << ", length = " << length;

        cout << getFullPath().substr(13, 8) << " Posizione: "
                << getMobility()->getCurrentPosition() << endl;

        //  std::vector<Coord>positions = this->getMempos()->getListaPos();
        //  cout<<getAngolo(Coord(2,0,0),Coord(1,2,0))<<endl;
        //  cout<<getAngolo(positions[0],positions[1])<<endl;
    }
    return AntennaBase::printToStream(stream, level);
}

void MassiveMIMOUHPA::receiveSignal(cComponent *source, simsignal_t signalID,
        double val, cObject *details) {
    if (signalID == MassiveMIMOUHPAConfigureChange) {
        if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER
                && radio->getTransmissionState()
                        == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            // pending
            pendingConfiguration = true;
            newConfigurtion = val;
        }
        if (val >= -180 && val <= 180)
            phiz = val;
        else if (val == 360)
            phiz = 360;
    }
}

void MassiveMIMOUHPA::receiveSignal(cComponent *source, simsignal_t signalID,
        long val, cObject *details) {
    if (signalID != MassiveMIMOUHPAConfigureChange)
        // Radio signals
        if (pendingConfiguration) {
            if (radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER
                    || (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER
                            && radio->getTransmissionState()
                                    != IRadio::TRANSMISSION_STATE_TRANSMITTING)) {
                if (val >= -180 && val <= 180)
                    phiz = val;
                else if (val == 360)
                    phiz = 360;
            }
        }
}

void Simpson2D2::initializeCoeff(Mat &coeff, int size) {
    Vec uniDiCoeff;
    // check if size is odd or even
    int correctSize = (size % 2 == 0) ? size + 1 : size;
    uniDiCoeff.push_back(1);
    for (int i = 0; i < correctSize - 2; i++) {
        if (i % 2)
            uniDiCoeff.push_back(2);
        else
            uniDiCoeff.push_back(4);
    }
    uniDiCoeff.push_back(1);
    for (unsigned int i = 0; i < uniDiCoeff.size(); i++) {
        Vec auxCoeff;
        for (unsigned int j = 0; j < uniDiCoeff.size(); j++) {
            auxCoeff.push_back(uniDiCoeff[i] * uniDiCoeff[j]);
        }
        coeff.push_back(auxCoeff);
    }
}

double Simpson2D2::Integral(double (*fun)(double, double), const Mat &coeff,
        limits xLimit, limits yLimit, int size) {

    double xInterval = xLimit.getUpper() - xLimit.getLower();
    double yInterval = yLimit.getUpper() - yLimit.getLower();
    double xStep = xInterval / (coeff.size() - 1);
    double yStep = yInterval / (coeff.size() - 1);
    double val = 0;

    double x = xLimit.getLower();
    for (unsigned int i = 0; i < coeff.size(); i++) {
        double y = yLimit.getLower();
        for (unsigned int j = 0; j < coeff.size(); j++) {
            val += fun(x, y) * coeff[i][j];
            y += yStep;
        }
        x += xStep;
    }

    return val * xStep * yStep / 9;
}

double Simpson2D2::Integral(double (*fun)(double, double), limits xLimit,
        limits yLimits, int tam) {
    Mat coeff;
    initializeCoeff(coeff, tam);
    return Integral(fun, coeff, xLimit, yLimits, tam);
}

double MassiveMIMOUHPA::calcolaInt() {

    Simpson2D2::Mat matrix;
    Simpson2D2::limits limitX, limitY;
    limitX.setLower(0);
    limitX.setUpper(M_PI);
    limitY.setLower(0);
    limitY.setUpper(2 * M_PI);
    return Simpson2D2::Integral(getFunc, limitX, limitY, 999);

}

} // namespace physicallayer

} // namespace inet

