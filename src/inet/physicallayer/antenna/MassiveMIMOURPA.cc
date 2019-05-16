
#include "inet/physicallayer/antenna/MassiveMIMOURPA.h"
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

double MassiveMIMOURPA::risInt = -1;

int MassiveMIMOURPA::M = NaN;
int MassiveMIMOURPA::N = NaN;
simsignal_t MassiveMIMOURPA::MassiveMIMOURPAConfigureChange = registerSignal(
        "MassiveMIMOURPAConfigureChange");

Define_Module(MassiveMIMOURPA);

MassiveMIMOURPA::MassiveMIMOURPA() :
        AntennaBase(), length(NaN),
        freq(NaN),
        distance (NaN),
        phiz(NaN)


{


}

void MassiveMIMOURPA::initialize(int stage) {
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        length = m(par("length"));
        freq = (par("freq"));
        distance = (par("distance"));
        phiz = (par("phiz"));
        M = (par("M"));
        N = (par("N"));
#ifdef REGISTER_IN_NODE
		// this code register in the node module
		cModule *mod = getContainingNode(this);
		mod->subscribe(MassiveMIMOURPAConfigureChange, this);
#else
		// register in the interface module
		getParentModule()->getParentModule()->subscribe(MassiveMIMOURPAConfigureChange, this);
#endif
        // cout << "Posizione: " << getMobility()->getCurrentPosition() << endl;

		if (risInt < 0)
		      risInt =  Simpson2D::calcolaInt();
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


 int emme = MassiveMIMOURPA::M;
 int enne = MassiveMIMOURPA::N;

 double first = sin(0.5*(emme)*M_PI*sin(x)*cos(y))/sin(0.5*M_PI*sin(x)*cos(y));
  double second = sin(0.5*(enne)*M_PI*sin(x)*sin(y))/sin(0.5*M_PI*sin(x)*sin(y));
  double third = sin(x);
  double afmod= (first*second)*(first*second);
  return afmod*third;}

double MassiveMIMOURPA::getMaxGain() const {

       IRadio *radio= check_and_cast<IRadio *>(getParentModule());
       IRadioMedium *ra =  const_cast< IRadioMedium *> (radio->getMedium());
       RadioMedium *rm=dynamic_cast< RadioMedium *>(ra);
    double maxG;
    int numel = getNumAntennas();


    double numer = 4 * M_PI * numel*numel;
    maxG= 20 * log10 (numer/risInt);
    return maxG;
}

double  MassiveMIMOURPA::computeGain(const EulerAngles direction) const {

       double gain;
       double maxGain = getMaxGain();
       IRadio *radio= check_and_cast<IRadio *>(getParentModule());
       IRadioMedium *ra =  const_cast< IRadioMedium *> (radio->getMedium());
       RadioMedium *rm=dynamic_cast< RadioMedium *>(ra);

    if (phiz == -1) {
        // omni
        return 1;
    }


    int numel = getNumAntennas(); //without energy control
    cout<<"ACTIVE ARRAY ELEMENTS:"<<numel<<endl;

    double c = 300000 * 10 * 10 * 10;
    double lambda = c / freq;
    double d = distance * lambda;

    double k = (2 * M_PI) / lambda;
    double phizero = phiz * (M_PI / 180);

    double heading = direction.alpha;
    double elevation = direction.beta;
    double currentangle = heading * 180.0/M_PI;

    if (currentangle == phiz ) gain = maxGain;
    cout<<"PHI:"<<phiz<<endl;
    cout<<"CurrentAngle(degree):"<<currentangle<<endl;
    double psi1 = (k * d * sin(heading) * cos(heading)) + (- k * d * sin(phizero) * cos(phizero));
    double psi2 = (k * d * sin(heading) * sin(heading)) + (- k * d * sin(phizero) * sin(phizero));
    double first = sin(0.5*M*psi1)/sin(0.5*psi1);
    double second = sin(0.5*N*psi2)/sin(0.5*psi2);
    double afmodu= (first*second)*(first*second);

    double nume = 4 * M_PI * afmodu;
       gain= 20 * log10 (nume/risInt);

    //if (phiz==0)gain=1;
    if (gain<0)gain=1;
    if (gain > maxGain) gain=maxGain;

  Ieee80211ScalarReceiver * rec = dynamic_cast <Ieee80211ScalarReceiver *>(const_cast<IReceiver *>(radio->getReceiver()));
   if (rec != nullptr ) {
    // is of type Ieee80211ScalarReceiver
       if (phiz >=0 )
           gain = computeRecGain(direction.alpha - M_PI);
       else
           gain = computeRecGain(direction.alpha + M_PI);
   }

   cout<<"Gain (dB) at angle (degree): "<<currentangle<< " is: "<<gain<<endl;

    return gain;

}

