#pragma once

// CommonLib only declares these base virtuals, we gotta define them here so custom collectors link and reset to Havok's maximum-distance sentinel
inline RE::hkpCdPointCollector::~hkpCdPointCollector() = default;
inline void RE::hkpCdPointCollector::Reset()
{
    earlyOutDistance = std::bit_cast<float>(0x7F7FFFEEu);
}

namespace Util
{
    inline void ForEachObjectInRange(RE::TESObjectREFR* a_center, float a_radius, std::function<RE::BSContainer::ForEachResult(RE::TESObjectREFR*)> a_forEachFunc)
    {
        const auto TES = RE::TES::GetSingleton();
        const auto center_coords = a_center->GetPosition();
        if (const auto interior = TES->interiorCell; interior) {
            interior->ForEachReferenceInRange(center_coords, a_radius, a_forEachFunc);
        } else if (const auto grids = TES->gridCells; grids) {
            auto gridLength = grids->length;
            if (gridLength > 0) {
                float yPlus = center_coords.y + a_radius;
                float yMinus = center_coords.y - a_radius;
                float xPlus = center_coords.x + a_radius;
                float xMinus = center_coords.x - a_radius;
                for (uint32_t x = 0, y = 0; (x < gridLength && y < gridLength); x++, y++) {
                    const auto gridcell = grids->GetCell(x, y);
                    if (gridcell && gridcell->IsAttached()) {
                        auto cellCoords = gridcell->GetCoordinates();
                        if (!cellCoords)
                            continue;
                        float worldX = cellCoords->worldX;
                        float worldY = cellCoords->worldY;
                        if (worldX < xPlus && (worldX + 4096.0) > xMinus && worldY < yPlus && (worldY + 4096.0) > yMinus) {
                            gridcell->ForEachReferenceInRange(center_coords, a_radius, a_forEachFunc);
                        }
                    }
                }
            }
        }
    }

    namespace World
    {
        struct AABB
        {
            RE::NiPoint3 min;
            RE::NiPoint3 max;
        };

        struct ActorBound
        {
            RE::Actor* actor;
            AABB bound;
        };

        inline constexpr float ACTOR_BOUND_PADDING = 2.0f;
        inline constexpr float MINIMUM_TILE_SIZE = 10.0f;
        inline constexpr float MAX_SEARCH_DISTANCE = 1524.0f;
        inline constexpr float WORLD_CAST_GROUND_CLEARANCE = 5.0f;
        inline constexpr float MAX_NAVMESH_DISTANCE_ABOVE = 64.0f;
        inline constexpr float MAX_NAVMESH_DISTANCE_BELOW = 128.0f;
        inline constexpr std::size_t MAX_SEARCH_CANDIDATES = 456;

        inline AABB Translate(const AABB& a_box, const RE::NiPoint3& a_offset)
        {
            return {
                { a_box.min.x + a_offset.x, a_box.min.y + a_offset.y, a_box.min.z + a_offset.z },
                { a_box.max.x + a_offset.x, a_box.max.y + a_offset.y, a_box.max.z + a_offset.z }
            };
        }

        inline bool Intersects(const AABB& a_left, const AABB& a_right)
        {
            return a_left.min.x < a_right.max.x && a_left.max.x > a_right.min.x &&
                   a_left.min.y < a_right.max.y && a_left.max.y > a_right.min.y &&
                   a_left.min.z < a_right.max.z && a_left.max.z > a_right.min.z;
        }

        inline const RE::hkpCollidable* GetCharacterCollidable(RE::bhkCharacterController* a_controller)
        {
            if (!a_controller)
                return nullptr;
            const auto worldObject = reinterpret_cast<RE::hkpWorldObject*>(a_controller->GetRigidBody());
            return worldObject ? worldObject->GetCollidable() : nullptr;
        }

        inline const RE::hkpCollidable* GetRootCollidable(const RE::hkpCdBody* a_body)
        {
            if (!a_body)
                return nullptr;
            while (a_body->parent) {
                a_body = a_body->parent;
            }
            return reinterpret_cast<const RE::hkpCollidable*>(a_body);
        }

        inline RE::hkpWorldObject* GetWorldObject(const RE::hkpCollidable* a_collidable)
        {
            if (!a_collidable || a_collidable->ownerOffset >= 0)
                return nullptr;

            using enum RE::hkpWorldObject::BroadPhaseType;
            switch (static_cast<RE::hkpWorldObject::BroadPhaseType>(a_collidable->broadPhaseHandle.type)) {
            case kEntity:
                return a_collidable->GetOwner<RE::hkpRigidBody>();
            case kPhantom:
                return a_collidable->GetOwner<RE::hkpPhantom>();
            default:
                return nullptr;
            }
        }

