///
/// @file   resultrecorders.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Oct/20/2011
///
/// @brief  Provides additional result recorders which are not currently implemented in OMNeT++.
///
/// @remarks Copyright (C) 2011 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

#include <algorithm>
#include <math.h>
#include <vector>
#include <sstream>
#include <omnetpp.h>
#ifdef __linux__
#include <expression.h>
#endif

using namespace std;

template <class T>
inline std::string to_string (const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

///
/// Listener for recording the 90th, 95th, and 99th percentiles of
/// signal values [1]
///
/// @note
/// The current implementation corresponds to "type 6" of R's
/// quantile() function.
///
/// @par References:
/// <ol>
/// <li> "Engineering Statistics Handbook: 7.2.6.2 Percentiles"
/// NIST. Retrieved 2010-06-23. [Online]. Available:
/// http://www.itl.nist.gov/div898/handbook/prc/section2/prc262.htm
/// </li>
/// </ol>
///
class SIM_API PercentileRecorder : public cNumericResultRecorder
{
protected:
    enum 
    {
        NUMBER_OF_PERCENTAGES = 3 
    };
    static const double percentage[NUMBER_OF_PERCENTAGES];
    vector<double> signalVector;    // container of signal values
protected:
    virtual void collect(simtime_t_cref t, double value);
public:
    virtual void finish(cResultFilter *prev);
};

Register_ResultRecorder("percentile", PercentileRecorder);

// initialize percentage values
const double PercentileRecorder::percentage[NUMBER_OF_PERCENTAGES]
    = {90.0, 95.0, 99.0};

void PercentileRecorder::collect(simtime_t_cref t, double value)
{
    signalVector.push_back(value);
}

void PercentileRecorder::finish(cResultFilter *prev)
{
    int N = signalVector.size();
    bool empty = (N == 0);

    if (empty == false)
    {
        sort(signalVector.begin(), signalVector.end()); // sort the signal values

        // calculate percentiles for given percentage numbers
        double percentile[NUMBER_OF_PERCENTAGES];
        for (int i = 0; i < NUMBER_OF_PERCENTAGES; i++)
        {
            int k;
            double d, tmp;

            d = modf(percentage[i] * (N + 1) / 100, &tmp);
            // tmp and d are the integral and the fractional parts of
            // "percentage * (N + 1) / 100", respectively.
            k = int(tmp);
            if (k == 0)
                percentile[i] = signalVector[0];
            else if (k >= N)
                percentile[i] = signalVector[N - 1];
            else
                percentile[i] = signalVector[k - 1] + d * (signalVector[k] - signalVector[k - 1]);
        }

        // record the results
        // FIXME revise them later to use recordStatistic() as follows:
        // - opp_string_map attributes = getStatisticAttributes();
        // - ev.recordScalar(getComponent(), getResultName().c_str(), empty ? NaN : p, &attributes);
        for (int i = 0; i < NUMBER_OF_PERCENTAGES; i++)
        {
            string percentage_name = to_string<int>(int(percentage[i])) + "th-";
            opp_string_map attributes = getStatisticAttributes();
            // FIXME remove macro processing here once the library issues with windows platfom have been solved
#ifdef __linux__
            ev.recordScalar(getComponent(), (percentage_name + getResultName()).c_str(), empty ? NaN : percentile[i], &attributes);
#else
            ev.recordScalar(getComponent(), (percentage_name + getResultName()).c_str(), percentile[i], &attributes);
#endif
        }
    }
    // FIXME Provide better processing for the case of zero-size signalVector
}
