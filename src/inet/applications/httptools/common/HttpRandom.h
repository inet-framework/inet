//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_HTTPRANDOM_H
#define __INET_HTTPRANDOM_H

#include <exception>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/applications/httptools/common/HttpUtils.h"
#include "inet/common/INETMath.h"

namespace inet {

namespace httptools {

enum DistrType { dt_normal, dt_uniform, dt_exponential, dt_histogram, dt_constant, dt_zipf };

// Defines for the distribution names
#define DISTR_NORMAL_STR         "normal"
#define DISTR_UNIFORM_STR        "uniform"
#define DISTR_EXPONENTIAL_STR    "exponential"
#define DISTR_HISTOGRAM_STR      "histogram"
#define DISTR_CONSTANT_STR       "constant"
#define DISTR_ZIPF_STR           "zipf"

/**
 * Base random object. Should not be instantiated directly.
 */
class INET_API rdObject
{
  protected:
    DistrType m_type = dt_normal;

  protected:
    bool _hasKey(cXMLAttributeMap attributes, std::string key) { return attributes.find(key) != attributes.end(); }

  public:
    virtual ~rdObject() {}
    DistrType getType() { return m_type; }
    std::string typeStr();
    virtual std::string toString() { return typeStr(); }

    /*
     * Returns a random number. Must be implemented in derived classes.
     */
    virtual double draw() = 0;
};

/**
 * Normal distribution random object.
 * Wraps the OMNeT++ normal distribution function but adds a minimum limit.
 */
class INET_API rdNormal : public rdObject
{
  protected:
    double m_mean = NaN;    ///< The mean of the distribution.
    double m_sd = NaN;    ///< The sd of the distribution.
    double m_min = NaN;    ///< The minimum limit   .
    bool m_bMinLimit = NaN;    ///< Set if the minimum limit is set.
    bool m_nonNegative = NaN;    ///< Non-negative only -- uses the truncnormal function.

  public:

    /*
     * Constructor for direct initialization
     */
    rdNormal(double mean, double sd, bool nonNegative = false);

    /*
     * Constructor for initialization with an XML element
     */
    rdNormal(cXMLAttributeMap attributes);

    /*
     * Set the min limit for the random values
     */
    void setMinLimit(double min) { m_min = min; m_bMinLimit = true; }

    /*
     * Cancel the min limit when not needed any more
     */
    void resetMinLimit() { m_bMinLimit = false; }

    /*
     * Get a random value
     */
    virtual double draw() override;
};

/**
 * Uniform distribution random object.
 * Wraps the OMNeT++ uniform distribution function.
 */
class INET_API rdUniform : public rdObject
{
  protected:
    double m_beginning = NaN;    ///< Low limit
    double m_end = NaN;    ///< High limit

  public:
    /*
     * Constructor for direct initialization
     */
    rdUniform(double beginning, double end);

    /*
     * Constructor for initialization with an XML element
     */
    rdUniform(cXMLAttributeMap attributes);

    /*
     * Get a random value
     */
    virtual double draw() override;

    // Getters and setters
    double getBeginning() { return m_beginning; }
    void setBeginning(double beginning) { m_beginning = beginning; }
    double getEnd() { return m_end; }
    void setEnd(double end) { m_end = end; }
};

/**
 * Exponential distribution random object.
 * Wraps the OMNeT++ exponential distribution function, but adds min and max limits.
 */
class INET_API rdExponential : public rdObject
{
  protected:
    double m_mean = NaN;    // the distribution mean
    double m_min = NaN;    // the low limit
    double m_max = NaN;    // the high limit
    bool m_bMinLimit = false;
    bool m_bMaxLimit = false;

  public:

    /*
     * Constructor for direct initialization
     */
    rdExponential(double mean);

    /*
     *  Constructor for initialization with an XML element
     */
    rdExponential(cXMLAttributeMap attributes);

    /*
     * Get a random value
     */
    virtual double draw() override;

    // Getters and setters
    void setMinLimit(double min) { m_min = min; m_bMinLimit = true; }
    void resetMinLimit() { m_bMinLimit = false; }
    void setMaxLimit(double max) { m_max = max; m_bMaxLimit = true; }
    void resetMaxLimit() { m_bMaxLimit = false; }
};

/**
 * Histogram distribution random object.
 */
class INET_API rdHistogram : public rdObject
{
  protected:
    struct rdHistogramBin
    {
        int count = 0;
        double sum = NaN;
    };

    typedef std::vector<rdHistogramBin> rdHistogramBins;

    rdHistogramBins m_bins;
    bool m_zeroBased = false;

  private:
    void __parseBinString(std::string binstr);
    void __normalizeBins();

  public:
    /*
     * Constructor for direct initialization
     */
    rdHistogram(rdHistogramBins bins, bool zeroBased = false);

    /*
     * Constructor for initialization with an XML element
     */
    rdHistogram(cXMLAttributeMap attributes);

    /*
     * Get a random value
     */
    double draw() override;
};

/**
 * Constant distribution random object.
 * Not really a random object, but used to allow constants to be assigned in stead of random distributions
 * when initializing parameters.
 */
class INET_API rdConstant : public rdObject
{
  protected:
    double m_value = NaN;    ///< The constant

  public:

    /*
     * Constructor for direct initialization
     */
    rdConstant(double value);

    /*
     * Constructor for initialization with an XML element
     */
    rdConstant(cXMLAttributeMap attributes);

    /*
     * Get a random value
     */
    double draw() override;
};

/**
 * Zipf distribution random object.
 * Returns a random value from a zipf distribution (1/n^a), where a is the constant alpha and n is a order of popularity.
 * See more details on http://en.wikipedia.org/wiki/Zipf.
 */
class INET_API rdZipf : public rdObject
{
  protected:
    double m_alpha = NaN;    // the alpha value
    int m_number = 0;    // the number of nodes to pick from
    double m_c = NaN;    // helper constant.
    bool m_baseZero = false;    // true if we want a zero-based return value

  private:
    // Initialization methods.
    void __initialize(int n, double alpha, bool baseZero);
    void __setup_c();

  public:

    /*
     * Constructor for direct initialization *
     */
    rdZipf(int n, double alpha, bool baseZero = false);

    /*
     * Constructor for initialization with an XML element
     */
    rdZipf(cXMLAttributeMap attributes);

    /*
     * Get a random value -- a element in the pick order (popularity order)
     */
    virtual double draw() override;

    /*
     *  Return the object definition as a string
     */
    virtual std::string toString() override;

    // Getters and setters
    void setN(int n) { m_number = n; __setup_c(); }
    int getN() { return m_number; }
    void setAlpha(double alpha) { m_alpha = alpha; __setup_c(); }
    double getAlpha() { return m_alpha; }
};

/**
 * A factory class used to construct random distribution objects based on XML elements.
 * The type name is used to instantiate the appropriate rdObject-derived class.
 */
class INET_API rdObjectFactory
{
  public:
    /*
     * Return a rdObject-derived class based on the type name in the XML element
     */
    rdObject *create(cXMLAttributeMap attributes);
};

} // namespace httptools

} // namespace inet

#endif // ifndef __INET_HTTPRANDOM_H

