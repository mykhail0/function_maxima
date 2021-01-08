#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <iostream>
#include <memory>
#include <set>

class InvalidArg : public std::exception {
public:
    virtual const char *what() const throw() {
        return "Argument is not in the domain.";
    }
};

template<typename A, typename V>
class FunctionMaxima {
public:
    class point_type;

    using iterator = typename std::multiset<point_type>::const_iterator;
    using mx_iterator = typename std::multiset<point_type>::const_iterator;
    using size_type = typename std::multiset<point_type>::size_type;

    FunctionMaxima() { imp = std::make_unique<MaximaImpl>(); }
    FunctionMaxima(const FunctionMaxima &);
    ~FunctionMaxima() = default;
    FunctionMaxima &operator=(FunctionMaxima) noexcept;

    iterator begin() const { return imp->begin(); }
    iterator end() const { return imp->end(); }
    iterator find(A const &x) const { return imp->find(x); }

    mx_iterator mx_begin() const { return imp->mx_begin(); }
    mx_iterator mx_end() const { return imp->mx_end(); }

    size_type size() const { return imp->size(); }

    V const &value_at(const A &a) const { return imp->value_at(a); }
    void set_value(const A &a, const V &v) { imp->set_value(a, v); }
    void erase(const A &a) { imp->erase(a); }

    void debug() {
        std::cout << "POINTS:" << std::endl;
        for(auto p : *this)
            std::cout << p.arg() << " -> " << p.value() << std::endl;

        std::cout << "MX_POINTS:" << std::endl;
        for(auto it = mx_begin(); it != mx_end(); it++){
            auto p = *it;
            std::cout << p.arg() << " -> " << p.value() << std::endl;
        }

        std::cout << "-----------------------------------" << std::endl;
    }

private:

    void swap(FunctionMaxima &) noexcept;

    // Exposed point_type constructor.
    static point_type
    make_point(const std::shared_ptr<A> &, const std::shared_ptr<V> &);

    // Exposed point_type constructor.
    static point_type
    make_point(const A &, const V &);

    class MaximaImpl;

    std::unique_ptr<MaximaImpl> imp;
};

/*
 * FunctionMaxima definitions.
 */
template <typename A, typename V>
FunctionMaxima<A, V>::FunctionMaxima(const FunctionMaxima<A, V> &other) {
    // We use copy constructor of MaximaImpl
    imp = std::make_unique<MaximaImpl>(*other.imp);
}

template <typename A, typename V>
void FunctionMaxima<A, V>::swap(FunctionMaxima<A, V> &other) noexcept {
    std::swap(this->imp, other.imp);
}

