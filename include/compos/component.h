#pragma once

#include "const.h"
#include "type_alias.h"
#include "untyped_vector.h"
#include <unordered_map>

namespace ecs {

// A marker class to use for UUID counting.
class Component {};


class ComponentProvider {
public:
    size_t make_component_store(typeid_t type_id, const UntypedVector::TypeMetadata &metadata) {
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

private:
    using TypeContainer = std::vector<UntypedVector>;
    std::unordered_map<typeid_t, TypeContainer> m_type_containers;
};

}   // ecs namespace
