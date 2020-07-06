

#include "inet/physicallayer/base/packetlevel/AntennaBase.h"
#include "inet/physicallayer/antenna/massivearray/MassiveArray.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarReceiver.h"

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

//double MassiveArray::risInt = NaN;

//int MassiveArray::M = NaN;
//int MassiveArray::N = NaN;

std::map<int, double> MassiveArray::risValuesUcpa;
std::map<int, double> MassiveArray::risValuesUhpa;
std::map<MassiveArray::ConfigAntenna, double> MassiveArray::risValuesUrpa;
simsignal_t MassiveArray::MassiveArrayConfigureChange = registerSignal("MassiveArrayConfigureChange");


MassiveArray::MassiveArray() : AntennaBase()
{
}

void MassiveArray::initialize(int stage) {
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
#ifdef REGISTER_IN_NODE
		// this code register in the node module
		cModule *mod = getContainingNode(this);
		mod->subscribe(MassiveMIMOURPAConfigureChange, this);
#else
		// register in the interface module
		getParentModule()->getParentModule()->subscribe(MassiveArrayConfigureChange, this);
#endif
        // cout << "Posizione: " << getMobility()->getCurrentPosition() << endl;
        cModule *radioModule = getParentModule();
        IRadio * radio = check_and_cast<IRadio *>(radioModule);
        if (radio == nullptr)
            throw cRuntimeError("No IRadio module");
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receivedSignalPartChangedSignal, this);

    }
}

void MassiveArray::Simpson2D::initializeCoeff(Mat &coeff,int size) {
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


double MassiveArray::Simpson2D::Integral(double (*fun)(double,double, int, int), const Mat &coeff, limits xLimit, limits yLimit, int size, int M, int N) {

    double xInterval = xLimit.getUpper() - xLimit.getLower();
    double yInterval = yLimit.getUpper() - yLimit.getLower();
    double xStep = xInterval/(coeff.size()-1);
    double yStep = yInterval/(coeff.size()-1);
    double val = 0;

    double x = xLimit.getLower();
    for (unsigned int i = 0; i < coeff.size(); i++) {
        double y = yLimit.getLower();
        for (unsigned int j = 0; j < coeff.size(); j++) {
            val += fun(x,y, M, N)*coeff[i][j];
            y += yStep;
        }
        x += xStep;
    }

    return val*xStep*yStep/9;
}

double MassiveArray::Simpson2D::Integral(double (*fun)(double,double, int, int),limits xLimit, limits yLimits, int tam, int M, int N) {
    Mat coeff;
    initializeCoeff(coeff, tam);
    return Integral(fun,coeff,xLimit, yLimits, tam, M, N);
}



} // namespace physicallayer

} // namespace inet

