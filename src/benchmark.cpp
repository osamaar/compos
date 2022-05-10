#include "compos/compos.h"
#include "compos_fmt.h"
#include "timing.h"
// #include <cstdio>
#include <fmt/format.h>
#include <crash.h>
#include <vector>

#define N 100000
#define SEED 0xdeadbeef

struct CompGeometry { int x; int y; };
struct CompVisual { };
struct CompUserController { };

ecs::EntityID e0(ecs::EntityManager& cm) { return cm.create_entity<CompGeometry, CompVisual>(); }
ecs::EntityID e1(ecs::EntityManager& cm) { return cm.create_entity<CompGeometry, CompVisual, CompUserController>(); }
ecs::EntityID e2(ecs::EntityManager& cm) { return cm.create_entity<CompGeometry>(); }
ecs::EntityID e3(ecs::EntityManager& cm) { return cm.create_entity<CompGeometry, CompUserController>(); }
ecs::EntityID e4(ecs::EntityManager& cm) { return cm.create_entity<CompVisual, CompUserController>(); }

int main() {
    using namespace ecs;

    tickcount_t start, end;
    int64_t t;

    CrashState rstate = crash_seed(SEED);
    typedef EntityID(*EFunc)(EntityManager&);

    EFunc entity_func[] = {e0, e1, e2, e3, e4};
    std::vector<EntityID> entities;
    entities.reserve(N);

    start = timing_getticks();

    EntityManager cm;
    cm.register_component<CompGeometry>();
    cm.register_component<CompVisual>();
    cm.register_component<CompUserController>();

    for (int i = 0; i < N; ++i) {
        int32_t idx = crash_choose_int32(&rstate, 0, 5);
        EntityID e = entity_func[idx](cm);
        entities.push_back(e);
        while(0);
    }

    end = timing_getticks();
    t = timing_timediff(start, end);

    fmt::print("Created {} entities in: {}us ({:.3}s)\n", N, t, t/1000000.);
    fmt::print("Have: {} entities\n", cm.size());

    start = timing_getticks();

    int n_del = 0;
    for (int i = 0; i < N; ++i) {
        bool roll = crash_roll_with_prob(&rstate, 0.1);
        if (roll) {
          cm.delete_entity(entities[i]);
          n_del++;
        }
        while(0);
    }

    end = timing_getticks();
    t = timing_timediff(start, end);

    fmt::print("\n");
    fmt::print("Deleted {} entities in: {}us ({:.3}s)\n", n_del, t, t/1000000.);
    fmt::print("Have: {} entities\n", cm.size());

    fmt::print("\n");
    fmt::print("cm: {}\n", cm);

    for (auto&& a: cm.archetype_table()) {
      fmt::print("Archetype {}\n", a.second);
    }

    return 0;
}