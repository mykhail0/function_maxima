#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <utility>
#include <memory>
#include <set>

template <typename A, typename V>
class point_type {
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
    point_type(const point_type &other) :
        arg_(make_shared(other.arg())), val_(make_shared(other.value())) {}

    // Exceptions safety? Is here a memory leak?
    point_type &operator=(const point_type &other) {
        arg_ = make_shared(other.arg());
        val_ = make_shared(other.value());
    }

  private:
    std::shared_ptr<A> arg_;
    std::shared_ptr<V> val_;

    friend point_type FunctionMaxima::make_point(
        std::shared_ptr<A>, std::shared_ptr<V>
    );

    explicit point_type(std::shared_ptr<A> arg, std::shared_ptr<V> val) :
        arg_(arg), val_(val) {};
};

template<typename A, typename V>
class FunctionMaxima {
  public:
    using iterator = typename std::multiset<point_type>::const_iterator;
    using mx_iterator = typename std::multiset<point_type>::const_iterator;
    using size_type = typename std::multiset<point_type>::size_type;

    FunctionMaxima() = default;
    FunctionMaxima(const FunctionMaxima &other) = default;
    ~FunctionMaxima() = default;

    FunctionMaxima &operator=(FunctionMaxima other) {
        other.swap(*this);
        return *this;
    }

    iterator begin() const { return imp->begin(); }
    iterator end() const { return imp->end(); }
    iterator find(A const &x) const { return imp->find(x); }
    mx_iterator mx_begin() const { return imp->mx_begin(); }
    mx_iterator mx_end() const { return imp->mx_end(); }
    size_type size() const { return imp->size(); }
    V const &value_at(const A &a) const { return imp->value_at(a); }
    void set_value(const A &a, const V &v) { imp->set_value(a, v); }
    void erase(const A &a) { imp->erase(a); }

  private:
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

    template <typename A, typename V>
    class MaximaImpl {
      public:
        MaximaImpl() = default;
        ~MaximaImpl() = default;

        MaximaImpl(const MaximaImpl &other) : points(other.points) {
            for (auto &pt : other.mx_points) {
                // fishy
                mx_points.insert(*points.find(pt));
            }
        }

        iterator find(A const &x) const {
            // Possible because A is guaranteed to have a copy constructor.
            std::shared_ptr<A> A_ptr = make_shared<A>(x);
            point_type pt = make_point(A_ptr, nullptr);
            return points.find(pt);
        }

        iterator begin() const { return points.begin(); }
        iterator end() const { return points.end(); }
        mx_iterator mx_begin() const { return mx_points.begin(); }
        mx_iterator mx_end() const { return  mx_points.end(); }
        size_type size() const { return points.size(); }

        V const &value_at(const A &a) const {
        }

        void set_value(const A &a, const V &v) {
        }

        void erase(const A &a) {
        }

      private:
        std::multiset<point_type, point_type_comparator_by_arg> points;
        std::multiset<point_type, point_type_comparator_by_value> mx_points;
    };

    std::unique_ptr<MaximaImpl<A, V>> imp;
};

#endif /* FUNCTION_MAXIMA_H */
