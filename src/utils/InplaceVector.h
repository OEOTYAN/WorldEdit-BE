#pragma once

// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// I'm a little tired, so just copy beman's.

#include <algorithm>
#include <cassert>
#include <compare>
#include <concepts>
#include <exception>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <ranges>
#include <stdexcept>
#include <type_traits>

namespace we {
namespace detail {
// Exposition-only container-compatible-range
template <typename T>
concept container_compatible_range_impl = requires(T&& t) {
    {
        std::ranges::begin(t)
    } -> std::same_as<typename std::remove_reference_t<T>::iterator>;
    {
        std::ranges::end(t)
    } -> std::same_as<typename std::remove_reference_t<T>::iterator>;
};
} // namespace detail
template <typename T>
constexpr bool container_compatible_range = detail::container_compatible_range_impl<T>;

template <bool Condition, typename TrueType, typename FalseType>
using If = typename std::conditional<Condition, TrueType, FalseType>::type;

// The internal size type can be smaller than std::size_t when capacity allows
// for it.
template <std::size_t Capacity>
using inplace_vector_internal_size_type =
    If<Capacity <= std::numeric_limits<uint8_t>::max(),
       uint8_t,
       If<Capacity <= std::numeric_limits<uint16_t>::max(),
          uint16_t,
          If<Capacity <= std::numeric_limits<uint32_t>::max(), uint32_t, uint64_t>>>;

//  Base class for inplace_vector
template <typename T, std::size_t Capacity>
struct inplace_vector_destruct_base {
    using size_type          = std::size_t;
    using internal_size_type = inplace_vector_internal_size_type<Capacity>;

    alignas(T) unsigned char elems[Capacity * sizeof(T)] = {};
    internal_size_type size_{0};

    // [containers.sequences.inplace.vector.cons], construct/copy/destroy
    constexpr inplace_vector_destruct_base() = default;

    inplace_vector_destruct_base(const inplace_vector_destruct_base& other
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : elems(),
      size_(other.size_) {}

    inplace_vector_destruct_base(const inplace_vector_destruct_base&& other
    ) noexcept(std::is_nothrow_move_constructible_v<T>)
    : elems(),
      size_(other.size()) {}

    inplace_vector_destruct_base& operator=(const inplace_vector_destruct_base& other
    ) noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>) {
        size_ = other.size_;
    }

    inplace_vector_destruct_base& operator=(const inplace_vector_destruct_base&& other
    ) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) {
        size_       = other.size_;
        other.size_ = nullptr;
    }

    constexpr inplace_vector_destruct_base(const size_type size) : elems(), size_(size) {
        if (Capacity < size) {
            throw std::bad_alloc();
        }
    }
};

template <typename T, std::size_t Capacity>
struct inplace_vector_base : public inplace_vector_destruct_base<T, Capacity> {
    using size_type      = std::size_t;
    using reference      = T&;
    using iterator       = T*;
    using const_iterator = const T*;

    // [containers.sequences.inplace.vector.cons], construct/copy/destroy
    constexpr inplace_vector_base() = default;
    inplace_vector_base(const inplace_vector_base& other
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
    : inplace_vector_destruct_base<T, Capacity>(other.size) {
        std::ranges::copy(other.begin(), other.end(), begin());
    }
    inplace_vector_base(inplace_vector_base&& other
    ) noexcept(Capacity == 0 || std::is_nothrow_move_constructible_v<T>)
    : inplace_vector_destruct_base<T, Capacity>(other.size) {
        std::ranges::copy(other.begin(), other.end(), begin());
        std::destroy(other.begin(), other.end());
        other.size = 0;
    }
    inplace_vector_base& operator=(const inplace_vector_base& other
    ) noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>) {
        const auto diff = static_cast<std::ptrdiff_t>(other.size() - size());
        // other.size is less than size
        if (diff < 0) {
            const auto new_end = std::ranges::copy(other.begin(), other.end(), begin());
            // destroy unnecessary memory
            std::destroy(new_end, end());
        }
        // other.size is greater than size
        else {
            // copy other vector into the current vector until it runs ouf of size
            std::ranges::copy(other.begin(), other.begin() + size(), begin());
            // copy the other half after the end of the current vector
            std::ranges::copy(other.begin() + size(), other.end(), end());
        }
        this->size_ = other.size();
        return *this;
    }