        inline std::optional<AABB> BuildCharacterAABB(RE::Actor* a_actor)
        {
            const auto controller = a_actor ? a_actor->GetCharController() : nullptr;
            const auto bhkWorld = controller ? controller->GetHavokWorld() : nullptr;
            if (!controller || !bhkWorld)
                return std::nullopt;

            const RE::BSReadLockGuard lock{ bhkWorld->worldLock };
            const auto collidable = GetCharacterCollidable(controller);
            const auto worldObject = GetWorldObject(collidable);
            const auto motionState = worldObject ? worldObject->GetMotionState() : nullptr;
            if (!collidable || !collidable->shape || !motionState)
                return std::nullopt;

            RE::hkAabb havokBound;
            const auto worldScale = RE::bhkWorld::GetWorldScale();
            collidable->shape->GetAabbImpl(motionState->transform, ACTOR_BOUND_PADDING * worldScale, havokBound);

            alignas(16) float min[4];
            alignas(16) float max[4];
            _mm_store_ps(min, havokBound.min.quad);
            _mm_store_ps(max, havokBound.max.quad);
            const auto worldScaleInverse = RE::bhkWorld::GetWorldScaleInverse();
            return AABB{
                { min[0] * worldScaleInverse, min[1] * worldScaleInverse, min[2] * worldScaleInverse },
                { max[0] * worldScaleInverse, max[1] * worldScaleInverse, max[2] * worldScaleInverse }
            };
        }

        inline std::vector<ActorBound> GatherActorBounds(const RE::NiPoint3& a_center, std::span<RE::Actor* const> a_requiredActors)
        {
            std::vector<ActorBound> bounds;
            const auto addActor = [&](RE::Actor* a_actor, bool a_required) {
                if (!a_actor || std::ranges::contains(bounds, a_actor, &ActorBound::actor))
                    return;

                const auto position = a_actor->GetPosition();
                if (!a_required && std::hypot(position.x - a_center.x, position.y - a_center.y) > MAX_SEARCH_DISTANCE + MINIMUM_TILE_SIZE)
                    return;

                if (const auto bound = BuildCharacterAABB(a_actor))
                    bounds.push_back({ a_actor, *bound });
            };

            addActor(RE::PlayerCharacter::GetSingleton(), true);
            for (const auto actor : a_requiredActors) {
                addActor(actor, true);
            }

            const auto processLists = RE::ProcessLists::GetSingleton();
            if (!processLists)
                return bounds;

            const auto addHandle = [&](const RE::ActorHandle& a_handle) {
                const auto reference = a_handle.get();
                addActor(reference ? reference->As<RE::Actor>() : nullptr, false);
            };

            // Just in case we get everyone's handles. The lower handles are likely too far, but it's a cheap check
            for (const auto& handle : processLists->highActorHandles)
                addHandle(handle);
            for (const auto& handle : processLists->middleHighActorHandles)
                addHandle(handle);
            for (const auto& handle : processLists->middleLowActorHandles)
                addHandle(handle);
            for (const auto& handle : processLists->lowActorHandles)
                addHandle(handle);
            return bounds;
        }

        class PlacementCastCollector final : public RE::hkpCdPointCollector
        {
          public:
            PlacementCastCollector(RE::Actor* a_actor, const RE::hkpCollidable* a_collidable, std::span<const ActorBound> a_actorBounds) :
              actor(a_actor), collidable(a_collidable), actorBounds(a_actorBounds)
            {
                Reset();
            }

            ~PlacementCastCollector() override = default;

            void AddCdPoint(const RE::hkpCdPoint& a_point) override
            {
                const auto rootA = GetRootCollidable(a_point.cdBodyA);
                const auto rootB = GetRootCollidable(a_point.cdBodyB);
                if (rootA != collidable && rootB != collidable) {
                    blocked = true;
                    earlyOutDistance = 0.0f;
                    return;
                }
                const auto other = rootA == collidable ? rootB : rootA;
                if (!other || other == collidable)
                    return;

                const auto worldObject = GetWorldObject(other);
                const auto reference = worldObject ? worldObject->GetUserData() : nullptr;
                const auto hitActor = reference ? reference->As<RE::Actor>() : nullptr;
                if (hitActor && (hitActor == actor || std::ranges::contains(actorBounds, hitActor, &ActorBound::actor)))
                    return;

                alignas(16) float normal[4];
                _mm_store_ps(normal, a_point.contact.separatingNormal.quad);
                const auto normalZ = rootA == collidable ? normal[2] : -normal[2];
                if (normalZ >= 0.7f)
                    return;

                blocked = true;
                earlyOutDistance = 0.0f;
            }

