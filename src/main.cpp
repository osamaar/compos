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

//     IDType c0 = CompGeometry::uuid();
//     IDType c1 = CompVisual::uuid();
//     IDType c2 = CompUserController::uuid();

//     IDType compcount = UUID<Component>::id_counter;

//     IDType a0 = arche.uuid();
//     IDType a1 = arche1.uuid();

//     IDType archecount = UUID<Archetype>::id_counter;
    
//     return 0;
// }

int main() {
    using namespace ecs;

    ComponentManager cm;
    cm.register_component<CompGeometry>();
    cm.register_component<CompVisual>();
    cm.register_component<CompUserController>();

    EntityID ent0 = cm.create_entity<CompGeometry, CompVisual>();
    EntityID ent1 = cm.create_entity<CompUserController, CompGeometry>();

    EntityID ent2 = cm.create_entity<CompGeometry, CompVisual>();
    // auto ent3 = cm.create_entity<CompGeometry, CompVisual>();
    // auto ent4 = cm.create_entity<CompGeometry, CompVisual>();
    return 0;
}