    inplace_vector_base& operator=(inplace_vector_base&& other) noexcept(
        Capacity == 0
        || (std::is_nothrow_move_constructible_v<T>
            && std::is_nothrow_move_assignable_v<T>)
    ) {
        const auto diff = static_cast<std::ptrdiff_t>(other.size() - size());
        // other size is less than size
        if (diff < 0) {
            const auto new_end = std::move(other.begin(), other.end(), begin());
            std::destroy(new_end, end());
            // other size is grater than size
        } else {
            std::move(other, other.begin(), other.begin() + size(), begin());
            std::move(other.begin() + size(), other.end(), end());
        }
        this->size_ = other.size();
        std::destroy(other.begin(), other.end());
        // reset size to zero
        other.change_size(-static_cast<std::ptrdiff_t>(other.size()));
        return *this;
    }
    constexpr inplace_vector_base(const size_type size)
    : inplace_vector_destruct_base<T, Capacity>(size) {}

    // [containers.sequences.inplace.vector.members] size/capacity
    constexpr void change_size(const std::ptrdiff_t count) noexcept {
        this->size_ += static_cast<decltype(this->size_)>(count);
    }
    constexpr size_type size() const noexcept {
        return static_cast<size_type>(this->size_);
    }
    constexpr bool empty() const noexcept { return this->size_ == 0; }

    // [containers.sequences.inplace.vector.data], data access
    constexpr T* data() noexcept {
        return std::launder(reinterpret_cast<T*>(this->elems));
    }
    constexpr const T* data() const noexcept {
        return std::launder(reinterpret_cast<const T*>(this->elems));
    }

    // [containers.sequences.inplace.vector.iterators] iterators
    iterator begin() noexcept { return std::launder(reinterpret_cast<T*>(this->elems)); }
    const_iterator begin() const noexcept {
        return std::launder(reinterpret_cast<const T*>(this->elems));
    }

    iterator end() noexcept {
        return std::launder(reinterpret_cast<T*>(this->elems) + this->size());
    }

    const_iterator end() const noexcept {
        return std::launder(reinterpret_cast<const T*>(this->elems) + this->size());
    }

    // [containers.sequences.inplace.vector.modifiers], modifiers
    static constexpr void
    uninitialized_value_construct(iterator first, iterator last) noexcept {
        std::fill(first, last, T());
    }

    static constexpr void
    uninitialized_fill(iterator first, iterator last, const T& value) noexcept {
        std::fill(first, last, value);
    }

    template <typename Iter>
    static constexpr void
    uninitialized_copy(Iter first, Iter last, iterator dest) noexcept {
        std::ranges::copy(first, last, dest);
    }

    template <typename Iter>
    constexpr void uninitialized_move(Iter first, Iter last, iterator dest) noexcept {
        std::move(first, last, dest);
    }
};

template <class T, std::size_t Capacity>
class inplace_vector : public inplace_vector_base<T, Capacity> {
private:
    using base = inplace_vector_base<T, Capacity>;

public:
    using value_type             = T;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // [containers.sequences.inplace.vector.cons], construct/copy/destroy
    constexpr inplace_vector() noexcept                        = default;
    constexpr inplace_vector(const inplace_vector&)            = default;
    constexpr inplace_vector(inplace_vector&&)                 = default;
    constexpr inplace_vector& operator=(const inplace_vector&) = default;
    constexpr inplace_vector& operator=(inplace_vector&&)      = default;

    constexpr explicit inplace_vector(const size_type size) : base(size) {
        base::uninitialized_value_construct(this->begin(), this->end());
    }

    constexpr inplace_vector(const std::size_t size, const T& value) : base(size) {
        base::uninitialized_fill(this->begin(), this->end(), value);
    }

