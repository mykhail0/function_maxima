#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <memory>
#include <set>

template<typename A, typename V>
class FunctionMaxima {
  public:
    class point_type {
    public:
        // Returns function argument.
        A const &arg() const noexcept { return *arg_; }

        // Returns function value at a given point.
        V const &value() const noexcept { return *val_; }

        point_type &operator=(const point_type &other) noexcept {
            // We use copy constructors of A and V.
            arg_ = std::make_shared<A>(other.arg());
            val_ = std::make_shared<V>(other.value());
        }

        point_type(const point_type &other) = default;

    private:
        std::shared_ptr<A> arg_;
        std::shared_ptr<V> val_;

        friend point_type FunctionMaxima::make_point(
                std::shared_ptr<A>, std::shared_ptr<V>
        );

        explicit point_type(std::shared_ptr<A> arg, std::shared_ptr<V> val)
        noexcept:
                arg_(arg), val_(val) {};
    };

    class InvalidArg : public std::exception {
        virtual const char *what() const throw() {
            return "Argument is not in the domain.";
        }
    };

    using iterator = typename std::multiset<point_type>::const_iterator;
    using mx_iterator = typename std::multiset<point_type>::const_iterator;
    using size_type = typename std::multiset<point_type>::size_type;

    FunctionMaxima() {
        imp = std::make_unique<MaximaImpl>();
    }

    FunctionMaxima(const FunctionMaxima &other) {
        // We use copy constructor of MaximaImpl
        imp = std::make_unique<MaximaImpl>(*other.imp);
    }

    ~FunctionMaxima() = default;

    FunctionMaxima &operator=(FunctionMaxima other) noexcept {
        other.swap(*this);
        return *this;
    }

    void swap(FunctionMaxima &other) noexcept {
        std::swap(this->imp, other.imp);
    }

    iterator begin() const { return imp->begin(); }
    iterator end() const { return imp->end(); }
    iterator find(A const &x) const { return imp->find(x); }
    mx_iterator mx_begin() const { return imp->mx_begin(); }
    mx_iterator mx_end() const { return imp->mx_end(); }
    size_type size() const { return imp->size(); }
/*
    V const &value_at(const A &a) const { return imp->value_at(a); }
    void set_value(const A &a, const V &v) { imp->set_value(a, v); }
    void erase(const A &a) { imp->erase(a); }
*/

  private:

    // First compares by value, if equal compares by argument.
    class point_type_comparator_by_value {
        bool operator()(const point_type &p1, const point_type &p2) {
            return (!(p1.value() < p2.value()) && !(p2.value() < p1.value())) ?
                    p1.arg() < p2.arg() :
                    p1.value() < p2.value();
        }
    };

    static point_type
    make_point(std::shared_ptr<A> arg, std::shared_ptr<V> value) {
        return point_type(arg, value);
    }

    class MaximaImpl {
      public:
        MaximaImpl() = default;
        ~MaximaImpl() = default;

        MaximaImpl(const MaximaImpl &other) : points(other.points) {
            for (auto &pt : other.mx_points) {
                mx_points.insert(*points.find(pt));
            }
        }

        iterator find(A const &x) const {
            // Possible because A is guaranteed to have a copy constructor.
            std::shared_ptr<A> A_ptr = std::make_shared<A>(x);
            point_type pt = make_point(A_ptr, nullptr);
            return points.find(pt);
        }

        iterator begin() const { return points.begin(); }
        iterator end() const { return points.end(); }
        mx_iterator mx_begin() const { return mx_points.begin(); }
        mx_iterator mx_end() const { return  mx_points.end(); }
        size_type size() const { return points.size(); }

        const V &value_at(const A &a) const {
            iterator it = find(a);
            if (it == end())
                throw invalid_exception;
            return *it;
        }

        void set_value(const A &a, const V &v) {
        }

        void erase(const A &a) {
        }

    private:

        class point_type_comparator_by_arg {
        public:
            bool operator()(const point_type &p1, const point_type &p2) const {
                return p1.arg() < p2.arg();
            }
        };

        // First compares by value, if equal compares by argument.
        class point_type_comparator_by_value {
        public:
            bool operator()(const point_type &p1, const point_type &p2) const {
                return (!(p1.value() < p2.value()) &&
                        !(p2.value() < p1.value())) ?
                       p1.arg() < p2.arg() :
                       p1.value() < p2.value();
            }
        };

        InvalidArg invalid_exception;
        std::multiset<point_type, point_type_comparator_by_arg> points;
        std::multiset<point_type, point_type_comparator_by_value> mx_points;
    };

    std::unique_ptr<MaximaImpl> imp;
};

#endif /* FUNCTION_MAXIMA_H */
