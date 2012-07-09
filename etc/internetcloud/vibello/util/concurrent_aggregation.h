#ifndef VIBELLO_UTIL_CONCURRENT_AGGREGATION_H
#define	VIBELLO_UTIL_CONCURRENT_AGGREGATION_H

#include <boost/thread/mutex.hpp>

namespace vibello {
namespace util {

template<typename T>
class Max
{
public:
    inline Max(const T& initial) : val(initial) {}
    inline Max(const Max& other) : val(other)   {}

    inline Max&operator=(const Max& other)
    {
        val=other;
    }

    inline void update(const T& v)
    {
        boost::mutex::scoped_lock scoped_lock(mutex);
        val=std::max(val, v); // TODO: use read lock to speed up
    }

    inline operator T() const
    {
        boost::mutex::scoped_lock scoped_lock(mutex);
        return val;
    }

private:
    mutable boost::mutex mutex;
    T val;
};

template<typename T>
class Sum
{
public:
    inline Sum() : val(0)                       {}
    inline Sum(const T& initial) : val(initial) {}
    inline Sum(const Sum& other) : val(other)   {}

    inline Sum&operator=(const Sum& other)
    {
        val=other;
    }

    inline void update(const T& v)
    {
        boost::mutex::scoped_lock scoped_lock(mutex);
        val+=v;
    }

    inline Sum&operator+=(const T& v)
    {
        boost::mutex::scoped_lock scoped_lock(mutex);
        val+=v;
    }

    inline operator T() const
    {
        boost::mutex::scoped_lock scoped_lock(mutex);
        return val;
    }

private:
    mutable boost::mutex mutex;
    T val;
};

}
}

#endif	/* VIBELLO_UTIL_CONCURRENT_AGGREGATION_H */