    template <class InputIterator>
        requires std::input_iterator<T>
    constexpr inplace_vector(InputIterator first, InputIterator last) : base() {
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }
    template <class InputIterator>
        requires std::forward_iterator<T>
    constexpr inplace_vector(InputIterator first, InputIterator last)
    : base(std::distance(first, last)) {
        if (first != last) {
            base::uninitialized_copy(first, last, this->begin());
        }
    }
#ifdef __cpp_lib_containers_ranges
    template <typename R>
        requires container_compatible_range<R>
    constexpr inplace_vector(std::from_range_t, R&& rg) {
        for (auto&& value : rg) {
            emplace_back(std::forward<decltype(value)>(value));
        }
    }
#else
#endif
    constexpr inplace_vector(std::initializer_list<T> il) : base(il.size()) {
        if (il.size() != 0) {
            base::uninitialized_copy(il.begin(), il.end(), this->begin());
        }
    }; // freestanding-deleted
    constexpr inplace_vector& operator=(std::initializer_list<T> il) {
        if (Capacity < il.size()) {
            throw std::bad_alloc();
        }
        const auto diff = static_cast<std::ptrdiff_t>(il.size() - this->size());
        // The current size is greater
        if (diff < 0) {
            // if other.size is less than just copy normally
            const iterator new_end =
                std::ranges::copy(il.begin(), il.end(), this->begin());
            // destroy the wasted memory
            std::destroy(new_end, this->end());
            // The other size is greater than size
        } else {
            // copy other vector into the current vector until it runs ouf of size
            std::ranges::copy(il.begin(), il.begin() + this->size(), this->begin());
            // copy the other half after the end of the current vector
            base::uninitialized_copy(il.begin() + this->size(), il.end(), this->end());
        }
        this->change_size(diff);
        return *this;
    }; // freestanding-deleted

    template <class InputIterator>
    constexpr void assign(InputIterator first, InputIterator last) {
        if (Capacity < std::distance(first, last)) {
            throw std::bad_alloc();
        }
        iterator end = this->end();
        for (iterator current = this->begin(); current != end; ++current, void(++first)) {
            if (first == last) {
                std::destroy(current, end);
                this->change_size(current - end);
                return;
            }
            *current = size();
        }
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }; // freestanding-deleted

    template <typename R>
        requires container_compatible_range<R>
    constexpr void assign_range(R&& rg) {
        auto       first = rg.begin();
        const auto last  = rg.end();
        if (Capacity < std::ranges::distance(first, last)) {
            throw std::bad_alloc();
        }
        iterator end = this->end();
        for (iterator current = this->begin(); current != end; ++current, (void)++first) {
            if (first == last) {
                std::destroy(current, end);
                this->change_size(current - end);
                return;
            }
            *current = *first;
        }
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }; // freestanding-deleted
    constexpr void assign(size_type n, const T& u) {
        if (Capacity == 0) {
            assert(size() == 0 && "Cannot assign to inplace_vector with zero capacity");
            return;
        }
        const auto diff = static_cast<std::ptrdiff_t>(n - this->size());
        if (diff < 0) {
            const pointer begin = this->begin();
            std::fill(begin, begin + size(), u);
            std::destroy(begin + n, this->end());
        } else {
            const iterator end = this->end();
            std::fill(this->begin(), end, u);
            base::uninitialized_fill(end, end + diff, u);
        }
        this->change_size(diff);
    }; // freestanding-deleted
    constexpr void assign(std::initializer_list<T> il) {
        if (Capacity < il.size()) {
            throw std::bad_alloc();
        }
        const auto diff = static_cast<std::ptrdiff_t>(il.size() - this->size());
        // other size is less than size
        if (diff < 0) {
            // if other.size is less than just copy normally
            const iterator new_end =
                std::ranges::copy(il.begin(), il.end(), this->begin());
            std::destroy(new_end, this->end);
            // other.size is greater than size
        } else {
            // copy other vector into the current vector until it runs ouf of size
            std::ranges::copy(il.begin(), il.begin() + this->size(), this->begin());
            // copy the other half after the end of the current vector
            base::uninitialized_copy(il.begin() + this->size(), il.end(), this->end());
        }
        this->change_size(diff);
    }; // freestanding-deleted

