#ifndef ARENE_H
#define ARENE_H

#include <atomic>
#include <memory>
#include <cstring>

namespace arene {

    // A ring buffer which manages a block of memory.
    //
    // 
    template<typename ElementType> // The data type of one sample
    class BlockBuffer
    {
    public:
        typedef ElementType element_type;
        explicit BlockBuffer(std::size_t width=4096, std::size_t depth=(1<<20))
            : m_cycle{depth-1}
            , m_depth{depth}
            , m_width{width}
        {
            std::unique_ptr <element_type, BlockDeleter> 
                buffer(new element_type[depth*width]);
            m_buffer = std::move(buffer);
            m_write.store(0);
            m_read.store(0);
        }

        ~BlockBuffer() {}

        /// Try to allocate a slot in the buffer, returning true and
        /// setting ptr to the result to ptr.
        bool try_push(element_type*& ptr) {
            
            const auto head = m_write.load();
            const auto next_head = increment(head);

            const auto tail = m_read.load(std::memory_order_acquire);
            if (head - tail < m_depth) {
                const unsigned int slot = head & m_cycle;
                ptr = m_buffer.get() + (slot*m_width);
                m_write.store(next_head , std::memory_order_release);
                return true;
            }
            ptr = nullptr;
            return false;
        }
        /// Like try_push() but block if buffer is full.
        void push(element_type*& ptr) {
            while (!try_push(ptr));
        }

        /// Pop current head if buffer is not full.  If ptr is not
        /// null then copy the memory.
        bool try_pop(element_type** ptr=nullptr) {
            const auto tail = m_read.load();
            if (tail != m_write.load(std::memory_order_acquire)) {
                if (*ptr) {
                    const unsigned int slot = tail&m_cycle;
                    std::memcpy(*ptr, m_buffer.get() + (slot*m_width),
                                m_width*sizeof(element_type));
                }
                m_read.store(increment(tail) , std::memory_order_release);
                return true;
            }
            return false;
        }

        void pop(element_type** ptr = nullptr) {
            while(! try_pop(ptr));
        }

    private:
        int increment(unsigned int n) {
            // fixme: apply mask here to avoid overflow?
            return (n + 1);
        }

        std::size_t m_depth; // the depth of the buffer
        std::size_t m_width; // the width of the buffer
        std::size_t m_cycle; // the mask to cycle around the buffer

        // Using "int" for indices is slightly faster than size_t.
        // The app will not likely need 32 bits of buffer indexing as
        // that would correspond to 22 TB / APA.  Using unsigned int
        // is about 10-20% faster than using signed int.  However, at
        // 2MHz, 32 bit int will overflow in a bit more than a half
        // hour.  64bits will take almost 300 millennia.
        std::atomic<std::size_t> m_write;   // current head buffer index
        std::atomic<std::size_t> m_read;    // current tail buffer index


        
        struct BlockDeleter {
            void operator()(element_type* memory) { delete[] memory; }
        };

        std::unique_ptr<element_type, BlockDeleter> m_buffer;

        // Move ctor deletion
        BlockBuffer(BlockBuffer&& other) = delete;

        // Move assignment operator deletion
        BlockBuffer& operator=(BlockBuffer&& other) = delete;
    };
}

#endif
