#pragma once

#include "ActorState.h"
#include "Registry/Define/Animation.h"

namespace Thread::Interaction::NiSurface
{
    struct Scene
    {
        friend class Manager;

      public:
        Scene(const std::vector<RE::Actor*>& a_positions, const Registry::Scene* a_scene);

        bool VisitPositions(const std::function<bool(const ActorState&)>& a_visitor) const;

      private:
        void UpdateInteractions(float a_delta, bool a_drawCollision);
        void DetectShaftInteractions(std::vector<ActorState::Frame>& a_frames, const ActorState::Frame& a_source);
        void DetectVaginalInteractions(std::vector<ActorState::Frame>& a_frames, const ActorState::Frame& a_source);
        void DetectGeneralInteractions(std::vector<ActorState::Frame>& a_frames, const ActorState::Frame& a_source);

        std::vector<ActorState> positions;
        mutable std::mutex _mutex{};
    };

    class Manager
    {
      public:
        static void OnFrameUpdate(float a_delta);

        static std::shared_ptr<Scene> Register(RE::FormID a_id, std::vector<RE::Actor*> a_positions, const Registry::Scene* a_scene) noexcept;
        static void Unregister(RE::FormID a_id) noexcept;

      private:
        static inline std::mutex _mutex{};
        static inline std::vector<std::pair<RE::FormID, std::shared_ptr<Scene>>> scenes;
    };

}  // namespace Thread::Interaction::NiSurface
