/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/
#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#include <array>
#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace lpzrobots {

/**
 * @brief A modern circular buffer implementation using std::array
 * 
 * This template class provides a fixed-size circular buffer with efficient
 * access to historical values. It's designed to replace C-style arrays used
 * for buffering sensor and motor values in controllers.
 * 
 * @tparam T The type of elements stored in the buffer
 * @tparam N The size of the buffer (must be > 0)
 * 
 * @example
 * ```cpp
 * CircularBuffer<matrix::Matrix, 10> sensor_buffer;
 * sensor_buffer.push(current_sensors);
 * auto sensors_t_minus_1 = sensor_buffer.get(-1);  // Get previous value
 * ```
 */
template<typename T, std::size_t N>
class CircularBuffer {
    static_assert(N > 0, "CircularBuffer size must be greater than 0");
    
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    
    /// Default constructor - initializes all elements with default value
    CircularBuffer() : buffer_{}, head_(0), size_(0) {}
    
    /// Constructor with initial value for all elements
    explicit CircularBuffer(const T& init_value) : buffer_{}, head_(0), size_(0) {
        buffer_.fill(init_value);
    }
    
    /// Push a new element to the buffer (overwrites oldest if full)
    void push(const T& value) {
        buffer_[head_] = value;
        head_ = (head_ + 1) % N;
        if (size_ < N) {
            ++size_;
        }
    }
    
    /// Push a new element using move semantics
    void push(T&& value) {
        buffer_[head_] = std::move(value);
        head_ = (head_ + 1) % N;
        if (size_ < N) {
            ++size_;
        }
    }
    
    /**
     * @brief Get element at relative position
     * @param offset Relative position (0 = most recent, -1 = previous, etc.)
     * @return Reference to the element
     * @throws std::out_of_range if offset is out of valid range
     */
    [[nodiscard]] reference get(difference_type offset = 0) {
        return const_cast<reference>(const_cast<const CircularBuffer*>(this)->get(offset));
    }
    
    /**
     * @brief Get element at relative position (const version)
     * @param offset Relative position (0 = most recent, -1 = previous, etc.)
     * @return Const reference to the element
     * @throws std::out_of_range if offset is out of valid range
     */
    [[nodiscard]] const_reference get(difference_type offset = 0) const {
        if (empty()) {
            throw std::out_of_range("CircularBuffer is empty");
        }
        
        // Normalize offset to positive value
        while (offset < 0) {
            offset += static_cast<difference_type>(size_);
        }
        offset = offset % static_cast<difference_type>(size_);
        
        // Calculate actual index
        size_type actual_pos = (head_ + N - 1 - static_cast<size_type>(offset)) % N;
        return buffer_[actual_pos];
    }
    
    /**
     * @brief Get element at time t (for compatibility with old code)
     * @param t Time index (absolute)
     * @param time_offset Current time offset
     * @return Reference to the element
     */
    [[nodiscard]] reference at_time(size_type t, size_type time_offset) {
        size_type index = (t + time_offset) % N;
        return buffer_[index];
    }
    
    /**
     * @brief Get element at time t (const version)
     * @param t Time index (absolute)
     * @param time_offset Current time offset
     * @return Const reference to the element
     */
    [[nodiscard]] const_reference at_time(size_type t, size_type time_offset) const {
        size_type index = (t + time_offset) % N;
        return buffer_[index];
    }
    
    /// Clear the buffer (resets size but doesn't destroy elements)
    void clear() {
        size_ = 0;
        head_ = 0;
    }
    
    /// Fill all elements with a value
    void fill(const T& value) {
        buffer_.fill(value);
        size_ = N;
    }
    
    /// Check if buffer is empty
    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }
    
    /// Check if buffer is full
    [[nodiscard]] bool full() const noexcept {
        return size_ == N;
    }
    
    /// Get current number of elements
    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }
    
    /// Get maximum capacity
    [[nodiscard]] constexpr size_type capacity() const noexcept {
        return N;
    }
    
    /// Get most recent element (same as get(0))
    [[nodiscard]] reference front() {
        return get(0);
    }
    
    /// Get most recent element (const version)
    [[nodiscard]] const_reference front() const {
        return get(0);
    }
    
    /// Get oldest element
    [[nodiscard]] reference back() {
        return get(-static_cast<difference_type>(size_ - 1));
    }
    
    /// Get oldest element (const version)
    [[nodiscard]] const_reference back() const {
        return get(-static_cast<difference_type>(size_ - 1));
    }
    
    /// Access underlying array (for advanced use)
    [[nodiscard]] std::array<T, N>& data() noexcept {
        return buffer_;
    }
    
    /// Access underlying array (const version)
    [[nodiscard]] const std::array<T, N>& data() const noexcept {
        return buffer_;
    }
    
private:
    std::array<T, N> buffer_;  ///< The underlying storage
    size_type head_;          ///< Position for next insertion
    size_type size_;          ///< Current number of valid elements
};

/**
 * @brief Type alias for Matrix buffers commonly used in controllers
 */
template<std::size_t N>
using MatrixBuffer = CircularBuffer<matrix::Matrix, N>;

} // namespace lpzrobots

#endif // __CIRCULAR_BUFFER_H