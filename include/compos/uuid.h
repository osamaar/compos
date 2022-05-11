#pragma once

#include "type_alias.h"

namespace compos {

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

template<typename T>
typeid_t UUID<T>::id_counter = 0;

}   // namespace compos