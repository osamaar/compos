#pragma once

#include <iterator>
#include <cstddef>
#include <vector>
#include <unordered_map>
#include "const.h"
#include "uuid.h"
#include "type_alias.h"
#include "archetype.h"
#include "component.h"
#include "untyped_vector.h"

namespace ecs {

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

class EntityManager {
public:
    using ArchetypeTable = std::unordered_map<
        ArchetypeFingerprint, Archetype,
        ArchetypeFingerprintHasher
    >;
    using ComponentMetadataTable  = std::unordered_map<typeid_t, UntypedVector::TypeMetadata>;

    template <typename T>
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = value_type*;
        using reference         = value_type&;
    };

    EntityManager()
        : m_provider {}
        , m_archetypes {}
        , m_component_metadata {}
        , m_entity_table {}
        , m_free_head {ENT_IDX_NONE}
        , m_size {0}
    {

    }

    ~EntityManager() = default;

    template <typename T>
    void register_component() {
        UntypedVector::TypeMetadata meta;
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
            table_idx = (ent_idx_t)(m_entity_table.size() - 1);
            m_entity_table[table_idx].generation = 0;
        }

        ArchetypeFingerprint fp { UUID<Component>::get<Args>()... };
        Archetype &a = find_or_create_archetype(fp);
        ent_idx_t idx = a.create_entity(table_idx);

        auto& record = m_entity_table[table_idx];
        record.index = idx;
        record.archetype = a.uuid();

        m_size++;
        return EntityID{table_idx, record.generation};
    }

    bool is_entity_id_valid(const EntityID &id) {
        if (
            (m_entity_table.size() <= id.index) ||
            (m_entity_table[id.index].generation != id.generation)
        ) {
            return false;
        }

        return true;
    }

    bool delete_entity(EntityID entity) {
        if (!is_entity_id_valid(entity)) return false;
        EntityRecord& record = m_entity_table[entity.index];

        for (auto&& a: m_archetypes) {
            if (a.second.uuid() == record.archetype) {
                remove_entity_data_from_archetype(a.second, record.index);
                break;
            }
        }

        record.generation += 1;
        record.index = m_free_head;
        m_free_head = entity.index;
        m_size--;
        return true;
    }

    template <typename... Args>
    void add_component(EntityID id) {
        if (!is_entity_id_valid(entity)) return false;
        auto& record = m_entity_table[id.index];

        // Find old archetype.
        Archetype& a_old = *find_archetype(record.archetype);

        // Find old and new archetype fingerprints.
        ArchetypeFingerprint& fp_old = a_old->fingerprint();
        ArchetypeFingerprint fp_new{fp_old};
        fp_new.append(UUID<Component>::get<Args>()...);

        Archetype &a_new = find_or_create_archetype(fp_new);
        ent_idx_t idx_old = record.index;
        
        // Create new.
        ent_idx_t idx_new = a_new.create_entity(id.index);

        // Copy old data.
        for (auto&& type_id: fp_old->type_ids) {
            auto v_old = a_old.get_component_vector(type_id);
            auto v_new = a_new.get_component_vector(type_id);
            auto elem_old = v_old->get(idx_old);
            auto elem_new = v_old->get(idx_new);
            memcpy(elem_new, elem_old, v_old->element_size());
        }

        // Point to new data.
        record.archetype = a_new.uuid();
        record.index = idx_new;

        // Remove old.
        remove_entity_data_from_archetype(a_old, idx_old);
    }

    template <typename Arg0, typename... Args>
    void remove_component(EntityID id) {
        remove_component(
            id,
            {
                UUID<Component>::get<Arg0>(),
                UUID<Component>::get<Args>()...,
            }
        );
    }

    bool remove_component(EntityID eid, std::initializer_list<typeid_t> tids) {
        if (!is_entity_id_valid(eid)) return false;
        auto& record = m_entity_table[eid.index];

        // Find old archetype.
        Archetype& a_old = *find_archetype(record.archetype);

        // Find old and new archetype fingerprints.
        const ArchetypeFingerprint& fp_old = a_old.fingerprint();
        ArchetypeFingerprint fp_new{fp_old};
        fp_new.append(tids);

        for (auto&& tid: tids) {
            fp_new.type_ids.erase(tid);
        }

        Archetype &a_new = find_or_create_archetype(fp_new);
        ent_idx_t idx_old = record.index;
        
        // Create new.
        ent_idx_t idx_new = a_new.create_entity(eid.index);

        // Copy old data.
        for (auto&& type_id: fp_new.type_ids) {
            auto v_old = a_old.get_component_vector(type_id);
            auto v_new = a_new.get_component_vector(type_id);
            auto elem_old = v_old->get(idx_old);
            auto elem_new = v_new->get(idx_new);
            memcpy(elem_new, elem_old, v_old->element_size());
        }

        // Point to new data.
        record.archetype = a_new.uuid();
        record.index = idx_new;

        // Remove old.
        remove_entity_data_from_archetype(a_old, idx_old);
        return true;
    }

    template <typename T>
    T *get_component(EntityID id) {
        if (!is_entity_id_valid(id)) return nullptr;
        auto& record = m_entity_table[id.index];
        auto& a = find_archetype(record.archetype);
        return a->get_component<T>(record.index);
    }

    const ComponentProvider&
    component_provider() const { return m_provider; }

    const ArchetypeTable&
    archetype_table() const { return m_archetypes; }

    const ComponentMetadataTable&
    component_metadata() const { return m_component_metadata; }

    size_t size() { return m_size; }

private:
    ComponentProvider m_provider;
    ArchetypeTable m_archetypes;
    ComponentMetadataTable m_component_metadata;
    std::vector<EntityRecord> m_entity_table;
    ent_idx_t m_free_head;
    size_t m_size;

    // Returns the archetype matching fingerprint. Creates it if needed.
    Archetype &find_or_create_archetype(const ArchetypeFingerprint &fingerprint) {
        auto found = m_archetypes.find(fingerprint);
        if (found != m_archetypes.end()) return found->second;

        m_archetypes.emplace(fingerprint, m_provider);
        auto &a = m_archetypes.at(fingerprint);

        for (auto &&type_id: fingerprint.type_ids) {
            a.add_component(type_id, m_component_metadata[type_id]);
        }

        return a;
    }

    void remove_entity_data_from_archetype(Archetype &a, ent_idx_t idx) {
        ent_idx_t back_ptr = a.remove_swap(idx);
        if (back_ptr == ENT_IDX_NONE) return;       // no swap
        m_entity_table[back_ptr].index = idx;
    }

    Archetype* find_archetype(ent_archidx_t idx) {
        for (auto&& a: m_archetypes) {
            if (a.second.uuid() == idx) {
                return &a.second;
                break;
            }
        }
        return nullptr;
    }
};

}