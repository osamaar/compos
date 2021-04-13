#pragma once

#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <memory>
#include <cstdio>


namespace ecs {

using ComponentStorage = void *;
using IDType = uint32_t;
using ConstructItemFunc = void (*)(void*);
using DestroyItemFunc = void (*)(void*);


template <typename IDDomainType>
struct UUID {
    static IDType id_counter;

    template <typename ItemType>
    static const IDType get() {
        static IDType id = id_counter++;
        return id;
    }

    static const IDType generate() {
        return id_counter++;
    }
};

// template <typename IDDomainType, typename ItemType>
// IDType uuidof() {
//     return UUID<IDDomainType>::get<ItemType>();
// }

template<typename T>
IDType UUID<T>::id_counter = 0;

struct TypeMetadata {
    size_t size;
    ConstructItemFunc construct;
    DestroyItemFunc destroy;
};

template <typename T>
void construct_object(void *memory) {
    ::new (memory) T{};
}

template <typename T>
void destroy_object(void *memory) {
    static_cast<T*>(memory)->~T();
}

struct Component {
};
template <typename T>
struct ComponentBase: public Component {
    static const IDType uuid() {
        return UUID<Component>::get<T>();
    }

    // template <typename... Args>
    // static void construct(void *memory, Args... args) {
    //     ::new (memory) T{std::forward(args)...};
    // }
};

struct CompGeometry : public ComponentBase<CompGeometry> {
    int x; int y;
};

struct CompVisual : public ComponentBase<CompVisual> {

};

struct CompUserController : public ComponentBase<CompUserController> {

};

// struct VecWrapperBase {
//     virtual ~VecWrapperBase() = default;
// };

// template <typename T>
// struct VecWrapper: public VecWrapperBase {
//     std::vector<T> inner;
// };

class UntypedVector {
public:
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

    // void resize(size_t size) {
    //     if (size >= m_size) {
    //         reserve(size);
    //         return;
    //     }

    //     int last_elem = size*m_element_size;
    //     int end_elem = m_size*m_element_size;
    //     for ( int i = last_elem - 1; i < end_elem; i += m_element_size) {
    //         destroy(i);
    //     }

    //     m_size = size;
    // }

    void emplace_back_default() {
        if (m_size*m_element_size >= m_data.size()) {
            expand_buffer();
        }
        // T::construct(&m_data[m_size*m_element_size]);
        m_construct_item(&m_data[m_size*m_element_size]);
        m_size++;
    }

    // template <typename T, typename... Args>
    // void emplace_back(Args&&... args) {
    //     if (m_size*m_element_size >= m_data.size()) {
    //         expand_buffer();
    //     }
    //     ::new (&m_data[m_size*m_element_size]) T{std::forward(args)...};
    //     m_size++;
    // }