            void Reset() override
            {
                earlyOutDistance = std::numeric_limits<float>::max();
                blocked = false;
            }

            bool blocked{ false };

          private:
            RE::Actor* actor;
            const RE::hkpCollidable* collidable;
            std::span<const ActorBound> actorBounds;
        };

        inline bool HasBlockingWorldCollision(RE::Actor* a_actor, const RE::NiPoint3& a_target, std::span<const ActorBound> a_actorBounds)
        {
            const auto controller = a_actor->GetCharController();
            const auto bhkWorld = controller ? controller->GetHavokWorld() : nullptr;
            const auto havokWorld = bhkWorld ? bhkWorld->GetWorld1() : nullptr;
            if (!controller || !bhkWorld || !havokWorld)
                return true;

            const RE::BSReadLockGuard lock{ bhkWorld->worldLock };
            const auto collidable = GetCharacterCollidable(controller);
            const auto worldObject = GetWorldObject(collidable);
            const auto motionState = worldObject ? worldObject->GetMotionState() : nullptr;
            if (!collidable || !motionState)
                return true;

            const auto actorPosition = a_actor->GetPosition();
            const auto scale = RE::bhkWorld::GetWorldScale();
            // Navmesh and Havok terrain can diverge a bit, so lift the destination to keep the sweep from grazing walkable ground (Might have to mess with this WORLD_CAST_GROUND_CLEARANCE var)
            const RE::hkVector4 offset{
                (a_target.x - actorPosition.x) * scale,
                (a_target.y - actorPosition.y) * scale,
                (a_target.z + WORLD_CAST_GROUND_CLEARANCE - actorPosition.z) * scale,
                0.0f
            };
            RE::hkpLinearCastInput input{};
            input.to = motionState->transform.translation + offset;
            input.maxExtraPenetration = 0.01f;
            input.startPointTolerance = 0.01f;

            PlacementCastCollector collector{ a_actor, collidable, a_actorBounds };
            havokWorld->LinearCast(collidable, input, collector);
            return collector.blocked;
        }

        inline std::optional<RE::NiPoint3> ProjectToNavmesh(RE::Actor* a_actor, const RE::NiPoint3& a_target, float a_maxHorizontalSnap)
        {
            const auto pathing = RE::Pathing::GetSingleton();
            if (!pathing || !a_actor->GetParentCell())
                return std::nullopt;

            RE::BSTSmartPointer<RE::BSPathingCell> pathingCell;
            if (!pathing->GetPathingCell(a_target, a_actor->GetParentCell(), a_actor->GetWorldspace(), pathingCell))
                return std::nullopt;

            RE::FindTriangleForLocationFilterCheckDeltaZ filter;
            filter.maxDistAbove = MAX_NAVMESH_DISTANCE_ABOVE;
            filter.maxDistBelow = MAX_NAVMESH_DISTANCE_BELOW;
            RE::NiPoint3 projected;
            if (!pathing->FindClosestPointOnNavmesh(pathingCell, a_target, filter, projected) ||
                std::hypot(projected.x - a_target.x, projected.y - a_target.y) > a_maxHorizontalSnap)
                return std::nullopt;
            return projected;
        }

        inline bool IntersectsActor(const AABB& a_bound, RE::Actor* a_mover, std::span<const ActorBound> a_actorBounds)
        {
            return std::ranges::any_of(a_actorBounds, [&](const ActorBound& a_actorBound) {
                return a_actorBound.actor != a_mover && Intersects(a_bound, a_actorBound.bound);
            });
        }

