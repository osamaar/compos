#pragma once

#include "untyped_vector.h"
#include "type_alias.h"
#include "const.h"
#include "component.h"
#include "uuid.h"
#include <vector>
#include <map>
#include <set>

namespace compos {
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
    void append(std::initializer_list<typeid_t> ids) { type_ids.insert(ids); }
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
        for (auto &&comp: m_components) {
            UntypedVector &vec = *m_provider.get_component_store(comp.first, comp.second);
            vec.emplace_back_default();
        }

        m_back_ptr.push_back(table_index);
        m_size++;
        return (m_size - 1);
    }

    size_t size() const { return m_size; }

    void add_component(typeid_t type_id, const UntypedVector::TypeMetadata &metadata) {
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

    template <typename T>
    T* get_component(ent_idx_t idx) {
        auto type_id = UUID<Component>::get<T>();
        auto vec = get_component_vector(type_id);
        if (!vec) return nullptr;
        return vec.get(idx);
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
    ent_idx_t m_size;
};

}   //namespace compos