double MassiveMIMOURPA::computeRecGain(const EulerAngles &direction) const {

    double gain;
    double maxGain = getMaxGain();
    IRadio *radio= check_and_cast<IRadio *>(getParentModule());
    IRadioMedium *ra =  const_cast< IRadioMedium *> (radio->getMedium());
    RadioMedium *rm=dynamic_cast< RadioMedium *>(ra);
 if (phiz == -1) {
     // omni
     return 1;
 }


 int numel = getNumAntennas(); //without energy control
 cout<<"ACTIVE ARRAY ELEMENTS:"<<numel<<endl;

 double c = 300000 * 10 * 10 * 10;
 double lambda = c / freq;
 double d = distance * lambda;

 double k = (2 * M_PI) / lambda;
 double phizero = phiz * (M_PI / 180);

 double heading = direction.alpha;
 double elevation = direction.beta;
 double currentangle = heading * 180.0/M_PI;

 if (currentangle == phiz ) gain = maxGain;

 double psi1 = (k * d * sin(heading) * cos(heading)) + (- k * d * sin(phizero) * cos(phizero));
 double psi2 = (k * d * sin(heading) * sin(heading)) + (- k * d * sin(phizero) * sin(phizero));
 double first = sin(0.5*M*psi1)/sin(0.5*psi1);
 double second = sin(0.5*N*psi2)/sin(0.5*psi2);
 double afmodu= (first*second)*(first*second);

 double nume = 4 * M_PI * afmodu;
    gain= 20 * log10 (nume/risInt);

 //if (phiz==0)gain=1;
 if (gain<0)gain=1;
 if (gain > maxGain) gain=maxGain;

 return gain;

}

double MassiveMIMOURPA::getAngolo(Coord p1, Coord p2) const {
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


std::ostream& MassiveMIMOURPA::printToStream(std::ostream& stream, int level) const {
    stream << "MassiveMIMOURPA";
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

void MassiveMIMOURPA::receiveSignal(cComponent *source, simsignal_t signalID, double val, cObject *details)
{
    if (signalID == MassiveMIMOURPAConfigureChange)
    {
        if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER &&
                radio->getTransmissionState() == IRadio::TRANSMISSION_STATE_TRANSMITTING)
        {
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

void MassiveMIMOURPA::receiveSignal(cComponent *source, simsignal_t signalID, long val, cObject *details)
{
    if (signalID != MassiveMIMOURPAConfigureChange)
        // Radio signals
        if (pendingConfiguration)
        {
            if (radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER ||
                    (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER && radio->getTransmissionState() != IRadio::TRANSMISSION_STATE_TRANSMITTING))
            {
                if (val >= -180 && val <= 180)
                    phiz = val;
                else if (val == 360)
                    phiz = 360;
            }
        }
}

void Simpson2D::initializeCoeff(Mat &coeff,int size) {
    Vec uniDiCoeff;
    // check if size is odd or even
    int correctSize = (size%2 == 0)?size+1:size;
    uniDiCoeff.push_back(1);
    for (int i = 0; i < correctSize-2; i++) {
        if (i%2)
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


double Simpson2D::Integral(double (*fun)(double,double), const Mat &coeff, limits xLimit, limits yLimit, int size) {

    double xInterval = xLimit.getUpper() - xLimit.getLower();
    double yInterval = yLimit.getUpper() - yLimit.getLower();
    double xStep = xInterval/(coeff.size()-1);
    double yStep = yInterval/(coeff.size()-1);
    double val = 0;

    double x = xLimit.getLower();
    for (unsigned int i = 0; i < coeff.size(); i++) {
        double y = yLimit.getLower();
        for (unsigned int j = 0; j < coeff.size(); j++) {
            val += fun(x,y)*coeff[i][j];
            y += yStep;
        }
        x += xStep;
    }

    return val*xStep*yStep/9;
}

double Simpson2D::Integral(double (*fun)(double,double),limits xLimit, limits yLimits, int tam) {
    Mat coeff;
    initializeCoeff(coeff, tam);
    return Integral(fun,coeff,xLimit, yLimits, tam);
}

double Simpson2D::calcolaInt(){
    Simpson2D::Mat matrix;
        Simpson2D::limits limitX,limitY;
        limitX.setLower(0);
        limitX.setUpper(M_PI);
        limitY.setLower(0);
        limitY.setUpper(2* M_PI);
        double val =  Simpson2D::Integral(getFunc, limitX, limitY, 3999);

        return val;
}

} // namespace physicallayer

} // namespace inet

