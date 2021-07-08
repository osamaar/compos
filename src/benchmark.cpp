#include "ecs.h"
#include "timing.h"
#include <cstdio>

struct CompGeometry { int x; int y; };
struct CompVisual { };
struct CompUserController { };

int main() {
    tickcount_t start = timing_getticks();


    using namespace ecs;

    ComponentManager cm;
    cm.register_component<CompGeometry>();
    cm.register_component<CompVisual>();
    cm.register_component<CompUserController>();

    EntityID ent0 = cm.create_entity<CompGeometry, CompVisual, CompUserController>();

    for (int i = 0; i < 1000000; ++i) {
        EntityID ent0 = cm.create_entity<
            CompGeometry,
            CompVisual,
            CompUserController
        >();

    }

    tickcount_t end = timing_getticks();
    int64_t t = timing_timediff(start, end);
    printf("Elapsed Time: %lldus (%.3fs)\n", t, t/1000000.);

    return 0;
}