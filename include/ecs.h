#pragma once

#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <memory>
#include <cstdio>
#include <cstring>


namespace ecs {

using typeid_t = uint16_t;            // For class/object UUID
using ent_idx_t = uint32_t;
using ent_gen_t = uint32_t;
using ent_archidx_t = uint32_t;

static const typeid_t TYPEID_NONE = (ent_idx_t) -1;
static const ent_idx_t ENT_IDX_NONE = (ent_idx_t) -1;
static const ent_gen_t ENT_GEN_NONE = (ent_gen_t) -1;
static const ent_archidx_t ENT_ARCHIDX_NONE = (ent_archidx_t) -1;

struct EntityID {
    ent_idx_t index;
    ent_gen_t generation;
};

struct EntityRecord {
    ent_idx_t index;
    ent_archidx_t archetype;
    ent_gen_t generation;
};

struct EntityTable {
    std::vector<EntityRecord> index;
};

using ComponentStorage = void *;
using ConstructItemFunc = void (*)(void*);
using DestroyItemFunc = void (*)(void*);

template <typename IDDomainType>
struct UUID {
    static typeid_t id_counter;

    template <typename ItemType>
    static const typeid_t get() {
        static typeid_t id = id_counter++;
        return id;
    }

    static const typeid_t generate() {
        return id_counter++;
    }
};

// A marker class to use for UUID counting.
class Component {};

// template <typename IDDomainType, typename ItemType>
// IDType uuidof() {
//     return UUID<IDDomainType>::get<ItemType>();
// }

template<typename T>
typeid_t UUID<T>::id_counter = 0;

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

    size_t make_component_store(typeid_t type_id, const TypeMetadata &metadata) {
        const auto &found = m_type_containers.find(type_id);
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
    UntypedVector *get_component_store(typeid_t type_id, size_t idx) {
        return &m_type_containers[type_id][idx];
    }
    
    // ~ComponentProvider() {
    //     for (auto &&iter: m_type_containers) {
    //         delete iter.second;
    //     }
    // }
private:
    using TypeContainer = std::vector<UntypedVector>;
    std::unordered_map<typeid_t, TypeContainer> m_type_containers;
};

// TODO: Do away with std::set to avoid allocations.
struct ArchetypeFingerprint {
    std::set<typeid_t> type_ids;

    ArchetypeFingerprint(std::initializer_list<typeid_t> ids)
        : type_ids{ids}
    { 

    }

