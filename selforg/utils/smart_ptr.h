/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Smart pointer utilities for gradual migration to modern C++          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SMART_PTR_H
#define __SMART_PTR_H

#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>

namespace lpzrobots {

// Type aliases for commonly used smart pointers
template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using WeakPtr = std::weak_ptr<T>;

// Factory function for creating unique_ptr (C++11 compatible)
template<typename T, typename... Args>
inline UniquePtr<T> make_unique(Args&&... args) {
#if __cplusplus >= 201402L
    return std::make_unique<T>(std::forward<Args>(args)...);
#else
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
#endif
}

// Object pool for performance-critical allocations
template<typename T>
class ObjectPool {
public:
    using Deleter = std::function<void(T*)>;
    
    ObjectPool(size_t initialSize = 16) {
        pool_.reserve(initialSize);
        for (size_t i = 0; i < initialSize; ++i) {
            pool_.emplace_back(new T());
        }
    }
    
    ~ObjectPool() {
        for (auto* obj : pool_) {
            delete obj;
        }
    }
    
    // Get object from pool with custom deleter that returns it
    std::unique_ptr<T, Deleter> acquire() {
        if (pool_.empty()) {
            // Create new object if pool is empty
            return std::unique_ptr<T, Deleter>(
                new T(),
                [this](T* obj) { this->release(obj); }
            );
        }
        
        T* obj = pool_.back();
        pool_.pop_back();
        return std::unique_ptr<T, Deleter>(
            obj,
            [this](T* obj) { this->release(obj); }
        );
    }
    
private:
    void release(T* obj) {
        if (obj) {
            obj->reset(); // Assumes T has a reset() method
            pool_.push_back(obj);
        }
    }
    
    std::vector<T*> pool_;
    
    // Disable copy and move
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;
};

// RAII wrapper for legacy code that uses raw pointers
template<typename T>
class LegacyPtrWrapper {
public:
    explicit LegacyPtrWrapper(T* ptr = nullptr) : ptr_(ptr) {}
    
    ~LegacyPtrWrapper() {
        delete ptr_;
    }
    
    // Move operations
    LegacyPtrWrapper(LegacyPtrWrapper&& other) noexcept 
        : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    LegacyPtrWrapper& operator=(LegacyPtrWrapper&& other) noexcept {
        if (this != &other) {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
    
    // Disable copy
    LegacyPtrWrapper(const LegacyPtrWrapper&) = delete;
    LegacyPtrWrapper& operator=(const LegacyPtrWrapper&) = delete;
    
    // Access
    T* get() const { return ptr_; }
    T* operator->() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    
    // Release ownership
    T* release() {
        T* tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }
    
    // Reset with new pointer
    void reset(T* ptr = nullptr) {
        delete ptr_;
        ptr_ = ptr;
    }
    
private:
    T* ptr_;
};

// Helper for managing collections of pointers (common in lpzrobots)
template<typename T>
class PtrVector {
public:
    using value_type = std::unique_ptr<T>;
    using container_type = std::vector<value_type>;
    
    PtrVector() = default;
    
    // Add object (takes ownership)
    void add(T* obj) {
        vec_.emplace_back(obj);
    }
    
    // Add object (move unique_ptr)
    void add(std::unique_ptr<T> obj) {
        vec_.push_back(std::move(obj));
    }
    
    // Create and add object
    template<typename... Args>
    T* emplace(Args&&... args) {
        vec_.push_back(make_unique<T>(std::forward<Args>(args)...));
        return vec_.back().get();
    }
    
    // Access
    size_t size() const { return vec_.size(); }
    bool empty() const { return vec_.empty(); }
    
    T* operator[](size_t idx) { return vec_[idx].get(); }
    const T* operator[](size_t idx) const { return vec_[idx].get(); }
    
    // Iteration support
    auto begin() { return vec_.begin(); }
    auto end() { return vec_.end(); }
    auto begin() const { return vec_.begin(); }
    auto end() const { return vec_.end(); }
    
    // Clear all objects
    void clear() { vec_.clear(); }
    
    // Remove specific object
    void remove(T* obj) {
        vec_.erase(
            std::remove_if(vec_.begin(), vec_.end(),
                [obj](const value_type& ptr) { return ptr.get() == obj; }),
            vec_.end()
        );
    }
    
private:
    container_type vec_;
};

// Utility to help with gradual migration
// Usage: auto ptr = to_unique(new MyClass()); 
template<typename T>
inline std::unique_ptr<T> to_unique(T* ptr) {
    return std::unique_ptr<T>(ptr);
}

// Utility for shared ownership migration
template<typename T>
inline std::shared_ptr<T> to_shared(T* ptr) {
    return std::shared_ptr<T>(ptr);
}

} // namespace lpzrobots

#endif // __SMART_PTR_H