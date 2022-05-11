compos
===
Motivation
---
TODO

Intention
---
`compos` Is a library, not a framework. Components, the main focus and building block of the library, can be any user type. Systems simply exist to provide the ability to iterate over certain sets of components. the library doesn't impose any architectural choices on the user beyond that.

I will update this readme with more concrete description when available.

`compos` main features:

- Provides stable entity indices through an indirection (aka. Generational Indices).
- Allows for linear time component iteration (contiguous memory, segregated into archetypes), and constant time entity indexing with some constant overhead.
- Aims to maximize CPU cache utilization by storing components in contiguous blocks of memory as much as possible and provide easy-to-use, high-performance indexing over entities with certain sets of components.

All code here is subject to change. In particular, the use of `map` and `unordered_map` is temporary. They will be replaced later with cache-friendly alternatives.

Usage
---
Entities and components can be created and destroyed like this:
```
    using namespace compos;

    EntityManager cm;
    cm.register_component<CompGeometry>();
    cm.register_component<CompVisual>();
    cm.register_component<CompUserController>();

    EntityID ent0 = cm.create_entity<CompGeometry, CompVisual>();
    EntityID ent1 = cm.create_entity<CompUserController, CompGeometry>();

    EntityID ent2 = cm.create_entity<CompGeometry, CompVisual>();
    EntityID ent3 = cm.create_entity<CompUserController, CompGeometry>();

    bool r = cm.delete_entity(ent0);
    cm.remove_component<CompVisual>(ent0);
```