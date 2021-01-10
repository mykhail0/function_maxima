#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <memory>
#include <set>

class InvalidArg : public std::exception {
public:
    virtual const char *what() const throw() {
        return "invalid argument value";
    }
};

template<typename A, typename V>
class FunctionMaxima {
public:
    class point_type;

private:

    // First compares by argument, if equal compares by value.
    class point_type_comparator_by_arg;

    // First compares by value, if equal compares by argument.
    class point_type_comparator_by_value;

    // Handle body idiom, of use to non swap idiom.
    class MaximaImpl;

    void swap(FunctionMaxima &) noexcept;

    // Exposed point_type constructor.
    static point_type
    make_point(const std::shared_ptr<A> &, const std::shared_ptr<V> &);

    // Exposed point_type constructor.
    static point_type
    make_point(const A &, const V &);

    std::unique_ptr<MaximaImpl> imp;

public:

    using iterator = typename std::multiset<point_type,
            point_type_comparator_by_arg>::const_iterator;
    using mx_iterator = typename std::multiset<point_type,
            point_type_comparator_by_value>::const_iterator;
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

};

/*
 * FunctionMaxima definitions.
 */
template <typename A, typename V>
FunctionMaxima<A, V>::FunctionMaxima(const FunctionMaxima<A, V> &other) {
    // We use copy constructor of MaximaImpl
    imp = std::make_unique<MaximaImpl>(*other.imp);
}

// Exchanging pointers, nothrow swap idiom.
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

    // For the purpose of avoiding copy constructing the argument
    // if it was already present in the function.
    friend void FunctionMaxima<A, V>::MaximaImpl::set_value(const A &, const V &);

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
) noexcept : arg_(arg), val_(val) {}

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

    // Base Guard class with `commit()` functionality.
    class Guard;

    // Guards inserting the point_type object into the given multiset
    // with `it_type` iterator and `comparator` comparator.
    template <typename comparator, typename it_type>
    class InsertGuard;

    // Guards erasing a given iterator from the given multiset
    // with `it_type` iterator and `comparator` comparator.
    template <typename comparator, typename it_type>
    class DelayedErase;

    class EmptyGuard : public Guard {};

    // Checks if the point pointed to by the given iterator is a local maximum.
    // Omits the second given iterator.
    bool is_a_local_maximum(const iterator &, const iterator &);

    // Adds the point pointed to by the given iterator to mx_points
    // if it is a local maximum. Omits the second given iterator in calculations.
    std::unique_ptr<Guard> mark_as_maximum(const iterator &, const iterator &);

    // Removes the point pointed to by the given iterator from mx_points
    // if it is not a local maximum. Omits the second given iterator in calculations.
    std::unique_ptr<Guard> unmark_as_maximum(const iterator &, const iterator &);

    // Returns left neighbour of `start`, omitting the second iterator if needed.
    iterator left(const iterator &, const iterator &) noexcept;

    // Returns right neighbour of `start`, omitting the second iterator if needed.
    iterator right(const iterator &, const iterator &) noexcept;

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
    iterator previous = find(a);
    if (previous != end() && !(previous->value() < v || v <previous->value()))
        return;

    bool new_argument = previous == end();

    // Avoids copy constructing `a` if it is already present in the domain.
    point_type to_be_inserted = new_argument ?
        make_point(a, v) : make_point(previous->arg_, std::make_shared<V>(v));

    InsertGuard <point_type_comparator_by_arg, iterator> currentGuard
            (to_be_inserted, points);
    iterator current = currentGuard.it;

    iterator neighbours[] {
        left(current, new_argument ? end() : previous),
        current,
        right(current, end())
    };

    std::unique_ptr<Guard> updateGuards[] {
        mark_as_maximum(neighbours[0], new_argument ? end() : previous),
        mark_as_maximum(neighbours[1], new_argument ? end() : previous),
        mark_as_maximum(neighbours[2], end())
    };

    std::unique_ptr<Guard> eraseGuards[] {
        unmark_as_maximum(neighbours[0], new_argument ? end() : previous),
        unmark_as_maximum(neighbours[1], new_argument ? end() : previous),
        unmark_as_maximum(neighbours[2], end())
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
        left(to_erase, end()),
        right(to_erase, end())
    };

    std::unique_ptr<Guard> updateGuards[] {
        mark_as_maximum(neighbours[0], to_erase),
        mark_as_maximum(neighbours[1], to_erase)
    };

    std::unique_ptr<Guard> eraseGuards[] {
        unmark_as_maximum(neighbours[0], to_erase),
        unmark_as_maximum(neighbours[1], to_erase)
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
bool FunctionMaxima<A, V>::MaximaImpl::is_a_local_maximum(const iterator &it, const iterator &to_omit) {
    return (left(it, to_omit) == end() ||
            !(it->value() < left(it, to_omit)->value()))
           &&
           (right(it, to_omit) == end() ||
           !(it->value() < right(it, to_omit)->value()));
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::mark_as_maximum(
    const iterator &it, const iterator &to_omit
) -> std::unique_ptr<Guard> {
    if (it == points.end())
        return std::make_unique<EmptyGuard>();
    auto it_mx = mx_points.find(*it);

    bool was_a_local_maximum = (it_mx != mx_points.end());

    if (!was_a_local_maximum && is_a_local_maximum(it, to_omit))
        return std::make_unique<InsertGuard<point_type_comparator_by_value, mx_iterator>>(*it, mx_points);
    return std::make_unique<EmptyGuard>();
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::unmark_as_maximum(
    const iterator &it, const iterator &to_omit
) -> std::unique_ptr<Guard> {
    if (it == points.end())
        return std::make_unique<EmptyGuard>();
    auto it_mx = mx_points.find(*it);

    bool was_a_local_maximum = (it_mx != mx_points.end());

    if (was_a_local_maximum && !is_a_local_maximum(it, to_omit))
        return std::make_unique<DelayedErase<point_type_comparator_by_value, mx_iterator>>(it_mx, mx_points);

    return std::make_unique<EmptyGuard>();
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::left (
    const iterator &start, const iterator &to_omit
) noexcept -> iterator {
    if (start == begin())
        return end();

    iterator ans = std::prev(start);

    return ans == to_omit ? left(ans, to_omit) : ans;
}

template <typename A, typename V>
auto FunctionMaxima<A, V>::MaximaImpl::right (
    const iterator &start, const iterator &to_omit
) noexcept -> iterator {
    if (start == end())
        return end();

    iterator ans = std::next(start);

    return ans == to_omit ? right(ans, to_omit) : ans;
}

template <typename A, typename V>
class FunctionMaxima<A, V>::point_type_comparator_by_arg {
public:
    bool operator()(const point_type &p1, const point_type &p2) const {
        return p1.arg() < p2.arg();
    }
};

template <typename A, typename V>
class FunctionMaxima<A, V>::point_type_comparator_by_value {
public:
    bool operator()(const point_type &p1, const point_type &p2) const {
        if(p2.value() < p1.value())
            return true;
        else
            return !(p1.value() < p2.value()) && p1.arg() < p2.arg();
    }
};

#endif /* FUNCTION_MAXIMA_H */
