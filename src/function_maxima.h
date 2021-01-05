#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <utility>
#include <memory>
#include <set>

template<typename A, typename V>
class FunctionMaxima {
  private:
    //using Point = std::pair<A, V>;

    class point_type {
      private:
        std::shared_ptr<A> arg_;
        std::shared_ptr<V> val_;

        friend point_type FunctionMaxima::make_point(std::shared_ptr<A>, std::shared_ptr<V>);

        explicit point_type(std::shared_ptr<A> arg, std::shared_ptr<V> val) :
            arg_(arg), val_(val) {};

      public:
        // Returns function argument.
        A const &arg() const {
            return ptr.get()->first;
        }

        // Returns function value at a given point.
        V const &value() const {
            return ptr.get()->second;
        }

        // Can throw exception, is this ok then?
        point_type(const point_type &other) : arg_(make_shared(other.arg())), val_(make_shared(other.value())) {}

        // Exceptions safety? Is here a memory leak?
        point_type &operator=(const point_type &other) {
            arg_ = make_shared(other.arg());
            val_ = make_shared(other.value());
        }
    };

    class point_type_comparator_by_arg {
        bool operator()(const point_type &p1, const point_type &p2) {
            return p1.arg() < p2.arg();
        }
    };

    // First compares by value, if equal compares by argument.
    class point_type_comparator_by_value {
        bool operator()(const point_type &p1, const point_type &p2) {
            return (!(p1.value() < p2.value()) && !(p2.value() < p1.value())) ?
                    p1.arg() < p2.arg() :
                    p1.value() < p2.value();
        }
    };

    point_type make_point(std::shared_ptr<A> arg, std::shared_ptr<V> value) {
        return point_type(arg, value);
    }

    std::multiset<point_type, point_type_comparator_by_arg> points;
    std::multiset<point_type, point_type_comparator_by_value> mx_points;

  public:
    using iterator = typename std::multiset<point_type>::const_iterator;
    using mx_iterator = typename std::multiset<point_type>::const_iterator;
    using size_type = typename std::multiset<point_type>::size_type;

    FunctionMaxima() = default;
    ~FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima &other) : points(other.points) {
        for (auto &pt : other.mx_points) {
            // fishy
            mx_points.insert(*points.find(pt));
        }
    }

    FunctionMaxima &operator=(FunctionMaxima other) {
        // TODO pimpl, to create nothrow swap
        other.swap(*this);
        return *this;
    }

    iterator begin() const {
        return points.begin();
    }

    iterator end() const {
        return points.end();
    }

    iterator find(A const &x) const {
        // Possible because A is guaranteed to have a copy constructor.
        std::shared_ptr<A> A_ptr = make_shared<A>(x);
        point_type pt = make_point(A_ptr, nullptr);
        return points.find(pt);
    }

    mx_iterator mx_begin() const {
        return mx_points.begin();
    }
  
    mx_iterator mx_end() const {
        return  mx_points.end();
    }

    size_type size() const {
        return points.size();
    }

    // set_value mutator

    // erase mutator

    // value_at inspector
};

#endif /* FUNCTION_MAXIMA_H */
