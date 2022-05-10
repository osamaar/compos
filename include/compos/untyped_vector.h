#pragma once

#include <vector>

template <typename T>
void construct_object(void *memory) {
    ::new (memory) T{};
}

template <typename T>
void destroy_object(void *memory) {
    static_cast<T*>(memory)->~T();
}

class UntypedVector {
public:
    using ConstructItemFunc = void (*)(void*);
    using DestroyItemFunc = void (*)(void*);

    struct TypeMetadata {
        size_t size;
        UntypedVector::ConstructItemFunc construct;
        UntypedVector::DestroyItemFunc destroy;
    };

    UntypedVector(size_t element_size, ConstructItemFunc construct, DestroyItemFunc destroy)
        : m_element_size{element_size}
        , m_size{0}
        , m_expand_size{INITIAL_CAPACITY*element_size}
        , m_data{}
        , m_construct_item(construct)
        , m_destroy_item(destroy)
    {

    }

    void *operator [](size_t idx) { return get(idx); }

    void *get(size_t idx) {
        return static_cast<void*>(&m_data[idx*m_element_size]);
    }

    void *back() {
        auto idx = m_size - 1;
        return static_cast<void*>(&m_data[idx*m_element_size]);
    }

    template <typename T>
    T &get_ref(size_t idx) {
        return *static_cast<T*>(&m_data[idx*m_element_size]);
    }

    size_t capacity() {
        return m_data.size()/m_element_size;
    }

    size_t size() {
        return m_size;
    }

    void reserve(size_t size) {
        if (size <= m_size) return;
        resize_buffer(size*m_element_size);
    }

    void emplace_back_default() {
        if (m_size*m_element_size >= m_data.size()) {
            expand_buffer();
        }
        // T::construct(&m_data[m_size*m_element_size]);
        m_construct_item(&m_data[m_size*m_element_size]);
        m_size++;
    }

    void pop_back() {
        destroy(m_size-1);
        m_size--;
    }

    void remove_swap(size_t idx) {
        std::memcpy(&m_data[idx*m_element_size], back(), m_element_size);
        pop_back();
    }

    // template <typename DestroyFunc>
    void destroy(size_t idx) {
        // T* elem = static_cast<T*>(&m_data[idx*m_element_size]);
        // elem->~T;
        m_destroy_item(&m_data[idx*m_element_size]);
    }

    // template <typename DestroyFunc>
    void destroy_all() {
        for (int i = 0; i < m_size; i ++) {
            destroy(i);
        }
    }

    size_t element_size() { return m_element_size; }
private:
    static const size_t INITIAL_CAPACITY = 1;
    size_t m_element_size;
    size_t m_size;
    size_t m_expand_size;
    std::vector<unsigned char> m_data;
    ConstructItemFunc m_construct_item;
    DestroyItemFunc m_destroy_item;

    void expand_buffer() {
        resize_buffer(m_data.size() + m_expand_size);
        // m_expand_size <<= 1;
    }

    void resize_buffer(size_t size) {
        m_data.resize(size);
        if (size > 0) {
            m_expand_size = m_data.size();
        }
    }
};
