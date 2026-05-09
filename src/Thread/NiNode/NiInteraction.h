#pragma once

#include "NiActor.h"
#include "NiDescriptor.h"

namespace Thread::NiNode
{
    struct NiInteraction
    {
        NiInteraction() = default;
        NiInteraction(std::unique_ptr<INiDescriptor> a_descriptor, float a_velocity) :
          descriptor(std::move(a_descriptor)), velocity(a_velocity) {}
        ~NiInteraction() = default;
        NiInteraction(NiInteraction&&) = default;
        NiInteraction& operator=(NiInteraction&&) = default;
        NiInteraction(const NiInteraction&) = delete;
        NiInteraction& operator=(const NiInteraction&) = delete;

        NiType::Type GetType() const { return descriptor ? descriptor->GetType() : NiType::Type::None; }

      public:
        std::unique_ptr<INiDescriptor> descriptor{ nullptr };
        float velocity{ 0.0f };
        // historical data for hysteresis
        float timeActive{ 0.0f };
        bool active{ false };
    };

    struct NiInteractionCluster
    {
        NiInteractionCluster() = default;
        ~NiInteractionCluster() = default;
        NiInteractionCluster(NiInteractionCluster&&) = default;
        NiInteractionCluster& operator=(NiInteractionCluster&&) = default;
        NiInteractionCluster(const NiInteractionCluster&) = delete;
        NiInteractionCluster& operator=(const NiInteractionCluster&) = delete;

        std::vector<NiInteraction> interactions{};

        bool IncludesType(NiType::Type type) const;
        bool IsValid() const { return !interactions.empty(); }
        bool IsBinary() const { return interactions.size() == 1; }
        bool IsSoftmax() const { return interactions.size() > 1; }
        NiType::Cluster GetClusterType() const;
        NiInteraction* ApplySoftmax();
    };

    NiInteractionCluster EvaluateCrotchInteractions(const NiMotion& a_motionA, const NiMotion& a_motionB);
    NiInteractionCluster EvaluateHeadInteractions(const NiMotion& a_motionA, const NiMotion& a_motionB);

    NiInteractionCluster EvaluateKissingCluster(const NiMotion& a_motionA, const NiMotion& a_motionB);

    NiInteraction EvaluateFootJob(const NiMotion& a_motionA, const NiMotion& a_motionB);
    NiInteraction EvaluateHandJob(const NiMotion& a_motionA, const NiMotion& a_motionB);
    NiInteraction EvaluateFacial(const NiMotion& a_motionA, const NiMotion& a_motionB);
    NiInteraction EvaluateAnimObjFace(const NiMotion& a_motionA, const NiMotion& a_motionB);
    NiInteraction EvaluateFootWorship(const NiMotion& a_motionA, const NiMotion& a_motionB);
    NiInteraction EvaluateBoobjob(const NiMotion& a_motionA, const NiMotion& a_motionB);

}  // namespace Thread::NiNode
