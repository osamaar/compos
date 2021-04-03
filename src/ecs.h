#pragma once

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <cstdio>


namespace ecs {

using ComponentStorage = void *;

enum ComponentTypeTag {
    Geometry, Visual, UserController
};

template <typename T>
struct ComponentBase {
    static const ComponentTypeTag type_tag;
};

struct CompGeometry : public ComponentBase<CompGeometry> {

};

struct CompVisual : public ComponentBase<CompVisual> {
    static const ComponentTypeTag type_tag;

};

struct CompUserController : public ComponentBase<CompUserController> {
    static const ComponentTypeTag type_tag;

};

// Componenet subclasses type tag definitions
const ComponentTypeTag CompGeometry::type_tag = ComponentTypeTag::Geometry;
const ComponentTypeTag CompVisual::type_tag = ComponentTypeTag::Visual;
const ComponentTypeTag CompUserController::type_tag = ComponentTypeTag::UserController;
// --------------------------------------------

// struct Archetype {
//     struct Record {
//         ComponentTypeTag type_tag;
//         ComponentStorage component_store;
//     };

//     std::vector<Record> component_collection;

//     template <typename T>
//     void add_type(std::vector<T> *comp_vec) {
//         component_collection.emplace_back(Record{ T::type_tag, comp_vec });
//     }

//     template <typename T>
//     std::vector<T> *get_component_vector() {
//         for(auto &&record : component_collection) {
//             if (record.type_tag != T::type_tag) continue;
//             return static_cast<std::vector<T>*>(record.component_store);
//         }
//         return nullptr;
//     }
// };

// class ComponentContainer {
// public:
//     template <typename T>
//     ComponentContainer() : m_vec{ std::vector<T> } { }

//     template <typename T>
//     void get_vec() {
//         return static_cast<T>(m_vec);
//     }

// private:
//     void *m_vec;
// };

struct VecWrapperBase {
    virtual ~VecWrapperBase() = default;
};

template <typename T>
struct VecWrapper: public VecWrapperBase {
    std::vector<T> inner;
    virtual ~VecWrapper() { printf("deleting VecWrapper: %llu\n", typeid(T).hash_code()
    );}
};

class ComponentProvider {
public:
    template <typename T>
    std::vector<T> *make_component_store() {
        VecWrapper<std::vector<T>> *type_container = get_type_container<T>();
        if (!type_container) {
            type_container = new VecWrapper<std::vector<T>>;
            m_type_containers[std::type_index(typeid(T))] = type_container;
        }
        
        type_container->inner.emplace_back();
        return static_cast<std::vector<T>*>(&type_container->inner.back());
    }
    
    ~ComponentProvider() {
        for (auto &&iter: m_type_containers) {
            delete iter.second;
        }
    }
private:
    std::unordered_map<std::type_index, VecWrapperBase *> m_type_containers;

    template <typename T>
    VecWrapper<std::vector<T>> *get_type_container() {
        auto iter = m_type_containers.find(std::type_index(typeid(T)));
        if (iter == m_type_containers.end()) return nullptr;
        return static_cast<VecWrapper<std::vector<T>>*>(iter->second);
    }
};

/*
    Low-> |    16b    |    16b     |       32b       | <-High
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

class Archetype {
public:
    Archetype(ComponentProvider &provider) : m_provider(provider) { }
    template <typename T>
    void register_component() {
        auto iter = components.find(std::type_index(typeid(T)));
        if (iter != components.end()) return;   // already registered
        components[std::type_index(typeid(T))] = m_provider.make_component_store<T>();
    }

    template <typename T>
    std::vector<T> *get_component_vector() {
        auto iter = components.find(std::type_index(typeid(T)));
        if (iter == components.end()) return nullptr;
        return static_cast<std::vector<T>*>(iter->second);
    }
private:
    std::unordered_map<std::type_index, void*> components;
    ComponentProvider &m_provider;
};

class ComponentManager {
public:
    ComponentManager() = default;
    ~ComponentManager() = default;
private:
    ComponentProvider m_provider;
    std::vector<Archetype> m_archetypes;
};

}