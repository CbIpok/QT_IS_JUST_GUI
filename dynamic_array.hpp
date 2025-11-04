#pragma once

#include <cstddef>
#include <initializer_list>
#include <new>
#include <stdexcept>
#include <utility>

template <typename T>
class DynamicArray {
public:
    DynamicArray() : data_(nullptr), size_(0), capacity_(0) {}

    explicit DynamicArray(std::size_t count) : data_(nullptr), size_(0), capacity_(0) {
        reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            emplace_back();
        }
    }

    DynamicArray(std::initializer_list<T> init) : DynamicArray() {
        reserve(init.size());
        for (const T& value : init) {
            push_back(value);
        }
    }

    DynamicArray(const DynamicArray& other) : DynamicArray() {
        reserve(other.size_);
        for (std::size_t i = 0; i < other.size_; ++i) {
            push_back(other.data_[i]);
        }
    }

    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    DynamicArray& operator=(DynamicArray other) noexcept {
        swap(other);
        return *this;
    }

    ~DynamicArray() {
        clear();
        operator delete[](data_);
    }

    void swap(DynamicArray& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    std::size_t size() const noexcept { return size_; }
    bool        empty() const noexcept { return size_ == 0; }

    std::size_t capacity() const noexcept { return capacity_; }

    void reserve(std::size_t newCapacity) {
        if (newCapacity <= capacity_) {
            return;
        }
        T* newData = static_cast<T*>(operator new[](newCapacity * sizeof(T)));
        for (std::size_t i = 0; i < size_; ++i) {
            new (&newData[i]) T(std::move_if_noexcept(data_[i]));
            data_[i].~T();
        }
        operator delete[](data_);
        data_ = newData;
        capacity_ = newCapacity;
    }

    void shrink_to_fit() {
        if (size_ == capacity_) {
            return;
        }
        T* newData = nullptr;
        if (size_ > 0) {
            newData = static_cast<T*>(operator new[](size_ * sizeof(T)));
            for (std::size_t i = 0; i < size_; ++i) {
                new (&newData[i]) T(std::move_if_noexcept(data_[i]));
                data_[i].~T();
            }
        }
        operator delete[](data_);
        data_ = newData;
        capacity_ = size_;
    }

    void clear() {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        ensure_capacity_for_one_more();
        new (&data_[size_]) T(std::forward<Args>(args)...);
        ++size_;
        return data_[size_ - 1];
    }

    void push_back(const T& value) {
        ensure_capacity_for_one_more();
        new (&data_[size_]) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        ensure_capacity_for_one_more();
        new (&data_[size_]) T(std::move(value));
        ++size_;
    }

    bool remove_at(std::size_t index) {
        if (index >= size_) {
            return false;
        }
        data_[index].~T();
        for (std::size_t i = index; i + 1 < size_; ++i) {
            new (&data_[i]) T(std::move_if_noexcept(data_[i + 1]));
            data_[i + 1].~T();
        }
        --size_;
        return true;
    }

    template <typename U>
    bool remove_value(const U& value) {
        for (std::size_t i = 0; i < size_; ++i) {
            if (data_[i] == value) {
                return remove_at(i);
            }
        }
        return false;
    }

    template <typename U>
    bool replace_value(const U& oldValue, const U& newValue) {
        for (std::size_t i = 0; i < size_; ++i) {
            if (data_[i] == oldValue) {
                data_[i] = newValue;
                return true;
            }
        }
        return false;
    }

    T& operator[](std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("DynamicArray index out of range");
        }
        return data_[index];
    }

    const T& operator[](std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("DynamicArray index out of range");
        }
        return data_[index];
    }

    T& front() {
        if (empty()) {
            throw std::out_of_range("DynamicArray front on empty array");
        }
        return data_[0];
    }

    const T& front() const {
        if (empty()) {
            throw std::out_of_range("DynamicArray front on empty array");
        }
        return data_[0];
    }

    T& back() {
        if (empty()) {
            throw std::out_of_range("DynamicArray back on empty array");
        }
        return data_[size_ - 1];
    }

    const T& back() const {
        if (empty()) {
            throw std::out_of_range("DynamicArray back on empty array");
        }
        return data_[size_ - 1];
    }

    T*       data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    T* begin() noexcept { return data_; }
    const T* begin() const noexcept { return data_; }
    const T* cbegin() const noexcept { return data_; }

    T* end() noexcept { return data_ + size_; }
    const T* end() const noexcept { return data_ + size_; }
    const T* cend() const noexcept { return data_ + size_; }

private:
    T*          data_;
    std::size_t size_;
    std::size_t capacity_;

    void ensure_capacity_for_one_more() {
        if (size_ == capacity_) {
            std::size_t newCapacity = capacity_ == 0 ? 4 : capacity_ * 2;
            reserve(newCapacity);
        }
    }
};

