#ifndef VIBELLO_UTIL_COUNTINGOUTPUTITERATOR_H
#define	VIBELLO_UTIL_COUNTINGOUTPUTITERATOR_H

#include <cstddef>

namespace vibello {
namespace util {

/**
 * An OutputIterator that discards input but counts the times it is incremented.
 */

class CountingOutputIterator
{
private:
    struct Void
    {
        template<typename T>
                Void&operator=(T) { }
    };

    size_t cnt;

public:
    inline CountingOutputIterator()
    : cnt(0) {}

    inline CountingOutputIterator&operator++()
    {
        ++cnt;
    }

    inline Void operator*()
    {
        return Void();
    }

    inline operator size_t() const
    {
        return cnt;
    }

};

}
}

#endif	/* VIBELLO_UTIL_COUNTINGOUTPUTITERATOR_H */
