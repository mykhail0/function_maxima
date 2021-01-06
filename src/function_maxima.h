#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <iostream>
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

        point_type &operator=(const point_type &other) {
            // We use copy constructors of A and V.
            arg_ = std::make_shared<A>(other.arg());
            val_ = std::make_shared<V>(other.value());
        }

        point_type(const point_type &other) = default;

    private:
        std::shared_ptr<A> arg_;
        std::shared_ptr<V> val_;

        friend point_type FunctionMaxima::make_point(
                const std::shared_ptr<A> &, const std::shared_ptr<V> &
        );

        explicit point_type(const std::shared_ptr<A> &arg,
                            const std::shared_ptr<V> &val)
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

    FunctionMaxima &operator=(FunctionMaxima other) {
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

    V const &value_at(const A &a) const { return imp->value_at(a); }

    void set_value(const A &a, const V &v) { imp->set_value(a, v); }
//    void erase(const A &a) { imp->erase(a); }


private:

    // Exposed point_type constructor
    static point_type
    make_point(const std::shared_ptr<A> &arg, const std::shared_ptr<V> &value) {
        return point_type(arg, value);
    }

    static point_type
    make_point(const A &arg, const V &value) {
        return make_point(std::make_shared<A>(arg), std::make_shared<V>(value));
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

        iterator find(A const &a) const {
            return find_in_multiset(a, points);
        }

        iterator begin() const { return points.begin(); }

        iterator end() const { return points.end(); }

        mx_iterator mx_begin() const { return mx_points.begin(); }

        mx_iterator mx_end() const { return mx_points.end(); }

        size_type size() const { return points.size(); }

        const V &value_at(const A &a) const {
            iterator it = find(a);
            if (it == end())
                throw invalid_exception;
            return *it;
        }

        void set_value(const A &a, const V &v) {
            iterator previous = find(a);
            InsertGuard <point_type_comparator_by_arg> currentGuard
                    (make_point(a, v), points);
            iterator current = currentGuard.it;

            iterator neighbours[] = {left(current),
                                     current,
                                     right(current)};

            std::unique_ptr<Guard> updateGuards[] =
                    {mark_as_maximum(neighbours[0]),
                     mark_as_maximum(neighbours[1]),
                     mark_as_maximum(neighbours[2])};

            // From this point, the function will not throw an exception

            for (iterator neighbour : neighbours)
                unmark_as_maximum(neighbour);

            currentGuard.commit();
            for (auto &guard : updateGuards)
                guard->commit();

            if (previous != end())
                points.erase(previous);

            std::cout << is_a_local_maximum(current) << std::endl;
        }

        void erase(const A &a) {
        }

    private:

        template<typename comparator>
        static iterator find_in_multiset(
                const A &a,
                const std::multiset<point_type, comparator> &multiset) {
            // TODO avoid copying
            std::shared_ptr<A> A_ptr = std::make_shared<A>(a);
            point_type pt = make_point(A_ptr, nullptr);
            return multiset.find(pt);
        }

        class Guard {
        protected:
            bool done;

            Guard() : done(false) {}

        public:
            void commit() {
                done = true;
            }
        };

        template<typename comparator>
        class InsertGuard : public Guard {
        public:

            iterator it;
            std::multiset<point_type, comparator> *multiset;

            ~InsertGuard() noexcept {
                if (!Guard::done) {
                    multiset->erase(it);
                }
            }

            InsertGuard(const point_type &point,
                        std::multiset<point_type, comparator> &multiset) :
                    Guard() {
                it = multiset.insert(point);
                this->multiset = &multiset;
            }

            InsertGuard(const InsertGuard &other) = default;
        };

        class EmptyGuard : public Guard {
        };

        bool is_a_local_maximum(iterator it) {
            // TODO multiple values for argument
            return (left(it) == end() || left(it)->value() < it->value())
                   && (right(it) == end() || right(it)->value() < it->value());
        }

        std::unique_ptr<Guard> mark_as_maximum(iterator it) {
            if (it == points.end())
                return std::unique_ptr<EmptyGuard>();
            auto it_mx = find_in_multiset(it->arg(), mx_points);

            bool was_a_local_maximum = (it_mx != mx_points.end());

            if (!was_a_local_maximum && is_a_local_maximum(it))
                return std::make_shared<
                        InsertGuard < point_type_comparator_by_value>>
            (
                    *it, mx_points);
            return std::unique_ptr<EmptyGuard>();
        }

//
//  f(1) = 1                          MX                       MX
//  f(2) = 2,-1 MX -> mark_as_maximum MX -> unmark_as_maximum(noexcept)
//  f(3) = 1                          MX                       MX

        void unmark_as_maximum(iterator it) noexcept {
            if (it == points.end())
                return;
            auto it_mx = find_in_multiset(it->arg(), mx_points);

            bool was_a_local_maximum = (it_mx != mx_points.end());

            if (was_a_local_maximum && !is_a_local_maximum(it))
                mx_points.erase(it_mx);
        }

        iterator left(const iterator &start) {
            iterator it = points.lower_bound(*start);
            if (it != begin())
                return --it;
            else
                return end();
        }

        iterator right(const iterator &start) {
            return points.upper_bound(*start);
        }


        // First compares by argument, if equal compares by value
        class point_type_comparator_by_arg {
        public:

            bool operator()(const point_type &p1, const point_type &p2) const {
                return p1.arg() < p2.arg();
            }
        };

        // First compares by value, if equal compares by argument.
        class point_type_comparator_by_value {
        public:
            template<typename> friend
            class InsertGuard;

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
