#include "CollisionManager.h"

#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interaction::NiSurface
{
    namespace
    {
        constexpr std::uint32_t TIMING_LOG_INTERVAL{ 30 };
        constexpr float MOTION_FILTER_TIME{ 0.05f };
        constexpr float VELOCITY_FILTER_TIME{ 0.25f };
        constexpr float MOTION_STATE_RETENTION{ 0.2f };
        constexpr float MINIMUM_SAMPLE_TIME{ 0.0001f };
    }

    Scene::Scene(const std::vector<RE::Actor*>& a_positions, const Registry::Scene* a_scene)
    {
        positions.reserve(a_positions.size());
        for (std::size_t i = 0; i < a_positions.size(); ++i) {
            positions.emplace_back(a_positions[i], a_scene->GetNthPosition(i)->data.GetSex().get());
        }
    }

    bool Scene::VisitPositions(const std::function<bool(const ActorState&)>& a_visitor) const
    {
        std::scoped_lock lock{ _mutex };
        for (const auto& position : positions) {
            if (a_visitor(position)) {
                return true;
            }
        }
        return false;
    }

    void Scene::UpdateInteractions(float a_delta, bool a_drawCollision)
    {
        std::unique_lock lock{ _mutex, std::defer_lock };
        if (!lock.try_lock()) {
            return;
        }
        std::vector<ActorState::Frame> frames;
        frames.reserve(positions.size());
        for (auto& position : positions) {
            frames.emplace_back(position);
        }
        if (a_drawCollision) {
            auto& debugDraw = Interface::SceneHUD::GetSingleton().GetDebugDraw();
            for (const auto& frame : frames) {
                if (frame.mouthOpening) {
                    debugDraw.AddRing(frame.mouthOpening->center, frame.mouthOpening->right, frame.mouthOpening->up, frame.mouthOpening->radius);
                }
                if (frame.vaginalOpening) {
                    debugDraw.AddRing(frame.vaginalOpening->center, frame.vaginalOpening->right, frame.vaginalOpening->up, frame.vaginalOpening->radius);
                }
                if (frame.analOpening) {
                    debugDraw.AddRing(frame.analOpening->center, frame.analOpening->right, frame.analOpening->up, frame.analOpening->radius);
                }
                for (const auto& shaft : frame.state.geometry.shafts) {
                    if (const auto* collisionShape = shaft.GetCollisionShape()) {
                        // Draw the same tapered segment chain consumed by the opening collision test.
                        for (std::size_t section = 1; section < collisionShape->sections.size(); ++section) {
                            debugDraw.AddTaperedCapsule(collisionShape->sections[section - 1].center, collisionShape->sections[section].center,
                                collisionShape->sections[section - 1].radius, collisionShape->sections[section].radius);
                        }
                        if (!collisionShape->sections.empty()) {
                            debugDraw.AddTaperedCapsule(collisionShape->sections.back().center, collisionShape->tip, collisionShape->sections.back().radius, 0.0f);
                        }
                    }
                }
            }
        }
        for (const auto& source : frames) {
            DetectShaftInteractions(frames, source);
            DetectVaginalInteractions(frames, source);
            DetectGeneralInteractions(frames, source);
        }
        static std::uint32_t velocityLogFrame = 0;
        const bool logVelocity = velocityLogFrame++ % TIMING_LOG_INTERVAL == 1;
        for (std::size_t i = 0; i < positions.size(); ++i) {
            auto& position = positions[i];
            for (auto& [_, state] : position.motionStates) {
                state.elapsed += std::max(a_delta, 0.0f);
            }

            const std::set<Interaction> detected{ frames[i].interactions.begin(), frames[i].interactions.end() };
            std::set<Interaction> updated;
            for (auto interaction : detected) {
                const auto key = std::tuple{ interaction.partner->GetFormID(), interaction.action, interaction.motionSource };
                auto [where, inserted] = position.motionStates.try_emplace(key, ActorState::MotionState{ interaction.motion, interaction.motionScale });
                auto& state = where->second;
                if (!inserted && state.elapsed > MINIMUM_SAMPLE_TIME && state.elapsed <= MOTION_STATE_RETENTION) {
                    const float motionAlpha = 1.0f - std::exp(-state.elapsed / MOTION_FILTER_TIME);
                    const auto filteredPosition = state.position + (interaction.motion - state.position) * motionAlpha;
                    state.scale += (interaction.motionScale - state.scale) * motionAlpha;
                    const float rawVelocity = filteredPosition.GetDistance(state.position) * state.scale / state.elapsed;
                    if (std::isfinite(rawVelocity)) {
                        const float velocityAlpha = 1.0f - std::exp(-state.elapsed / VELOCITY_FILTER_TIME);
                        state.velocity += (rawVelocity - state.velocity) * velocityAlpha;
                    }
                    state.position = filteredPosition;
                } else if (!inserted && state.elapsed > MOTION_STATE_RETENTION) {
                    state.position = interaction.motion;
                    state.scale = interaction.motionScale;
                    state.velocity = 0.0f;
                }
                state.elapsed = 0.0f;
                interaction.velocity = state.velocity;
                updated.insert(std::move(interaction));
            }
            position.interactions = std::move(updated);

            std::erase_if(position.motionStates, [](const auto& a_entry) {
                return a_entry.second.elapsed > MOTION_STATE_RETENTION;
            });

            // Temporary interaction validation; remove after collision behavior is verified.
            if (logVelocity) {
                for (const auto& interaction : positions[i].interactions) {
                    logger::info("NiSurface Interaction: actor={}, partner={}, action={}, source={}, distance={:.2f}, motion=({:.3f}, {:.3f}, {:.3f}), scale={:.2f}, velocity={:.3f}",
                        position.actor->GetName(), interaction.partner->GetName(), magic_enum::enum_name(interaction.action), interaction.motionSource,
                        interaction.distance, interaction.motion.x, interaction.motion.y, interaction.motion.z, interaction.motionScale, interaction.velocity);
                }
            }
        }
    }

    void Scene::DetectShaftInteractions(std::vector<ActorState::Frame>& a_frames, const ActorState::Frame& a_source)
    {
        if (a_source.state.sex.any(Registry::Sex::Female)) {
            return;
        }
        for (const auto& shaft : a_source.state.geometry.shafts) {
            for (auto& target : a_frames) {
                if (target.DetectShaftHead(a_source, shaft)) {
                    break;
                }
                if (target.DetectShaftHand(a_source, shaft)) {
                    break;
                }
                if (a_source == target) {
                    continue;
                }
                if (target.DetectShaftCrotch(a_source, shaft)) {
                    break;
                }
                target.DetectShaftFoot(a_source, shaft);
            }
        }
    }

    void Scene::DetectVaginalInteractions(std::vector<ActorState::Frame>& a_frames, const ActorState::Frame& a_source)
    {
        if (a_source.state.sex.any(Registry::Sex::Male)) {
            return;
        }
        for (auto& target : a_frames) {
            if (a_source != target) {
                target.DetectVaginalContact(a_source);
            }
            target.DetectVaginalOral(a_source);
            target.DetectVaginalLimb(a_source);
        }
    }

    void Scene::DetectGeneralInteractions(std::vector<ActorState::Frame>& a_frames, const ActorState::Frame& a_source)
    {
        for (auto& target : a_frames) {
            if (a_source != target) {
                target.DetectKissing(a_source);
            }
            target.DetectToeSucking(a_source);
            target.DetectAnimObjectFace(a_source);
        }
    }

    void Manager::OnFrameUpdate(float a_delta)
    {
        std::scoped_lock lock{ _mutex };
        static std::uint32_t frame = 0;
        const bool logTiming = frame++ % TIMING_LOG_INTERVAL == 1;
        const auto start = std::chrono::high_resolution_clock::now();
        auto& debugDraw = Interface::SceneHUD::GetSingleton().GetDebugDraw();
        debugDraw.BeginFrame();
        const auto* linkedThread = Interface::SceneHUD::GetSingleton().GetLinkedThread();
        for (auto&& [id, scene] : scenes) {
            scene->UpdateInteractions(a_delta, linkedThread && id == linkedThread->GetFormID());
        }
        debugDraw.Publish();
        if (logTiming && !scenes.empty()) {
            const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start);
            logger::info("NiSurface Interaction: Frame -> {:.2f}ms", elapsed.count());
        }
    }

    std::shared_ptr<Scene> Manager::Register(RE::FormID a_id, std::vector<RE::Actor*> a_positions, const Registry::Scene* a_scene) noexcept
    {
        try {
            const auto where = std::ranges::find(scenes, a_id, [](const auto& a_entry) { return a_entry.first; });
            if (where != scenes.end()) {
                logger::info("NiSurface Interaction: Object with ID {:X} already registered; resetting NiSurface for scene", a_id);
                std::swap(*where, scenes.back());
                scenes.pop_back();
            }
            const auto start = std::chrono::high_resolution_clock::now();
            auto process = std::make_shared<Scene>(a_positions, a_scene);
            const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start);
            logger::info("NiSurface Interaction: Initialization -> {:.2f}ms", elapsed.count());
            return scenes.emplace_back(a_id, std::move(process)).second;
        } catch (const std::exception& e) {
            logger::error("NiSurface Interaction: Failed to register NiSurface for scene: {}", e.what());
            return nullptr;
        } catch (...) {
            logger::error("NiSurface Interaction: Failed to register NiSurface for scene: unknown error");
            return nullptr;
        }
    }

    void Manager::Unregister(RE::FormID a_id) noexcept
    {
        const auto where = std::ranges::find(scenes, a_id, [](const auto& a_entry) { return a_entry.first; });
        if (where == scenes.end()) {
            logger::error("NiSurface Interaction: No object registered using ID {:X}", a_id);
            return;
        }
        scenes.erase(where);
    }

}  // namespace Thread::Interaction::NiSurface
