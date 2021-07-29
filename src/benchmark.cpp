#include "ecs.h"
#include "ecs_fmt.h"
#include "timing.h"
// #include <cstdio>
#include <fmt/format.h>
#include <crash.h>

#define N 100000
#define SEED 0xdeadbeef

struct CompGeometry { int x; int y; };
struct CompVisual { };
struct CompUserController { };

void e0(ecs::ComponentManager& cm) { cm.create_entity<CompGeometry, CompVisual>(); }
void e1(ecs::ComponentManager& cm) { cm.create_entity<CompGeometry, CompVisual, CompUserController>(); }
void e2(ecs::ComponentManager& cm) { cm.create_entity<CompGeometry>(); }
void e3(ecs::ComponentManager& cm) { cm.create_entity<CompGeometry, CompUserController>(); }
void e4(ecs::ComponentManager& cm) { cm.create_entity<CompVisual, CompUserController>(); }

int main() {
    using namespace ecs;

    CrashState rstate = crash_seed(SEED);
    typedef void(*EFunc)(ComponentManager&);

    EFunc entity_func[] = {e0, e1, e2, e3, e4};

    tickcount_t start = timing_getticks();

    ComponentManager cm;
    cm.register_component<CompGeometry>();
    cm.register_component<CompVisual>();
    cm.register_component<CompUserController>();

    EntityID ent0 = cm.create_entity<CompGeometry, CompVisual, CompUserController>();

    for (int i = 0; i < N; ++i) {
        int32_t idx = crash_choose_int32(&rstate, 0, 5);
        entity_func[idx](cm);

        // EntityID ent0 = cm.create_entity<
        //     CompGeometry,
        //     CompVisual
        // >();

        // EntityID ent1 = cm.create_entity<
        //     CompVisual,
        //     CompUserController
        // >();

        while(0);
    }

    tickcount_t end = timing_getticks();
    int64_t t = timing_timediff(start, end);
    // printf("Elapsed Time: %lldus (%.3fs)\n", t, t/1000000.);
    fmt::print("Elapsed Time: {}us ({:.3}s)\n", t, t/1000000.);

    fmt::print("\n", cm);
    fmt::print("created {} entities\n", N);
    fmt::print("cm: {}\n", cm);

    for (auto&& a: cm.archetype_table()) {
      fmt::print("Archetype {}\n", a.second);
    }

    return 0;
}