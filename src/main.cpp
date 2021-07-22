#include "ecs.h"
#include <assert.h>
#include <vector>
#include <typeindex>

int I = 0;

int fn() {
    static int i = I++;
    return i;
}

// int test() {
//     using namespace ecs;

//     ComponentProvider provider;
//     Archetype arche {provider};
//     Archetype arche1 {provider};

//     arche.add_component<CompGeometry>();
//     arche.add_component<CompGeometry>();
//     arche.add_component<CompVisual>();

//     arche1.add_component<CompGeometry>();
//     arche1.add_component<CompVisual>();
//     arche1.add_component<CompUserController>();

//     typeid_t c0 = CompGeometry::uuid();
//     typeid_t c1 = CompVisual::uuid();
//     typeid_t c2 = CompUserController::uuid();

//     typeid_t compcount = UUID<Component>::id_counter;

//     typeid_t a0 = arche.uuid();
//     typeid_t a1 = arche1.uuid();

//     typeid_t archecount = UUID<Archetype>::id_counter;
    
//     return 0;
// }

// struct Component {
// };
// template <typename T>
// struct ComponentBase: public Component {
//     static const typeid_t uuid() {
//         return UUID<Component>::get<T>();
//     }

//     // template <typename... Args>
//     // static void construct(void *memory, Args... args) {
//     //     ::new (memory) T{std::forward(args)...};
//     // }
// };

// struct CompGeometry : public ComponentBase<CompGeometry> {
struct CompGeometry {
    int x; int y;
};

struct CompVisual {

};

struct CompUserController {

};

int main() {
    using namespace ecs;

    ComponentManager cm;
    cm.register_component<CompGeometry>();
    cm.register_component<CompVisual>();
    cm.register_component<CompUserController>();

    EntityID ent0 = cm.create_entity<CompGeometry, CompVisual>();
    EntityID ent1 = cm.create_entity<CompUserController, CompGeometry>();

    EntityID ent2 = cm.create_entity<CompGeometry, CompVisual>();
    EntityID ent3 = cm.create_entity<CompUserController, CompGeometry>();
    // auto ent3 = cm.create_entity<CompGeometry, CompVisual>();
    // auto ent4 = cm.create_entity<CompGeometry, CompVisual>();
    return 0;
}