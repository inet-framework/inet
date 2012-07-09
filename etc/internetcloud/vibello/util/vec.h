#ifndef VIBELLO_CONCURRENT_AGGREGATION_H
#define	VIBELLO_CONCURRENT_AGGREGATION_H

#include <cmath>
#include <iostream>

namespace vibello {
namespace util {

template<typename Elem_t, int dim>
class Vec
{
    Elem_t e[dim];

public:
    typedef Elem_t value_t;

    enum
    {
        DIM=dim
    };

    Elem_t&operator[](size_t i)
    {
        return e[i];
    }

    const Elem_t&operator[](size_t i) const
    {
        return e[i];
    }

    Vec()
    {
        for (int i=0; i<dim; ++i)
            e[i]=0;
    }

    Vec(const Vec& v)
    {
        (*this)=v;
    }

    Vec&operator=(const Vec& v)
    {
        for (int i=0; i<dim; ++i)
            e[i]=v[i];
    }

    Vec&operator+=(const Vec& x)
    {
        for (int i=0; i<dim; ++i)
            e[i]+=x[i];
        return *this;
    }

    Vec&operator/=(Elem_t x)
    {
        for (int i=0; i<dim; ++i)
            e[i]/=x;
        return *this;
    }

    void print(std::ostream& os) const
    {
        os<<"(";
        int i=0;
        for (int i=0;; ++i)
        {
            os<<e[i];
            if (i==dim-1)
                break;
            os<<", ";
        }
        os<<")";
    }
};

template<typename Vec>
Vec operator-(const Vec& v0, const Vec& v1)
{
    Vec res;
    for (int i=0; i<Vec::DIM; ++i)
        res[i]=v0[i]-v1[i];
    return res;
}

template<typename Vec>
Vec operator*(typename Vec::value_t s, const Vec& v)
{
    Vec res;
    for (int i=0; i<Vec::DIM; ++i)
        res[i]=s*v[i];
    return res;
}

template<typename Vec>
typename Vec::value_t abs(const Vec& v)
{
    typename Vec::value_t res(0);
    for (int i=0; i<Vec::DIM; ++i)
        res+=v[i]*v[i];
    return sqrt(res);
}

template<typename Vec>
Vec u(const Vec& v)
{
    Vec res(v);
    return res/=abs(v);
}

template<typename Elem_t, int dim>
std::ostream&operator<<(std::ostream& os, const Vec<Elem_t, dim>& v)
{
    v.print(os);
    return os;
}

} // namespace util
} // namespace vibello

#endif // not defined VIBELLO_CONCURRENT_AGGREGATION_H
