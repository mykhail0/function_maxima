#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <utility>
#include <memory>
#include <set>

using namespace std;

template<typename A, typename V>
class FunctionMaxima {
private:
    using Point = pair<A, V>;

    class point_type {
    private:
        shared_ptr<Point> ptr;

        friend point_type FunctionMaxima::make_point(A, V);

        explicit point_type(Point *ptr) : ptr(ptr) {};

    public:
        // Zwraca argument funkcji.
        A const &arg() const {
            return ptr.get()->first;
        }

        // Zwraca wartość funkcji w tym punkcie.
        V const &value() const {
            return ptr.get()->second;
        }
    };

    class point_type_comparator_by_arg {
        bool operator()(const point_type &p1, const point_type &p2) {
            return p1.arg() < p2.arg();
        }
    };

    class point_type_comparator_by_value {
        bool operator()(const point_type &p1, const point_type &p2) {
            return p1.value() < p2.value();
        }
    };

    point_type make_point(A arg, V value) {
        return point_type(new Point(arg, value));
    }

    set<point_type, point_type_comparator_by_arg> points;
    set<point_type, point_type_comparator_by_value> mx_points;

public:
    using iterator = typename set<point_type>::const_iterator;
    using mx_iterator = typename set<point_type>::const_iterator;
    using size_type = typename set<point_type>::size_type;

    FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima &);

    FunctionMaxima &operator=(const FunctionMaxima &);

    ~FunctionMaxima() = default;

    iterator begin() const {
        return points.begin();
    }

    iterator end() const {
        return points.end();
    }

    iterator find(A const &x) const {
        // TODO strong exception
        unique_ptr<V> dummy(new V);
        point_type pt = make_point(x, *dummy);
        return points.find(pt);
    }

    mx_iterator mx_begin() const {
        return mx_points.begin();
    }
  
  	mx_iterator mx_end() const {
     return  mx_points.end();
    }

    //set value mutator

    //erase mutator

    //value at inspector

    //size()
};


#endif /* FUNCTION_MAXIMA_H */