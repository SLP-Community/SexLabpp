#pragma once

namespace Thread::Interaction::NiSurface
{
    struct OpeningShape
    {
        RE::NiPoint3 center;
        RE::NiPoint3 deep;
        RE::NiPoint3 axis;
        RE::NiPoint3 right;
        RE::NiPoint3 up;
        float radius{ 0.0f };
    };

    struct ShaftSection
    {
        RE::NiPoint3 center;
        float radius{ 0.0f };
    };

    struct ShaftShape
    {
        std::vector<ShaftSection> sections;
        RE::NiPoint3 tip;
    };
}