    // Component-wise equality.
    bool operator ==(const ArchetypeFingerprint &other) const {
        if (type_ids.size() != other.type_ids.size()) return false;

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
    void append(typeid_t id) { type_ids.insert(id); }
    size_t size() const { return type_ids.size(); }
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
        , m_back_ptr{}
        , m_provider{provider}
        , m_uuid{UUID<Archetype>::generate()}
        , m_fingerprint{}
        , m_size{0}
    {
    }

    ent_idx_t create_entity(ent_idx_t table_index) {
        // printf("Archetype (%p):\n", this);
        for (auto &&comp: m_components) {
            // UntypedVector &vec = *static_cast<UntypedVector*>(comp.second);
            // printf("\tcomp[%x] = <%x>\n", comp.first, comp.second);
            UntypedVector &vec = *m_provider.get_component_store(comp.first, comp.second);
            vec.emplace_back_default();
        }

        m_back_ptr.push_back(table_index);
        m_size++;
        return (m_size - 1);
    }

    size_t size() const { return m_size; }

    // template <typename T>
    // void add_component() {
    //     auto iter = m_components.find(T::uuid());
    //     if (iter != m_components.end()) return;   // already registered
    //     m_components[T::uuid()] = m_provider.make_component_store<T>();
    // }

    void add_component(typeid_t type_id, const TypeMetadata &metadata) {
        auto iter = m_components.find(type_id);
        if (iter != m_components.end()) return;   // already registered
        m_components[type_id] = m_provider.make_component_store(type_id, metadata);
        // TODO: Increase size of new componenet table to match other comps.
    }

    ent_idx_t remove_swap(ent_idx_t idx) {
        for (auto &&comp: m_components) {
            UntypedVector &vec = *m_provider.get_component_store(comp.first, comp.second);
            vec.emplace_back_default();
            vec.remove_swap(idx);
        }

        ent_idx_t result = ENT_IDX_NONE;

        // Only swap if have 2+ elements and not removing last element.
        if ((m_size > 1) && (idx != (m_size - 1))) {
            ent_idx_t back_ptr_swapped = m_back_ptr.back();
            m_back_ptr[idx] = back_ptr_swapped;
            m_back_ptr.pop_back();
            result = back_ptr_swapped;
        }

        m_size--;
        return result;
    }

    UntypedVector *get_component_vector(typeid_t type_id) {
        auto iter = m_components.find(type_id);
        if (iter == m_components.end()) return nullptr;
        return m_provider.get_component_store(type_id, iter->second);
    }

    typeid_t uuid() const {
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

    const ArchetypeFingerprint &fingerprint() const {
        auto p = const_cast<Archetype*>(this);
        p->m_fingerprint.clear();
        for (auto &&iter: p->m_components) {
            p->m_fingerprint.append(iter.first);
        }
        return m_fingerprint;
    }

private:
    std::map<typeid_t, size_t> m_components;
    std::vector<ent_idx_t> m_back_ptr;
    ComponentProvider &m_provider;
    typeid_t m_uuid;
    ArchetypeFingerprint m_fingerprint;
    size_t m_size;
};

class ComponentManager {
public:
    using ArchetypeTable = std::unordered_map<
        ArchetypeFingerprint, Archetype,
        ArchetypeFingerprintHasher
    >;
    using ComponentMetadataTable  = std::unordered_map<typeid_t, TypeMetadata>;

    ComponentManager()
        : m_provider {}
        , m_archetypes {}
        , m_component_metadata {}
        , m_entity_table {}
        , m_free_head {ENT_IDX_NONE}
    {

    }

    ~ComponentManager() = default;

    template <typename T>
    void register_component() {
        TypeMetadata meta;
        meta.size = sizeof(T);
        meta.construct = &construct_object<T>;
        meta.destroy = &destroy_object<T>;
        m_component_metadata[UUID<Component>::get<T>()] = meta;
    }

    template <typename... Args>
    EntityID create_entity() {
        ent_idx_t table_idx = m_free_head;

        if (table_idx == ENT_IDX_NONE) {
            m_entity_table.push_back({});
            table_idx = m_entity_table.size() - 1;
            m_entity_table[table_idx].generation = 0;
        }


        ArchetypeFingerprint fp { UUID<Component>::get<Args>()... };
        Archetype &a = get_archetype(fp);
        ent_idx_t idx = a.create_entity(table_idx);

        auto& record = m_entity_table[table_idx];
        record.index = idx;
        record.archetype = a.uuid();

        return EntityID{table_idx, record.generation};
    }

    bool delete_entity(EntityID entity) {
        if (m_entity_table.size() <= entity.index) {
            return false;
        }

        EntityRecord record = m_entity_table[entity.index];

        if (record.generation != entity.generation) {
            return false;
        }

        for (auto&& a: m_archetypes) {
            if (a.second.uuid() == record.archetype) {
                ent_idx_t back_ptr = a.second.remove_swap(record.index);
                if (back_ptr == ENT_IDX_NONE) break;    // no swap
                m_entity_table[back_ptr].index = record.index;
                break;
            }
        }

        record.generation += 1;
        record.index = m_free_head;
        m_free_head = entity.index;
    }

    template <typename... Args>
    void add_component() {

    }

    template <typename... Args>
    void remove_component() {

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

    const ComponentProvider&
    component_provider() const { return m_provider; }

    const ArchetypeTable&
    archetype_table() const { return m_archetypes; }

    const ComponentMetadataTable&
    component_metadata() const { return m_component_metadata; }

private:
    ComponentProvider m_provider;
    ArchetypeTable m_archetypes;
    ComponentMetadataTable m_component_metadata;
    std::vector<EntityRecord> m_entity_table;
    ent_idx_t m_free_head;
};

}
