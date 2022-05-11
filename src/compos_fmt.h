#pragma once

#include "compos/compos.h"
#include <fmt/format.h>

template <>
struct fmt::formatter<compos::EntityManager> : fmt::formatter<fmt::string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(const compos::EntityManager &cm, FormatContext &ctx) -> decltype(ctx.out()) {
        string_view name = "unknown";
        fmt::format_to(ctx.out(), "<");
        fmt::format_to(
            ctx.out(),
            "EntityManager: {} Archetypes ( ", cm.archetype_table().size()
        );
        
        for (auto&& a : cm.archetype_table()) {
            fmt::format_to(ctx.out(), "{} ", a.second.uuid());
        }

        fmt::format_to(ctx.out(), ")");
        return fmt::format_to(ctx.out(), ">");
    }
};

template <>
struct fmt::formatter<compos::Archetype> : fmt::formatter<fmt::string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(const compos::Archetype &a, FormatContext &ctx) -> decltype(ctx.out()) {
        string_view name = "unknown";
        fmt::format_to(ctx.out(), "<");
        fmt::format_to(ctx.out(), "Archetype {}: {} Components ( ", a.uuid(), a.fingerprint().size());

        for (auto&& c : a.fingerprint().type_ids) {
            fmt::format_to(ctx.out(), "{} ", c);
        }

        fmt::format_to(ctx.out(), ")");
        fmt::format_to(ctx.out(), " - {} entities", a.size());
        return fmt::format_to(ctx.out(), ">");
    }
};
