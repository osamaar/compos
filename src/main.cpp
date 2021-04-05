#include "ecs.h"
#include <assert.h>
#include <vector>
#include <typeindex>

int I = 0;

int fn() {
    static int i = I++;
    return i;
}

int main() {
    using namespace ecs;

    ComponentProvider provider;
    Archetype arche {provider};
    Archetype arche1 {provider};

    arche.add_component<CompGeometry>();
    arche.add_component<CompGeometry>();
    arche.add_component<CompVisual>();

    arche1.add_component<CompGeometry>();
    arche1.add_component<CompVisual>();
    arche1.add_component<CompUserController>();

    IDType c0 = CompGeometry::uuid();
    IDType c1 = CompVisual::uuid();
    IDType c2 = CompUserController::uuid();

    IDType compcount = UUID<Component>::id_counter;

    IDType a0 = arche.uuid();
    IDType a1 = arche1.uuid();

    IDType archecount = UUID<Archetype>::id_counter;
    
    return 0;
}