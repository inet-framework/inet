#include <cmath>

#include <gnplib/api/Simulator.h>
#include <gnplib/api/common/random/Rng.h>
#include <gnplib/impl/network/gnp/topology/GnpPosition.h>

using std::vector;

namespace Simulator=gnplib::api::Simulator;
namespace Rng=gnplib::api::common::random::Rng;
using gnplib::api::common::Host;
using gnplib::impl::network::gnp::topology::GnpPosition;

//const long GnpPosition::serialVersionUID(-1103996725403557900L);

/**
 *
 * @param gnpCoordinates
 *            coordinate array for new position
 */
GnpPosition::GnpPosition(const vector<double>& _gnpCoordinates)
: gnpCoordinates(_gnpCoordinates),
gnpRef(0),
hostRef(0),
error(-1.0) { }

/**
 * Object will be initialized with a random position. Position must be
 * random according to the downhill simplex
 *
 * @param noOfDimensions
 *            number of dimensions
 * @param hostRef
 *            related Host object
 * @param gnpRef
 *            related GnpSpace object
 */
GnpPosition::GnpPosition(int noOfDimensions, Host& _hostRef, const GnpSpace& _gnpRef)
: gnpCoordinates(noOfDimensions),
gnpRef(&_gnpRef),
hostRef(&_hostRef),
error(-1.0)
{
    for (size_t c=0; c<gnpCoordinates.size(); c++)
        gnpCoordinates[c]=Rng::dblrand();
}

#if 0

/**
 *
 * @param dimension
 * @param maxDiversity
 */
void GnpPosition::diversify(double[][] dimension, double maxDiversity)
{
    for (int c=0; c < this.gnpCoordinates.length; c++)
    {
        double rand=(2*maxDiversity*Math.random())-maxDiversity;
        gnpCoordinates[c]=gnpCoordinates[c] + (rand*dimension[c][2]);
    }
    error= -1.0;
}

/**
 * reposition
 *
 * @param pos
 *            position in the coordinate array
 * @param value
 *            new value at position pos
 */
void GnpPosition::setGnpCoordinates(int pos, double value)
{
    gnpCoordinates[pos]=value;
    error= -1.0;
}

/**
 * Calculates the sum of all errors according to the downhill simplex
 * operator.
 *
 * @return error
 */
double GnpPosition::getDownhillSimplexError()
{
    if (error<0.0)
    {
        error=0.0;
        for (int c=0; c<getGnpRef().getNumberOfMonitors(); c++)
        {
            error+=getDownhillSimplexError(getGnpRef()
                                           .getMonitorPosition(c));
        }
    }
    return error;
}

/**
 * Calculates the error to a monitor according to the downhill simplex
 * operator
 *
 * @param monitor
 * @return error
 */
double GnpPosition::getDownhillSimplexError(const GnpPosition& monitor)
{
    double calculatedDistance=this.getDistance(monitor);
    double measuredDistance=this.getMeasuredRtt(monitor);
    if (Double.compare(measuredDistance, Double.NaN)==0)
        return 0.0;
    double error=Math.pow((calculatedDistance-measuredDistance)/calculatedDistance, 2);
    return error;
}

/**
 * Calculates an error, that indicates the deviation of the measured vs. the
 * calculated rtt.
 *
 * @param monitor
 * @return error value
 */
double GnpPosition::getDirectionalRelativError(const GnpPosition& monitor)
{
    double calculatedDistance=this.getDistance(monitor);
    double measuredDistance=this.getMeasuredRtt(monitor);
    if (Double.compare(measuredDistance, Double.NaN)==0)
        return Double.NaN;
    double error=(calculatedDistance-measuredDistance)
            /Math.min(calculatedDistance, measuredDistance);
    return error;
}

/**
 * Method must be overwrite to sort different GnpPositions in order of their
 * quality.
 *
 * Is needed for the positioning with the downhill simplex
 *
 */
int GnpPosition::compareTo(const GnpPosition& arg0)
{
    double val1=this.getDownhillSimplexError();
    double val2=arg0.getDownhillSimplexError();
    if (val1<val2)
        return -1;
    if (val1>val2)
        return 1;
    else
        return 0;
}

/**
 *
 * @return Comma-separated list of coordinates
 */
const string& GnpPosition::getCoordinateString()
{
    if (gnpCoordinates.length==0)
    {
        return "";
    } else
    {
        String result=String.valueOf(gnpCoordinates[0]);
        for (int c=1; c<gnpCoordinates.length; c++)
            result=result+","+gnpCoordinates[1];
        return result;
    }
}

/**
 *
 * @param monitor
 * @return measured rtt to monitor, nan if no rtt was measured
 */
double GnpPosition::getMeasuredRtt(const GnpPosition& monitor)
{
    return this.getHostRef().getRtt(monitor.getHostRef());
}

#endif

/**
 * @return euclidean distance
 */
double GnpPosition::getDistance(const NetPosition& point) const
{
    const GnpPosition&coord(dynamic_cast<const GnpPosition&>(point));
    double distance=0.0;
    for (size_t c=0; c<gnpCoordinates.size(); c++)
        distance+=pow(gnpCoordinates[c]-coord.getGnpCoordinates(c), 2);
    return sqrt(distance);
}

#if 0

/**
 * Static method generates a new GnpPosition according to the downhill
 * simplex operator
 *
 * @param solution
 * @param moveToSolution
 * @param moveFactor
 * @return new position
 */

static GnpPosition GnpPosition::getMovedSolution(GnpPosition solution,
                                                 GnpPosition moveToSolution, double moveFactor)
{
    GnpPosition returnValue=new GnpPosition(solution.getNoOfDimensions(),
                                            solution.getHostRef(), solution.getGnpRef());
    for (int c=0; c<solution.getNoOfDimensions(); c++)
    {
        double newCoord=(moveToSolution.getGnpCoordinates(c)-solution
                         .getGnpCoordinates(c))
                *moveFactor+solution.getGnpCoordinates(c);
        returnValue.setGnpCoordinates(c, newCoord);
    }
    return returnValue;
}

/**
 * Static method generates a new GnpPosition according to the downhill
 * simplex operator
 *
 * @param solution
 * @param moveToSolution
 * @param moveFactor
 * @return new position
 */
const static GnpPosition& GnpPosition::getCenterSolution(const vector<GnpPosition>& solutions)
{
    GnpPosition returnValue=new GnpPosition(solutions.get(0)
                                            .getNoOfDimensions(), solutions.get(0).getHostRef(), solutions
                                            .get(0).getGnpRef());
    for (int d=0; d<solutions.size(); d++)
    {
        for (int c=0; c<solutions.get(0).getNoOfDimensions(); c++)
        {
            returnValue.setGnpCoordinates(c, returnValue
                                          .getGnpCoordinates(c)
                                          +solutions.get(d).getGnpCoordinates(c));
        }
    }
    for (int c=0; c<returnValue.getNoOfDimensions(); c++)
    {
        returnValue.setGnpCoordinates(c, returnValue.getGnpCoordinates(c)
                                      /solutions.size());
    }
    return returnValue;
}

#endif
