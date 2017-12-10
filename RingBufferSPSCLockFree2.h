#ifndef _RING_BUFFER_SPSC_LOCKFREE2_H_
#define _RING_BUFFER_SPSC_LOCKFREE2_H_

#include <cstddef>
#include <atomic>
#include <utility>
#include <memory>


// Bounded queue ( ring buffer ) single producer single consumer queue
// Uses atomic operations with acquire-release memory ordering

// This version differs from RingBufferSPSCLockFree simply that it
// uses a capacity which is the nearest larger power-of-2 and thus the
// operator% can change into a bit mask.  This is a generalization of
// https://jeffreystedfast.blogspot.com/2008/06/calculating-nearest-power-of-2.html
// followed by not adding 1 which would just be removed.
template<typename number>
std::size_t make_mask (number num)
{
    number mask = num;
    size_t bits = sizeof(number) * 8 / 2;

    while (bits) {
        mask |= mask >> bits;
        bits /= 2;
    }

    return mask;
}

template<typename T>
class RingBufferSPSCLockFree2
{
public:

    explicit RingBufferSPSCLockFree2(std::size_t capacity)
        : m_mask{make_mask(capacity)}
        , m_capacity{m_mask+1}
    {
            
        std::unique_ptr <T, BufferDeleter> buffer(new T[m_capacity]);
        m_buffer = std::move(buffer);
        m_write.store(0);
        m_read.store(0);
    }

    ~RingBufferSPSCLockFree2() {
    }

    bool tryPush(T val) {
        const auto current_tail = m_write.load();
        const auto next_tail = increment(current_tail);

        if (current_tail - m_read.load(std::memory_order_acquire) <= m_capacity -1 ) {
            m_buffer.get()[current_tail & m_mask ] = val;
            m_write.store(next_tail , std::memory_order_release);
            return true;
        }

        return false;
    }

    void push(T val) {
        while (!tryPush(val));
    }

    bool tryPop(T* element) {
        const auto currentHead = m_read.load();

        if (currentHead != m_write.load(std::memory_order_acquire)) {
            *element = m_buffer.get()[currentHead & m_mask];
            m_read.store(increment(currentHead) , std::memory_order_release);
            return true;
        }

        return false;
    }

    T pop() {
        T ret;
        while( ! tryPop(&ret) );
        return ret;
    }

private:

    int increment(int n) {
        return (n + 1);
    }

    std::atomic<int> m_write;
    std::atomic<int> m_read;
    std::size_t m_mask;
    std::size_t m_capacity;

    struct BufferDeleter {
        void operator()(T* memory) { delete[] memory; }
    };

    std::unique_ptr<T, BufferDeleter> m_buffer;

    // Move ctor deletion
    RingBufferSPSCLockFree2(RingBufferSPSCLockFree2&& other) = delete;
    // Move assignment operator deletion
    RingBufferSPSCLockFree2& operator=(RingBufferSPSCLockFree2&& other) = delete;
};

#endif