// Copy and swap idiom.
template <typename A, typename V>
auto FunctionMaxima<A, V>::operator=(FunctionMaxima other) noexcept -> FunctionMaxima & {
    other.swap(*this);
    return *this;
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::make_point(const std::shared_ptr<A> &arg, const std::shared_ptr<V> &value) -> point_type {
    return point_type(arg, value);
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::make_point(const A &arg, const V &value) -> point_type {
    return make_point(std::make_shared<A>(arg), std::make_shared<V>(value));
}

/*
 * point_type definitions.
 */
template <typename A, typename V>
class FunctionMaxima<A, V>::point_type {
public:
    // Returns function argument.
    A const &arg() const noexcept { return *arg_; }

    // Returns function value at a given point.
    V const &value() const noexcept { return *val_; }

    point_type &operator=(const point_type &);

    // Causes shared ownership of `other` function objects.
    point_type(const point_type &other) = default;

private:
    std::shared_ptr<A> arg_;
    std::shared_ptr<V> val_;

    friend point_type FunctionMaxima::make_point(
            const std::shared_ptr<A> &, const std::shared_ptr<V> &
    );

    explicit point_type(
        const std::shared_ptr<A> &, const std::shared_ptr<V> &) noexcept;
};

/*
 * point_type members' definitions.
 */
template <typename A, typename V>
auto FunctionMaxima<A, V>::point_type::operator=(const point_type &other) -> point_type & {
    // We use copy constructors of A and V.
    arg_ = std::make_shared<A>(other.arg());
    val_ = std::make_shared<V>(other.value());
}

template <typename A, typename V>
FunctionMaxima<A, V>::point_type::point_type(
    const std::shared_ptr<A> &arg, const std::shared_ptr<V> &val
) noexcept : arg_(arg), val_(val) {};

/*
 * MaximaImpl definitions.
 */
template <typename A, typename V>
class FunctionMaxima<A, V>::MaximaImpl {
public:
    MaximaImpl() = default;
    MaximaImpl(const MaximaImpl &other) = default;
    ~MaximaImpl() = default;

    iterator find(A const &) const;
    iterator begin() const { return points.begin(); }
    iterator end() const { return points.end(); }

    mx_iterator mx_begin() const { return mx_points.begin(); }
    mx_iterator mx_end() const { return mx_points.end(); }

    size_type size() const { return points.size(); }

    const V &value_at(const A &) const;
    void set_value(const A &, const V &);
    void erase(const A &);

private:

    enum ToOmit {
        DONT_SKIP, SKIP_LEFT, SKIP_RIGHT
    };

    class Guard;

    // Guards inserting the point_type object into the given multiset
    // with `comparator` comparator.
    template <typename comparator, typename it_type>
    class InsertGuard;

    // Guards erasing a given iterator from the multiset
    // with `comparator` comparator.
    template <typename comparator, typename it_type>
    class DelayedErase;

    class EmptyGuard : public Guard {};

    bool is_a_local_maximum(const iterator &, ToOmit);

    std::unique_ptr<Guard> mark_as_maximum(const iterator &, ToOmit);

    std::unique_ptr<Guard> unmark_as_maximum(const iterator &, ToOmit);

    // Returns left neighbour of `start`.
    iterator left(const iterator &, ToOmit);

    // Returns right neighbour of `start`.
    iterator right(const iterator &, ToOmit);

    // First compares by argument, if equal compares by value
    class point_type_comparator_by_arg;

    // First compares by value, if equal compares by argument.
    class point_type_comparator_by_value;

    // Data members.
    InvalidArg invalid_exception;
    std::multiset<point_type, point_type_comparator_by_arg> points;
    std::multiset<point_type, point_type_comparator_by_value> mx_points;
};

/*
 * MaximaImpl members' definitions.
 */
template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::find(A const &a) const -> iterator {
    std::shared_ptr<A> A_ptr = std::make_shared<A>(a);
    // Using dummy pointer, set comparator only uses the A object.
    point_type pt = make_point(A_ptr, nullptr);
    return points.find(pt);
}

template <typename A, typename V>
const V &FunctionMaxima<A, V>::MaximaImpl::value_at(const A &a) const {
    iterator it = find(a);
    if (it == end())
        throw invalid_exception;
    return it->value();
}

template <typename A, typename V>
void FunctionMaxima<A, V>::MaximaImpl::set_value(const A &a, const V &v) {
    // TODO avoid copying a if already exists
    iterator previous = find(a);
    if (previous != end() && !(previous->value() < v || v <previous->value()))
        return;

    InsertGuard <point_type_comparator_by_arg, iterator> currentGuard
            (make_point(a, v), points);
    iterator current = currentGuard.it;

    bool new_argument = previous == end();

    iterator neighbours[] {
        left(current, new_argument ? DONT_SKIP : SKIP_LEFT),
        current,
        right(current, DONT_SKIP)
    };

    std::unique_ptr<Guard> updateGuards[] {
        mark_as_maximum(neighbours[0],
                new_argument ? DONT_SKIP : SKIP_RIGHT),
        mark_as_maximum(neighbours[1],
                new_argument ? DONT_SKIP : SKIP_LEFT),
        mark_as_maximum(neighbours[2], DONT_SKIP)
    };

    std::unique_ptr<Guard> eraseGuards[] {
        unmark_as_maximum(neighbours[0],
                new_argument ? DONT_SKIP : SKIP_RIGHT),
        unmark_as_maximum(neighbours[1],
                new_argument ? DONT_SKIP : SKIP_LEFT),
        unmark_as_maximum(neighbours[2], DONT_SKIP)
    };

    if (!new_argument) {
        mx_points.erase(*previous);
        // From this point, the function will not throw an exception
        points.erase(previous);
    }

    currentGuard.commit();
    for (auto &guard : updateGuards)
        guard->commit();

    for (auto &guard : eraseGuards)
        guard->commit();
}

template <typename A, typename V>
void FunctionMaxima<A, V>::MaximaImpl::erase(const A &a) {
    iterator to_erase = find(a);
    if (to_erase == end())
        return;

    mx_iterator to_erase_mx = mx_points.find(*to_erase);

    iterator neighbours[] {
        left(to_erase, DONT_SKIP),
        right(to_erase, DONT_SKIP)
    };

    std::unique_ptr<Guard> updateGuards[] {
        mark_as_maximum(neighbours[0], SKIP_RIGHT),
        mark_as_maximum(neighbours[1], SKIP_LEFT)
    };

    std::unique_ptr<Guard> eraseGuards[] {
        unmark_as_maximum(neighbours[0], SKIP_RIGHT),
        unmark_as_maximum(neighbours[1], SKIP_LEFT)
    };

    for (auto &guard : updateGuards)
        guard->commit();

    for (auto &guard : eraseGuards)
        guard->commit();

    if (to_erase_mx != mx_end())
        mx_points.erase(to_erase_mx);
    points.erase(to_erase);
}

template <typename A, typename V>
class FunctionMaxima<A, V>::MaximaImpl::Guard {
protected:
    bool done;

    Guard() : done(false) {}
public:
    virtual ~Guard() = default;

    void commit() { done = true; }
};

template <typename A, typename V>
template <typename comparator, typename it_type>
class FunctionMaxima<A, V>::MaximaImpl::InsertGuard : public Guard {
public:

    it_type it;
    std::multiset<point_type, comparator> *multiset;

    InsertGuard(
        const point_type &point, std::multiset<point_type, comparator> &multiset
    ) : Guard() {
        it = multiset.insert(point);
        this->multiset = &multiset;
    }

    ~InsertGuard() noexcept {
        if (!Guard::done)
            multiset->erase(it);
    }
};

template <typename A, typename V>
template <typename comparator, typename it_type>
class FunctionMaxima<A, V>::MaximaImpl::DelayedErase : public Guard {
public:

    it_type it;
    std::multiset<point_type, comparator> *multiset;

    DelayedErase(
        const it_type &iter, std::multiset<point_type, comparator> &multiset
    ) : Guard(), it(iter), multiset(&multiset) {}

    ~DelayedErase() noexcept {
        if (Guard::done)
            multiset->erase(it);
    }
};

template <typename A, typename V>
bool FunctionMaxima<A, V>::MaximaImpl::is_a_local_maximum(const iterator &it, ToOmit neighbour) {
    return (left(it, neighbour) == end() ||
            !(it->value() < left(it, neighbour)->value()))
           &&
           (right(it, neighbour) == end() ||
           !(it->value() < right(it, neighbour)->value()));
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::mark_as_maximum(
    const iterator &it, ToOmit neighbour
) -> std::unique_ptr<Guard> {
    if (it == points.end())
        return std::make_unique<EmptyGuard>();
    auto it_mx = mx_points.find(*it);

    bool was_a_local_maximum = (it_mx != mx_points.end());

    if (!was_a_local_maximum && is_a_local_maximum(it, neighbour))
        return std::make_unique<InsertGuard<point_type_comparator_by_value, mx_iterator>>(*it, mx_points);
    return std::make_unique<EmptyGuard>();
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::unmark_as_maximum(
    const iterator &it, ToOmit neighbour
) -> std::unique_ptr<Guard> {
    if (it == points.end())
        return std::make_unique<EmptyGuard>();
    auto it_mx = mx_points.find(*it);

    bool was_a_local_maximum = (it_mx != mx_points.end());

    if (was_a_local_maximum && !is_a_local_maximum(it, neighbour))
        return std::make_unique<DelayedErase<point_type_comparator_by_value, mx_iterator>>(it_mx, mx_points);

    return std::make_unique<EmptyGuard>();
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::left(
    const iterator &start, ToOmit neighbour
) -> iterator {
    if (start == begin())
        return end();

    if (neighbour == SKIP_LEFT) {
        return left(std::prev(start), DONT_SKIP);
    } else {
        return std::prev(start);
    }
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::right(
    const iterator &start, ToOmit neighbour
) -> iterator {
    if (start == end())
        return end();

    if (neighbour == SKIP_RIGHT) {
        return right(std::next(start), DONT_SKIP);
    } else {
        return std::next(start);
    }
}

template <typename A, typename V>
class FunctionMaxima<A, V>::MaximaImpl::point_type_comparator_by_arg {
public:
    bool operator()(const point_type &p1, const point_type &p2) const {
        return p1.arg() < p2.arg();
    }
};

template <typename A, typename V>
class FunctionMaxima<A, V>::MaximaImpl::point_type_comparator_by_value {
public:
    //TODO
    bool operator()(const point_type &p1, const point_type &p2) const {
        return !(p1.value() < p2.value()) &&
               !(p2.value() < p1.value()) ?
               p1.arg() < p2.arg() :
               p2.value() < p1.value();

    }
};

#endif /* FUNCTION_MAXIMA_H */