    // template <typename DestroyFunc>
    void pop_back() {
        destroy(m_size-1);
        m_size--;
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


class ComponentProvider {
public:
    // template <typename T>
    // UntypedVector *make_component_store() {
    //     return make_component_store(T::uuid(), sizeof(T));
    // }

    size_t make_component_store(IDType type_id, const TypeMetadata &metadata) {
        auto &found = m_type_containers.find(type_id);
        if (found == m_type_containers.end()) {
            m_type_containers.emplace(type_id, TypeContainer{});
        }

            m_type_containers[type_id].emplace_back(
                metadata.size,
                metadata.construct,
                metadata.destroy
            );

            return m_type_containers[type_id].size() - 1;

    }

    // Use indexing to refer to component storage (reallocations happen).
    UntypedVector *get_component_store(IDType type_id, size_t idx) {
        return &m_type_containers[type_id][idx];
    }
    
    // ~ComponentProvider() {
    //     for (auto &&iter: m_type_containers) {
    //         delete iter.second;
    //     }
    // }
private:
    using TypeContainer = std::vector<UntypedVector>;
    std::unordered_map<IDType, TypeContainer> m_type_containers;
};

/*
    High-> |    16b    |    16b     |       32b       | <-Low
           | archetype | generation |      index      |
*/
using ComponentIndex = uint64_t;
using ArchetypeIndex = uint16_t;

struct ComponentIndexView {
    uint32_t index;
    uint16_t generation;
    uint16_t archetype;

    ComponentIndex as_u64() {
        return (
            ((uint64_t) index) |
            (((uint64_t) generation) << 32) |
            (((uint64_t) archetype) << 48)
        );
    }

    static ComponentIndexView from_u64(ComponentIndex compidx) {
        return {
            compidx & 0xffffffff,
            (compidx >> 32) & 0xffff,
            (compidx >> 48) & 0xffff,
        };
    }
};


// TODO: Do away with std::set to avoid allocations.
struct ArchetypeFingerprint {
    std::set<IDType> type_ids;

    // Component-wise equality.
    bool operator ==(const ArchetypeFingerprint &other) const {
        if (type_ids.size() != type_ids.size()) return false;

        auto iter_this = type_ids.begin();
        auto iter_that = other.type_ids.begin();

        while (iter_this != type_ids.end()) {
            if (*iter_this != *iter_that) return false;
            ++iter_this; ++iter_that;
        }

        return true;
    }


    std::size_t hash() const noexcept {
        // TODO: Implement!
        return 0;
    }

    void clear() { type_ids.clear(); }
    void append(IDType id) { type_ids.insert(id); }
};

struct ArchetypeFingerprintHasher {
    std::size_t operator()(ArchetypeFingerprint const& a) const noexcept {
        return a.hash();
    }
};

class Archetype {
public:
    Archetype(ComponentProvider &provider)
        : m_components{}
        , m_provider{provider}
        , m_uuid{UUID<Archetype>::generate()}
        , m_fingerprint{}
        , m_size{0}
    {
    }

    IDType create_entity() {
        // printf("Archetype (%p):\n", this);
        for (auto &&comp: m_components) {
            // UntypedVector &vec = *static_cast<UntypedVector*>(comp.second);
            // printf("\tcomp[%x] = <%x>\n", comp.first, comp.second);
            UntypedVector &vec = *m_provider.get_component_store(comp.first, comp.second);
            size_t s = vec.size();
            vec.emplace_back_default();
            s = vec.size();
            s = vec.size();
        }

        m_size++;

        return (m_size - 1);
    }

    // template <typename T>
    // void add_component() {
    //     auto iter = m_components.find(T::uuid());
    //     if (iter != m_components.end()) return;   // already registered
    //     m_components[T::uuid()] = m_provider.make_component_store<T>();
    // }

    void add_component(IDType type_id, const TypeMetadata &metadata) {
        auto iter = m_components.find(type_id);
        if (iter != m_components.end()) return;   // already registered
        m_components[type_id] = m_provider.make_component_store(type_id, metadata);
        // TODO: Increase size of new componenet table to match other comps.
    }

    UntypedVector *get_component_vector(IDType type_id) {
        auto iter = m_components.find(type_id);
        if (iter == m_components.end()) return nullptr;
        return m_provider.get_component_store(type_id, iter->second);
    }

    IDType uuid() {
        return m_uuid;
    };

    // Component-wise equality.
    bool operator ==(const Archetype &other) const {
        if (m_components.size() != m_components.size()) return false;

        auto iter_this = m_components.begin();
        auto iter_that = other.m_components.begin();

        while (iter_this != m_components.end()) {
            if (iter_this->first != iter_that->first) return false;
            ++iter_this; ++iter_that;
        }

        return true;
    }

    const ArchetypeFingerprint &fingerprint() {
        m_fingerprint.clear();
        for (auto &&iter: m_components) {
            m_fingerprint.append(iter.first);
        }
        return m_fingerprint;
    }

private:
    std::map<IDType, size_t> m_components;
    ComponentProvider &m_provider;
    IDType m_uuid;
    ArchetypeFingerprint m_fingerprint;
    size_t m_size;
};

class ComponentManager {
public:
    ComponentManager() = default;
    ~ComponentManager() = default;

    template <typename T>
    void register_component() {
        TypeMetadata meta;
        meta.size = sizeof(T);
        meta.construct = &construct_object<T>;
        meta.destroy = &destroy_object<T>;
        m_component_metadata[T::uuid()] = meta;
    }

    // IDType create_entity(const ArchetypeFingerprint &fingerprint) {
    //     Archetype &a = get_archetype(fingerprint);
    //     // TODO: Construct and return entity id.
    //     for (auto &&type_id : fingerprint.type_ids) {
    //         UntypedVector &v = *a.get_component_vector(type_id);
    //         // TODO: Construct component.
    //         // v.emplace_back<?>();
    //     }
    //     return -1;
    // }

    // struct EntityBuilder {};

    template <typename Arg0, typename... Args>
    IDType create_entity() {
        ArchetypeFingerprint fp;
        return build_entity<Arg0, Args...>(fp);
    }

    template <typename Arg0, typename Arg1, typename... Args>
    IDType build_entity(ArchetypeFingerprint &fp) {
        fp.append(Arg0::uuid());
        return build_entity<Arg1, Args...>(fp);
    }

    template <typename Arg0>
    IDType build_entity(ArchetypeFingerprint &fp) {
        fp.append(Arg0::uuid());
        Archetype &a = get_archetype(fp);
        return a.create_entity();
    }

    // Returns the archetype matching fingerprint. Creates it if needed.
    Archetype &get_archetype(const ArchetypeFingerprint &fingerprint) {
        auto found = m_archetypes.find(fingerprint);
        if (found != m_archetypes.end()) return found->second;

        m_archetypes.emplace(fingerprint, m_provider);
        auto &a = m_archetypes.at(fingerprint);

        for (auto &&type_id: fingerprint.type_ids) {
            a.add_component(type_id, m_component_metadata[type_id]);
        }

        return a;
    }
private:
    using ArchetypeTable = std::unordered_map<
        ArchetypeFingerprint, Archetype,
        ArchetypeFingerprintHasher
    >;

    ComponentProvider m_provider;
    ArchetypeTable m_archetypes;
    std::unordered_map<IDType, TypeMetadata> m_component_metadata;
};

}
