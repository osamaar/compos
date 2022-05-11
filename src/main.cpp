#include "compos/compos.h"
#include <assert.h>
#include <vector>
#include <typeindex>

#define UNUSED(expr) do { (void)(expr); } while (0)
struct CompGeometry {
    int x; int y;
};

struct CompVisual {

};

struct CompUserController {

};

int main() {
    using namespace compos;

    EntityManager cm;
    cm.register_component<CompGeometry>();
    cm.register_component<CompVisual>();
    cm.register_component<CompUserController>();

    EntityID ent0 = cm.create_entity<CompGeometry, CompVisual>();
    EntityID ent1 = cm.create_entity<CompUserController, CompGeometry>();
    EntityID ent1_1 = cm.create_entity<CompUserController, CompGeometry>();
    EntityID ent1_2 = cm.create_entity<CompUserController, CompGeometry>();

    EntityID ent2 = cm.create_entity<CompGeometry, CompVisual>();
    EntityID ent3 = cm.create_entity<CompUserController, CompGeometry>();
    // auto ent3 = cm.create_entity<CompGeometry, CompVisual>();
    // auto ent4 = cm.create_entity<CompGeometry, CompVisual>();

    // bool r = cm.delete_entity(ent0);
    cm.remove_component<CompVisual>(ent0);
    EntityID ent00 = cm.create_entity<CompGeometry, CompVisual>();
    bool rr = cm.delete_entity(ent00);
    UNUSED(rr);

    return 0;
}