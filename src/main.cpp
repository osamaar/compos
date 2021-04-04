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

    // std::vector<CompGeometry> geo;
    // std::vector<CompVisual> vis;

    ComponentProvider provider;
    Archetype arche {provider};
    Archetype arche1 {provider};
    // arche.add_type<CompGeometry>(&geo);
    // arche.add_type<CompVisual>(&vis);

    arche.register_component<CompGeometry>();
    arche.register_component<CompGeometry>();
    arche.register_component<CompVisual>();

    arche1.register_component<CompGeometry>();
    arche1.register_component<CompVisual>();
    arche1.register_component<CompUserController>();

    assert(
        std::type_index(typeid(arche.get_component_vector<CompGeometry>()))
        == std::type_index(typeid(std::vector<CompGeometry>*))
    );

    assert(
        std::type_index(typeid(arche.get_component_vector<CompVisual>())) 
        == std::type_index(typeid(std::vector<CompVisual>*))
    );

    assert(arche.get_component_vector<CompUserController>() == nullptr);

    IDType c0 = CompGeometry::uuid();
    IDType c1 = CompVisual::uuid();
    IDType c2 = CompUserController::uuid();

    IDType compcount = UUID<Component>::id_counter;

    IDType a0 = arche.uuid();
    IDType a1 = arche1.uuid();

    IDType archecount = UUID<Archetype>::id_counter;
    
    return 0;
}