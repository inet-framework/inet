#ifndef __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_GNPPOSITION_H
#define __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_GNPPOSITION_H

#include <vector>
#include <string>
#include <gnplib/api/network/NetPosition.h>

namespace gnplib { namespace api { namespace common {
    class Host;
} }

namespace impl { namespace network { namespace gnp {

class GnpSpace;

namespace topology {

/**
 * This class implements a NetPosition for a GNP-Based calculation of round trip
 * times.
 * Therefore it includes methods for error estimation and methods for positioning
 * by a downhill simplex algorithm in the GnpSpace class
 * 
 * @author Gerald Klunker
 * @author ported to C++ by Philipp Berndt <philipp.berndt@tu-berlin.de>
 * @version 0.1, 09.01.2008
 * 
 */
class GnpPosition : public api::network::NetPosition
{
    // const static long serialVersionUID;

    std::vector<double> gnpCoordinates;
    const GnpSpace* gnpRef;
    api::common::Host* hostRef;
    double error;

    /**
     *
     * @param gnpCoordinates
     *            coordinate array for new position
     */
public:
    GnpPosition(const std::vector<double>& gnpCoordinates);

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
    GnpPosition(int noOfDimensions, api::common::Host& hostRef, const GnpSpace& gnpRef);

    /**
     *
     * @param dimension
     * @param maxDiversity
     */
    //void diversify(double[][] dimension, double maxDiversity);

    /**
     * reposition
     *
     * @param pos
     *            position in the coordinate array
     * @param value
     *            new value at position pos
     */
    void setGnpCoordinates(int pos, double value);

    /**
     *
     * @return the related GnpSpace object
     */
private:

    inline const GnpSpace& getGnpRef() const
    {
        return *gnpRef;
    }

    /**
     *
     * @return the related Host object
     */
public:

    inline api::common::Host& getHostRef()
    {
        return *hostRef;
    }

    /**
     *
     * @return number of dimensions
     */
    inline int getNoOfDimensions()
    {
        return gnpCoordinates.size();
    }

    /**
     *
     * @param pos
     *            position in the coordinate array
     * @return value at position pos
     */
    inline double getGnpCoordinates(int pos) const
    {
        return gnpCoordinates[pos];
    }

    /**
     * Calculates the sum of all errors according to the downhill simplex
     * operator.
     *
     * @return error
     */
    double getDownhillSimplexError();

    /**
     * Calculates the error to a monitor according to the downhill simplex
     * operator
     *
     * @param monitor
     * @return error
     */
    double getDownhillSimplexError(const GnpPosition& monitor);

    /**
     * Calculates an error, that indicates the deviation of the measured vs. the
     * calculated rtt.
     *
     * @param monitor
     * @return error value
     */
    double getDirectionalRelativError(const GnpPosition& monitor);

    /**
     * Method must be overwrite to sort different GnpPositions in order of their
     * quality.
     *
     * Is needed for the positioning with the downhill simplex
     *
     */
    int compareTo(const GnpPosition& arg0);

    /**
     *
     * @return Comma-separated list of coordinates
     */
    const std::string& getCoordinateString();

    /**
     *
     * @param monitor
     * @return measured rtt to monitor, nan if no rtt was measured
     */
    double getMeasuredRtt(const GnpPosition& monitor);

    /**
     * @return euclidean distance
     */
    // @Overrride
    double getDistance(const NetPosition& point) const;

    /**
     * Static method generates a new GnpPosition according to the downhill
     * simplex operator
     *
     * @param solution
     * @param moveToSolution
     * @param moveFactor
     * @return new position
     */
    static GnpPosition getMovedSolution(GnpPosition solution,
                                        GnpPosition moveToSolution, double moveFactor);

    /**
     * Static method generates a new GnpPosition according to the downhill
     * simplex operator
     *
     * @param solution
     * @param moveToSolution
     * @param moveFactor
     * @return new position
     */
    const static GnpPosition& getCenterSolution(const std::vector<GnpPosition>& solutions);

};

} } } } } // namespace gnplib::impl::network::gnp::topology

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_GNPPOSITION_H
