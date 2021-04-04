#pragma once

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <cstdio>


namespace ecs {

using ComponentStorage = void *;
using IDType = uint64_t;

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

class Archetype {
public:
    Archetype(ComponentProvider &provider)
        : m_components {}
        , m_provider {provider}
        , m_uuid {UUID<Archetype>::generate()}
    {
    }

    template <typename T>
    void register_component() {
        auto iter = m_components.find(std::type_index(typeid(T)));
        if (iter != m_components.end()) return;   // already registered
        m_components[std::type_index(typeid(T))] = m_provider.make_component_store<T>();
    }

    template <typename T>
    std::vector<T> *get_component_vector() {
        auto iter = m_components.find(std::type_index(typeid(T)));
        if (iter == m_components.end()) return nullptr;
        return static_cast<std::vector<T>*>(iter->second);
    }

    IDType uuid() {
        return m_uuid;
    };

private:
    std::unordered_map<std::type_index, void*> m_components;
    ComponentProvider &m_provider;
    IDType m_uuid;
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