#pragma once

#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <memory>
#include <cstdio>


namespace ecs {

using ComponentStorage = void *;
using IDType = uint32_t;

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

template<typename T>
IDType UUID<T>::id_counter = 0;

struct Component { };
template <typename T>
struct ComponentBase: public Component {
    static const IDType uuid() {
        return UUID<Component>::get<T>();
    }
};

struct CompGeometry : public ComponentBase<CompGeometry> {

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

class ComponentBuffer {
public:
    ComponentBuffer(size_t element_size)
        : m_element_size{element_size}
        , data{}
    {

    }

    void *operator [](size_t idx) { return get(idx); }

    void *get(size_t idx) {
        return static_cast<void*>(&data[idx*m_element_size]);
    }

    size_t size() {
        return data.size()/m_element_size;
    }

private:
    size_t m_element_size;
    std::vector<unsigned char> data;
};


class ComponentProvider {
public:
    template <typename T>
    ComponentBuffer *make_component_store() {
        return make_component_store(T::uuid(), sizeof(T));
    }

    ComponentBuffer *make_component_store(IDType comptype, size_t element_size) {
        auto &found = m_type_containers.find(comptype);
        if (found == m_type_containers.end()) {
            m_type_containers.emplace(comptype, TypeContainer{});
        }

            m_type_containers[comptype].emplace_back(element_size);
            return &m_type_containers[comptype].back();

    }
    
    // ~ComponentProvider() {
    //     for (auto &&iter: m_type_containers) {
    //         delete iter.second;
    //     }
    // }
private:
    using TypeContainer = std::vector<ComponentBuffer>;
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


struct ArchetypeFingerprint {
    std::vector<IDType> type_ids;

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
    void append(IDType id) { type_ids.push_back(id); }
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
    {
    }

    template <typename T>
    void add_component() {
        auto iter = m_components.find(T::uuid());
        if (iter != m_components.end()) return;   // already registered
        m_components[T::uuid()] = m_provider.make_component_store<T>();
    }

    void add_component(IDType type_id, size_t element_size) {
        auto iter = m_components.find(type_id);
        if (iter != m_components.end()) return;   // already registered
        m_components[type_id] = m_provider.make_component_store(type_id, element_size);
    }

    template <typename T>
    std::vector<char[sizeof(T)]> *get_component_vector() {
        auto iter = m_components.find(T::uuid());
        if (iter == m_components.end()) return nullptr;
        return static_cast<std::vector<char[sizeof(T)]>*>(iter->second);
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
    std::map<IDType, void*> m_components;
    ComponentProvider &m_provider;
    IDType m_uuid;
    ArchetypeFingerprint m_fingerprint;
};

class ComponentManager {
public:
    ComponentManager() = default;
    ~ComponentManager() = default;

    template <typename T>
    void register_component() {
        m_component_sizes[T::uuid()] = sizeof(T);
    }

    IDType create_entity(const ArchetypeFingerprint &fingerprint) {
        Archetype &a = get_archetype(fingerprint);
        // TODO: Construct and return entity id.
        return -1;
    }

    // Returns the archetype matching fingerprint. Creates it if needed.
    Archetype &get_archetype(const ArchetypeFingerprint &fingerprint) {
        auto found = m_archetypes.find(fingerprint);
        if (found == m_archetypes.end()) return found->second;

        m_archetypes.emplace(fingerprint, m_provider);
        auto &a = m_archetypes.at(fingerprint);

        for (auto &&type_id: fingerprint.type_ids) {
            a.add_component(type_id, m_component_sizes[type_id]);
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
    std::unordered_map<IDType, size_t> m_component_sizes;
};

}