        inline std::optional<RE::NiPoint3> FindSafePosition(RE::Actor* a_actor, const RE::NiPoint3& a_center, std::span<const ActorBound> a_actorBounds)
        {
            const auto worldBound = std::ranges::find(a_actorBounds, a_actor, &ActorBound::actor);
            const auto playerBound = std::ranges::find(a_actorBounds, RE::PlayerCharacter::GetSingleton(), &ActorBound::actor);
            if (worldBound == a_actorBounds.end() || playerBound == a_actorBounds.end())
                return std::nullopt;

            const auto actorPosition = a_actor->GetPosition();
            const auto localBound = Translate(worldBound->bound, { -actorPosition.x, -actorPosition.y, -actorPosition.z });
            const auto tileWidth = std::max({ localBound.max.x - localBound.min.x, playerBound->bound.max.x - playerBound->bound.min.x, MINIMUM_TILE_SIZE });
            const auto tileDepth = std::max({ localBound.max.y - localBound.min.y, playerBound->bound.max.y - playerBound->bound.min.y, MINIMUM_TILE_SIZE });
            const auto playerCenterX = (playerBound->bound.min.x + playerBound->bound.max.x) * 0.5f;
            const auto playerCenterY = (playerBound->bound.min.y + playerBound->bound.max.y) * 0.5f;
            const auto localCenterX = (localBound.min.x + localBound.max.x) * 0.5f;
            const auto localCenterY = (localBound.min.y + localBound.max.y) * 0.5f;
            const RE::NiPoint3 gridOrigin{ playerCenterX - localCenterX, playerCenterY - localCenterY, a_center.z };

            struct Tile
            {
                int x;
                int y;
                bool operator==(const Tile&) const = default;
            };
            constexpr std::array<Tile, 4> directions{ Tile{ 0, 1 }, Tile{ 1, 0 }, Tile{ 0, -1 }, Tile{ -1, 0 } };
            std::vector<Tile> queue{ { 0, 0 } };
            std::vector<Tile> visited{ { 0, 0 } };
            std::size_t queueIndex = 0;
            std::size_t candidateCount = 0;

            while (queueIndex < queue.size() && candidateCount < MAX_SEARCH_CANDIDATES) {
                const auto current = queue[queueIndex++];
                for (const auto& direction : directions) {
                    const Tile next{ current.x + direction.x, current.y + direction.y };
                    if (std::ranges::contains(visited, next))
                        continue;
                    visited.push_back(next);
                    if (candidateCount == MAX_SEARCH_CANDIDATES)
                        return std::nullopt;
                    candidateCount++;

                    const RE::NiPoint3 target{
                        gridOrigin.x + static_cast<float>(next.x) * tileWidth,
                        gridOrigin.y + static_cast<float>(next.y) * tileDepth,
                        gridOrigin.z
                    };
                    if (std::hypot(static_cast<float>(next.x) * tileWidth, static_cast<float>(next.y) * tileDepth) > MAX_SEARCH_DISTANCE)
                        continue;

                    const auto projected = ProjectToNavmesh(a_actor, target, std::min(tileWidth, tileDepth) * 0.5f);
                    if (!projected || HasBlockingWorldCollision(a_actor, *projected, a_actorBounds))
                        continue;

                    queue.push_back(next);
                    const auto candidateBound = Translate(localBound, *projected);
                    if (!IntersectsActor(candidateBound, a_actor, a_actorBounds))
                        return projected;
                }
            }
            return std::nullopt;
        }

        // Most failure cases will be due to starting a scene at a position with no navmesh. This uses a floodfill algorithm, so missing navmesh wont be able to solve
        // No navmesh also means the NPC can't walk away, so it might be ideal to implement a failsafe (Like teleport to nearest navmesh, or previous pos)
        inline void MoveActorsAwayFromPlayer(std::span<RE::Actor* const> a_actors, bool a_movePlayer)
        {
            const auto player = RE::PlayerCharacter::GetSingleton();
            if (!player)
                return;

            auto actorBounds = GatherActorBounds(player->GetPosition(), a_actors);
            const auto playerBound = std::ranges::find(actorBounds, player, &ActorBound::actor);
            if (playerBound == actorBounds.end()) {
                logger::warn("Cannot move actors away from the player because the player's character controller shape is unavailable.");
                return;
            }

            if (a_movePlayer) {
                const auto intersectsSceneActor = std::ranges::any_of(a_actors, [&](RE::Actor* a_actor) {
                    if (!a_actor || a_actor == player)
                        return false;
                    const auto bound = std::ranges::find(actorBounds, a_actor, &ActorBound::actor);
                    return bound != actorBounds.end() && Intersects(playerBound->bound, bound->bound);
                });
                if (!intersectsSceneActor)
                    return;

                if (const auto target = FindSafePosition(player, player->GetPosition(), actorBounds)) {
                    player->SetPosition(*target, true);
                } else {
                    logger::warn("No safe position was found for the player within {:.0f} units.", MAX_SEARCH_DISTANCE);
                }
                return;
            }

            for (const auto actor : a_actors) {
                if (!actor || actor == player)
                    continue;

                const auto bound = std::ranges::find(actorBounds, actor, &ActorBound::actor);
                if (bound == actorBounds.end() || !Intersects(playerBound->bound, bound->bound))
                    continue;

                if (const auto target = FindSafePosition(actor, player->GetPosition(), actorBounds)) {
                    const auto position = actor->GetPosition();
                    bound->bound = Translate(bound->bound, { target->x - position.x, target->y - position.y, target->z - position.z });
                    actor->SetPosition(*target, true);
                } else {
                    logger::warn("No safe position was found for actor {:08X} within {:.0f} units.", actor->GetFormID(), MAX_SEARCH_DISTANCE);
                }
            }
        }
    }
}