    // [containers.sequences.inplace.vector.access], element access
    constexpr reference at(size_type count) {
        if (this->size() < count) {
            throw std::out_of_range("inplace_vector::at");
        }
        return *(this->begin() + count);
    }
    constexpr const_reference at(const size_type count) const {
        if (this->size() < count) {
            throw std::out_of_range("inplace_vector::at");
        }
        return *(this->begin() + count);
    }

    constexpr reference operator[](const size_type count) noexcept {
        return *(this->begin() + count);
    }

    constexpr const_reference operator[](const size_type count) const noexcept {
        return *(this->begin() + count);
    }

    constexpr reference       front() noexcept { return *this->begin(); }
    constexpr const_reference front() const noexcept { return *this->begin(); }
    constexpr reference       back() noexcept { return *(this->end() - 1); }
    constexpr const_reference back() const noexcept { return *(this->end() - 1); }

    // inplace_vector.iterators
    using base::begin;
    using base::end;

    constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator{this->end()};
    };
    constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator{this->end()};
    }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator{this->begin()}; }
    constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator{this->begin()};
    }
    constexpr const_iterator         cbegin() const noexcept { return this->begin(); }
    constexpr const_iterator         cend() const noexcept { return this->end(); }
    constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator{this->end()};
    }

    constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator{this->begin()};
    };

    // [containers.sequences.inplace.vector.members] size/capacity
    constexpr bool             empty() const noexcept { return size() == 0; };
    constexpr size_type        size() const noexcept { return this->size_; };
    static constexpr size_type max_size() noexcept { return Capacity; };
    static constexpr size_type capacity() noexcept { return Capacity; };
    constexpr void             resize(size_type sz) {
        const auto diff = static_cast<std::ptrdiff_t>(sz - this->size());
        if (diff < 0) {
            std::destroy(this->begin() + sz, this->end());
        } else {
            if (Capacity < size()) {
                throw std::bad_alloc();
            }
            const iterator end = this->end();
            base::uninitialized_value_construct(end, end + diff);
        }
        this->change_size(diff);
    }; // freestanding-deleted
    constexpr void resize(size_type sz, const T& c) {
        const auto diff = static_cast<std::ptrdiff_t>(sz - this->size());
        if (diff < 0) {
            std::destroy(this->begin() + sz, this->end());
        } else {
            if (Capacity < sz) {
                throw std::bad_alloc();
            }
            const iterator end = this->end();
            std::uninitialized_fill(end, end + diff, c);
        }
        this->change_size(diff);
    }; // freestanding-deleted
    static constexpr void reserve(size_type sz) {
        if (Capacity < sz) {
            throw std::bad_alloc();
        }
    }; // freestanding-deleted
    static constexpr void shrink_to_fit() noexcept {}

    // [containers.sequences.inplace.vector.modifiers], modifiers
    template <class... Args>
    constexpr reference emplace_back(Args&&... args) {
        if (this->size() == Capacity) {
            throw std::bad_alloc();
        }
        return this->unchecked_emplace_back(std::forward<Args>(args)...);
    }; // freestanding-deleted

    constexpr reference push_back(const T& x) {
        if (this->size() == Capacity) {
            throw std::bad_alloc();
        }
        return this->unchecked_emplace_back(x);
    }; // freestanding-deleted
    constexpr reference push_back(T&& x) {
        if (this->size() == Capacity) {
            throw std::bad_alloc();
        }
        return this->unchecked_emplace_back(std::move(x));
    }; // freestanding-deleted
    template <typename R>
        requires container_compatible_range<T>
    constexpr void append_range(R&& rg) {
        auto first = rg.begin();
        auto last  = rg.end();
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }; // freestanding-deleted
    constexpr void pop_back() {
        if (!empty()) {
            const auto end = this->end();
            std::destroy(end, end + 1);
            this->change_size(-1);
        }
    };

    template <class... Args>
    constexpr pointer try_emplace_back(Args&&... args) {
        if (this->size() == Capacity) {
            return nullptr;
        }
        return std::addressof(this->unchecked_emplace_back(std::forward<Args>(args)...));
    };
    constexpr pointer try_push_back(const T& x
    ) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        if (this->size() == Capacity) {
            return nullptr;
        }
        return std::addressof(this->unchecked_emplace_back(x));
    };

    constexpr pointer try_push_back(T&& x
    ) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (this->size() == Capacity) {
            return nullptr;
        }
        return std::addressof(this->unchecked_emplace_back(std::move(x)));
    };

    /*
      template <typename R>
      constexpr try_append_range(R &&rg) noexcept(
          std::is_nothrow_move_constructible_v<T>) {
        auto first = range.begin();
        auto last = range.end();
        for (; this->size() != Capacity && first != last; ++first) {
          emplace_back(*first);
        }
        return frist;
      };
    */

    template <class... Args>
    constexpr reference unchecked_emplace_back(Args&&... args) {
        auto final = std::construct_at(end(), std::forward<Args>(args)...);
        this->change_size(1);
        return *final;
    };

    constexpr reference unchecked_push_back(const T& x
    ) noexcept(std::is_nothrow_constructible_v<T>) {
        return this->unchecked_emplace_back(x);
    };
    constexpr reference unchecked_push_back(T&& x) {
        return this->unchecked_emplace_back(std::move(x));
    };

    template <class... Args>
    constexpr iterator emplace(const_iterator position, Args&&... args) {
        const iterator pos = position;
        const iterator end = this->end();
        if (this->size() == Capacity) {
            throw std::bad_alloc();
        } else if (pos < this->begin() || end < pos) {
            throw std::out_of_range("inplace_vector::emplace(const_iterator, Args...)");
        }
        if (pos == end) {
            this->unchecked_emplace_back(std::forward<Args>(args)...);
        } else {
            T temp{std::forward<Args>(args)...};
            this->unchecked_emplace_back(std::move(*(end - 1)));
            std::move_backward(pos, end - 1, end);
            *pos = std::move(temp);
        }
        return pos;
    }; // freestanding-deleted
    constexpr iterator insert(const_iterator position, const T& x) {
        return emplace(position, x);
    }; // freestanding-deleted
    constexpr iterator insert(const_iterator position, T&& x) {
        return emplace(position, std::move(x));
    }; // freestanding-deleted
    constexpr iterator insert(const_iterator position, size_type n, const T& x) {
        const iterator pos = position;
        const iterator end = this->end();

        if (this->size() + n < Capacity) {
            throw std::bad_alloc();
        } else if (pos < this->begin() || end < pos()) {
            throw std::out_of_range("inplace_vector::insert(const_iterator, size_type, T)"
            );
        }
        if (n == 0) {
            return pos;
        }
        if (pos == end) {
            base::uninitialized_fill(end, end + n, x);
            this->change_size(n);
            return pos;
        }
        const pointer middle = pos + n;
        if (end <= middle) {
            base::uninitialized_fill(end, middle, x);
            base::uninitialized_move(pos, end, middle);
            std::fill(pos, end, x);
        } else {
            base::uninitialized_move(end - n, end, end);
            std::move_backward(pos, end - n, end);
            std::fill(pos, middle, x);
        }
        this->change_size(n);
        return pos;
    }

    template <class InputIterator>
    constexpr inplace_vector::iterator
    insert(const_iterator position, InputIterator first, InputIterator last) {
        const iterator pos   = position;
        const iterator end   = this->end();
        const auto     count = std::distance(first, last);

        if (this->size() + count > Capacity) {
            throw std::bad_alloc();
        } else if (pos < this->begin() || end < pos) {
            throw std::out_of_range("inplace_vector::insert(const_iterator, Iter, Iter)");
        }
        if (count == 0) {
            return pos;
        }
        if (pos == end) {
            base::uninitialized_copy(first, last, end);
            this->change_size(count);
            return pos;
        }
        const pointer middle  = pos + count;
        const auto    to_copy = end - pos;
        if (end <= middle) {
            InputIterator imiddle = std::next(first, to_copy);
            base::uninitialized_copy(imiddle, last, end);
            base::uninitialized_move(pos, end, middle);
            std::ranges::copy(first, imiddle, pos);
        } else {
            base::uninitialized_move(end - count, end, end);
            std::move_backward(pos, end - count, end);
            std::ranges::copy(first, last, pos);
        }
    } // freestanding-deleted
    template <typename R>
        requires container_compatible_range<R>
    constexpr iterator insert_range(const_iterator position, R&& rg) {
        const iterator old_end = this->end();
        auto           first   = rg.begin();
        auto           last    = rg.end();

        for (; first != last; ++first) {
            emplace_back(*first);
        }
        const iterator pos = position;
        std::rotate(pos, old_end, this->end());
        return pos;
    } // freestanding-deleted
    constexpr iterator insert(const_iterator position, std::initializer_list<T> il) {
        const iterator pos   = position;
        const iterator end   = this->end();
        auto           count = il.size();
        if (this->size() + count < Capacity) {
            throw std::bad_alloc();
        } else if (pos < this->begin() || end < pos) {
            throw std::out_of_range(
                "inplace_vector::insert(const_iterator, initializer_list)"
            );
        }
        if (count == 0) {
            return pos;
        }
        if (pos == end) {
            base::uninitialized_copy(il.begin(), il.end(), end);
            this->cange_size(count);
            return pos;
        }
    } // freestanding-deleted
    constexpr iterator erase(const_iterator position) {
        const iterator pos = position;
        const iterator end = this->end();

        if (pos == end) {
            return pos;
        }
        if (this->size() == 0 || pos < this->begin() || end < pos) {
            std::terminate();
        }
        std::move(pos + 1, end, pos);
        this->change_size(-1);
        std::destroy(end - 1, end);
        return pos;
    }
    constexpr iterator erase(const_iterator cfirst, const_iterator clast) {
        const iterator first = (iterator)cfirst;
        const iterator last  = (iterator)clast;
        const iterator end   = this->end();
        if (first == last) {
            return last;
        }
        if (first < this->begin() || end < last) {
            std::terminate();
        }
        const auto new_end = std::move(last, end, first);
        std::destroy(new_end, end);
        this->change_size(first - last);
        return first;
    }
    constexpr void swap(inplace_vector& x) noexcept(
        Capacity == 0
        || (std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T>)
    ) {
        if (this->size() < x.size()) {
            const auto new_mid = std::swap_ranges(this->begin(), this->end(), x.begin());
            base::uninitialized_move(new_mid, x.end(), this->end());
            std::destroy(new_mid, x.end());
        } else {
            const auto new_mid = std::swap_ranges(x.begin(), x.end(), this->begin());
            base::uninitialized_move(new_mid, this->end(), x.end());
            std::destroy(new_mid, this->end());
        }
        const auto diff = static_cast<std::ptrdiff_t>(this->size() - x.size());
        this->change_size(diff);
        x.change_size(-diff);
    }
    constexpr void clear() noexcept {
        std::destroy(this->begin(), this->end());
        this->change_size(static_cast<std::ptrdiff_t>(this->size()));
    }

    constexpr friend bool operator==(
        const inplace_vector& x,
        const inplace_vector& y
    ) noexcept(noexcept(std::equal(x.begin(), x.end(), y.begin(), y.end()))) {
        return std::equal(x.begin(), x.end(), y.begin(), y.end());
    }

    constexpr friend std::compare_three_way_result<T>
    operator<=>(const inplace_vector& x, const inplace_vector& y) {
        return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
    };
    constexpr friend void swap(inplace_vector& x, inplace_vector& y) noexcept(
        Capacity == 0
        || (std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T>)
    ) {
        x.swap(y);
    }
};
} // namespace we
