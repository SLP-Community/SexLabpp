#include "ActorGeometry.h"

#include "Registry/Define/RaceKey.h"
#include "SurfaceCache.h"

namespace Thread::Interaction::NiSurface::Geometry
{
    using namespace Detail;

    namespace
    {
        constexpr std::string_view FACE_ROOT{ "BSFaceGenNiNodeSkinned"sv };
        constexpr std::string_view HEAD{ "NPC Head [Head]"sv };          // Back-of-throat reference.
        constexpr std::string_view PELVIS{ "NPC Pelvis [Pelv]"sv };      // Bottom-middle front reference.
        constexpr std::string_view LOWER_SPINE{ "NPC Spine [Spn0]"sv };  // Bottom-middle back reference.
        constexpr std::string_view LEFT_HAND{ "SHIELD"sv };
        constexpr std::string_view RIGHT_HAND{ "WEAPON"sv };
        constexpr std::string_view LEFT_THUMB{ "NPC L Finger02 [LF02]"sv };  // Thumb.
        constexpr std::string_view RIGHT_THUMB{ "NPC R Finger02 [RF02]"sv };
        constexpr std::string_view LEFT_FOOT{ "NPC L Foot [Lft ]"sv };  // Ankle.
        constexpr std::string_view RIGHT_FOOT{ "NPC R Foot [Rft ]"sv };
        constexpr std::string_view LEFT_TOE{ "NPC L Toe0 [LToe]"sv };  // Base of the middle toe.
        constexpr std::string_view RIGHT_TOE{ "NPC R Toe0 [RToe]"sv };
        constexpr std::string_view VAGINA_DEEP{ "VaginaDeep1"sv };
        constexpr std::string_view VAGINA_LEFT{ "NPC L Pussy02"sv };
        constexpr std::string_view VAGINA_RIGHT{ "NPC R Pussy02"sv };
        constexpr std::string_view ANUS_DEEP{ "NPC Anus Deep1"sv };
        constexpr std::string_view ANUS_LEFT{ "NPC LT Anus2"sv };
        constexpr std::string_view ANUS_RIGHT{ "NPC RT Anus2"sv };
        constexpr std::string_view ANIM_OBJECT_A{ "AnimObjectA"sv };
        constexpr std::string_view ANIM_OBJECT_B{ "AnimObjectB"sv };
        constexpr std::string_view ANIM_OBJECT_LEFT{ "AnimObjectL"sv };
        constexpr std::string_view ANIM_OBJECT_RIGHT{ "AnimObjectR"sv };

        RE::NiPointer<RE::NiAVObject> FindMouth(RE::NiAVObject* a_root)
        {
            auto* faceObject = a_root ? a_root->GetObjectByName(FACE_ROOT) : nullptr;
            auto* faceNode = faceObject ? faceObject->AsNode() : nullptr;
            if (!faceNode) {
                return {};
            }
            const auto& children = faceNode->GetChildren();
            const auto where = std::ranges::find_if(children, [](const auto& a_child) {
                return a_child && std::string_view(a_child->name.c_str()).contains("Mouth");
            });
            if (where == children.end()) {
                return {};
            }
            return *where;
        }

        struct ShaftBase
        {
            std::string_view name;
            glm::mat3 rotation{ 1.0f };
        };

        // Only the base is configured; Shaft discovers the weighted descendant branch.
        constexpr std::array SHAFT_BASES{
            ShaftBase{ "NPC Genitals01 [Gen01]" },
            ShaftBase{ "AH Base" },
            ShaftBase{ "DD 2" },
            ShaftBase{ "NPC IceGenital02" },
            ShaftBase{ "BearD 3" },
            ShaftBase{ "GS 3" },
            ShaftBase{ "BoarDick01" },
            ShaftBase{ "RD 2" },
            ShaftBase{ "CDPenis 2" },
            ShaftBase{ "CO 2" },
            ShaftBase{ "ElkD03" },
            ShaftBase{ "DwarvenSpiderDildo01" },
            ShaftBase{ "FD 3" },
            ShaftBase{ "GD 3" },
            ShaftBase{ "Goat_Penis02" },
            ShaftBase{ "Horker_Penis04" },
            ShaftBase{ "HS 3" },
            ShaftBase{ "SCD 3" },
            ShaftBase{ "SkeeverD 03" },
            ShaftBase{ "TD 3" },
            ShaftBase{ "VLDick03" },
            // Default Euler = (-158.18, -1.51, -54.54), facing Y at approximately (0, 0, 90).
            ShaftBase{ "NPC Torso Rock 01", { 0.76184751, 0.28855865, 0.579933, 0.37156284, -0.92803376, -0.02635142, 0.53059347, 0.23555732, -0.81423788 } },
            // Default Euler = (-176.49, 22.60, -131.08), facing Y at approximately (0, 0, 90).
            ShaftBase{ "NPC Torso Rock 02", { 0.76783908, -0.20590216, -0.60665266, 0.05652146, -0.92147839, 0.38429532, -0.63814456, -0.32936586, -0.69590923 } },
            // Default Euler = (-7.68, 0, 0), facing Y at approximately (72.32, 0, 0).
            ShaftBase{ "Torso Rock 2", { 0.17364818, -0.98480775, 0, 0.90363453, 0.42830438, 0, 0, 0, 1 } },
            // Default Euler = (-7.68, 0, 0), facing Y at approximately (43.32, 0, 0).
            ShaftBase{ "Torso Rock 1", { 0.62932039, -0.77714596, 0, 0.77714596, 0.62932039, 0, 0, 0, 1 } },
        };
    }

    ActorGeometry::ActorGeometry(RE::Actor* a_actor) :
      ownerActor(a_actor)
    {
        const auto obj = a_actor->Get3D();
        if (!obj) {
            const auto msg = std::format("Unable to retrieve 3D of actor {:X}", a_actor->GetFormID());
            throw std::exception(msg.c_str());
        }
        const auto racekey = Registry::RaceKey(a_actor);
        const auto racestr = racekey.IsValid() ? racekey.AsString() : "?";
        const auto getNode = [&](std::string_view a_name, auto& a_target, bool a_log) {
            auto* object = obj->GetObjectByName(a_name);
            auto* node = object ? object->AsNode() : nullptr;
            if (!node) {
                if (a_log) {
                    logger::info("NiSurface Interaction: Actor {:X} (Race: {}) is missing node {} (this may be expected)", a_actor->GetFormID(), racestr, a_name);
                }
                return false;
            }
            a_target = RE::NiPointer{ node };
            return true;
        };
        // This is likely a mistake: requiring these human nodes will always reject non-human actors.
        if (!getNode(PELVIS, pelvis, true) || !getNode(LOWER_SPINE, lowerSpine, true)) {
            throw std::exception("Missing mandatory 3d object (body)");
        }
        getNode(HEAD, head, true);
        getNode(LEFT_HAND, leftHand, true);
        getNode(RIGHT_HAND, rightHand, false);
        getNode(LEFT_THUMB, leftThumb, false);
        getNode(RIGHT_THUMB, rightThumb, false);
        getNode(LEFT_FOOT, leftFoot, true);
        getNode(RIGHT_FOOT, rightFoot, false);
        getNode(LEFT_TOE, leftToe, true);
        getNode(RIGHT_TOE, rightToe, true);
        getNode(VAGINA_DEEP, deepVagina, true);
        getNode(VAGINA_LEFT, leftVagina, false);
        getNode(VAGINA_RIGHT, rightVagina, false);
        getNode(ANUS_DEEP, deepAnus, true);
        getNode(ANUS_LEFT, leftAnus, false);
        getNode(ANUS_RIGHT, rightAnus, false);
        getNode(ANIM_OBJECT_A, animObjectA, false);
        getNode(ANIM_OBJECT_B, animObjectB, false);
        getNode(ANIM_OBJECT_LEFT, animObjectLeft, false);
        getNode(ANIM_OBJECT_RIGHT, animObjectRight, false);

        // FaceGen mouth meshes are denture-like; retain the named child and read its updated world bound each frame.
        mouth = FindMouth(obj);

        for (const auto& base : SHAFT_BASES) {
            auto* object = obj->GetObjectByName(base.name);
            auto* node = object ? object->AsNode() : nullptr;
            if (!node) {
                continue;
            }
            shafts.emplace_back(a_actor, RE::NiPointer{ node }, base.rotation);
        }
        logger::info("NiSurface Interaction: Shaft discovery actor {:X}: configured bases found={}", a_actor->GetFormID(), shafts.size());
        const std::array<RE::NiAVObject*, 2> vaginalTargets{ leftVagina.get(), rightVagina.get() };
        const std::array<RE::NiAVObject*, 2> analTargets{ leftAnus.get(), rightAnus.get() };
        const bool wantsVaginal = deepVagina && std::ranges::all_of(vaginalTargets, [](auto* a_target) { return a_target != nullptr; });
        const bool wantsAnal = deepAnus && std::ranges::all_of(analTargets, [](auto* a_target) { return a_target != nullptr; });

        const auto& biped = a_actor->GetBiped();
        if (biped && (wantsVaginal || wantsAnal)) {
            for (const auto& object : biped->objects) {
                if (!object.part || !object.partClone) {
                    continue;
                }
                const auto* path = object.part->GetModel();
                const auto modelPath = path ? std::string_view(path) : std::string_view{};
                const auto selection = GetOpeningSelection(modelPath);
                if (!selection || ((!wantsVaginal || trackedVagina || selection->vaginal.empty()) && (!wantsAnal || trackedAnus || selection->anal.empty()))) {
                    continue;
                }

                RE::BSVisit::TraverseScenegraphGeometries(object.partClone.get(), [&](RE::BSGeometry* a_geometry) {
                    const auto geometryName = std::string_view(a_geometry->name.c_str());
                    if (wantsVaginal && !trackedVagina && geometryName == selection->vaginal) {
                        TrackedOpening candidate;
                        if (candidate.Bind(a_geometry, modelPath, "vaginal", vaginalTargets, deepVagina.get())) {
                            trackedVagina = std::move(candidate);
                        }
                    }
                    if (wantsAnal && !trackedAnus && geometryName == selection->anal) {
                        TrackedOpening candidate;
                        if (candidate.Bind(a_geometry, modelPath, "anal", analTargets, deepAnus.get())) {
                            trackedAnus = std::move(candidate);
                        }
                    }
                    return (!wantsVaginal || trackedVagina) && (!wantsAnal || trackedAnus) ? RE::BSVisit::BSVisitControl::kStop : RE::BSVisit::BSVisitControl::kContinue;
                });
                if ((!wantsVaginal || trackedVagina) && (!wantsAnal || trackedAnus)) {
                    break;
                }
            }
        }

        RE::NiPointer<RE::BSGeometry> vaginalGeometry;
        RE::NiPointer<RE::BSGeometry> analGeometry;
        OpeningWeights vaginalWeights{};
        OpeningWeights analWeights{};
        if ((wantsVaginal && !trackedVagina) || (wantsAnal && !trackedAnus)) {
            RE::BSVisit::TraverseScenegraphGeometries(obj, [&](RE::BSGeometry* a_geometry) {
                const auto consider = [&](const auto& a_targets, RE::NiPointer<RE::BSGeometry>& a_bestGeometry, OpeningWeights& a_bestWeights) {
                    const auto weights = GetOpeningWeights(a_geometry, a_targets);
                    if (!weights) {
                        return;
                    }
                    const auto score = std::min(weights->maximum[0], weights->maximum[1]);
                    const auto bestScore = std::min(a_bestWeights.maximum[0], a_bestWeights.maximum[1]);
                    const auto sum = weights->maximum[0] + weights->maximum[1];
                    const auto bestSum = a_bestWeights.maximum[0] + a_bestWeights.maximum[1];
                    bool preferModelPath = false;
                    if (a_bestGeometry && score == bestScore && sum == bestSum) {
                        preferModelPath = !GetModelPath(a_actor, a_geometry).empty() && GetModelPath(a_actor, a_bestGeometry.get()).empty();
                    }
                    if (!a_bestGeometry || score > bestScore || (score == bestScore && sum > bestSum) || preferModelPath) {
                        a_bestGeometry.reset(a_geometry);
                        a_bestWeights = *weights;
                    }
                };
                if (wantsVaginal && !trackedVagina) {
                    consider(vaginalTargets, vaginalGeometry, vaginalWeights);
                }
                if (wantsAnal && !trackedAnus) {
                    consider(analTargets, analGeometry, analWeights);
                }
                return RE::BSVisit::BSVisitControl::kContinue;
            });
        }
        if (vaginalGeometry) {
            const auto modelPath = GetModelPath(a_actor, vaginalGeometry.get());
            logger::info("NiSurface Interaction: Vaginal surface selected '{}' in '{}': max weights left={:.4f} ({} vertices), right={:.4f} ({} vertices)", vaginalGeometry->name,
                modelPath, vaginalWeights.maximum[0], vaginalWeights.counts[0], vaginalWeights.maximum[1], vaginalWeights.counts[1]);
            TrackedOpening candidate;
            if (candidate.Bind(vaginalGeometry.get(), modelPath, "vaginal", vaginalTargets, deepVagina.get())) {
                trackedVagina = std::move(candidate);
                CacheOpeningSelection(modelPath, "vaginal", std::string_view(vaginalGeometry->name.c_str()));
            }
        }
        if (analGeometry) {
            const auto modelPath = GetModelPath(a_actor, analGeometry.get());
            logger::info("NiSurface Interaction: Anal surface selected '{}' in '{}': max weights left={:.4f} ({} vertices), right={:.4f} ({} vertices)", analGeometry->name,
                modelPath, analWeights.maximum[0], analWeights.counts[0], analWeights.maximum[1], analWeights.counts[1]);
            TrackedOpening candidate;
            if (candidate.Bind(analGeometry.get(), modelPath, "anal", analTargets, deepAnus.get())) {
                trackedAnus = std::move(candidate);
                CacheOpeningSelection(modelPath, "anal", std::string_view(analGeometry->name.c_str()));
            }
        }
        const auto mouthOpening = GetMouthOpening();
        if (mouth) {
            const auto& center = mouth->worldBound.center;
            logger::info("NiSurface Interaction: Actor {:X} mouth surface selected '{}' under '{}': center=({:.2f}, {:.2f}, {:.2f}), radius={:.2f}, opening={}", a_actor->GetFormID(), mouth->name,
                mouth->parent ? mouth->parent->name.c_str() : "", center.x, center.y, center.z, mouth->worldBound.radius, mouthOpening.has_value());
        }
        logger::info("NiSurface Interaction: Actor {:X} surface shapes: mouth={}, vaginal={}, anal={}", a_actor->GetFormID(), mouthOpening.has_value(), trackedVagina.has_value(), trackedAnus.has_value());
    }

    std::optional<GeometryMath::Segment> ActorGeometry::GetVaginalSegment() const
    {
        if (!deepVagina || !leftVagina || !rightVagina)
            return std::nullopt;

        const auto start = (leftVagina->world.translate + rightVagina->world.translate) / 2;
        const auto end = deepVagina->world.translate;
        return GeometryMath::Segment{ start, end };
    }

    std::optional<GeometryMath::Segment> ActorGeometry::GetAnalSegment() const
    {
        if (!deepAnus || !leftAnus || !rightAnus)
            return std::nullopt;

        const auto start = (leftAnus->world.translate + rightAnus->world.translate) / 2;
        const auto end = deepAnus->world.translate;
        return GeometryMath::Segment{ start, end };
    }

    std::optional<OpeningShape> ActorGeometry::GetMouthOpening()
    {
        auto* root = ownerActor ? ownerActor->Get3D() : nullptr;
        const bool attachedToCurrent3D = [&]() {
            for (auto* object = mouth.get(); object; object = object->parent) {
                if (object == root) {
                    return true;
                }
            }
            return false;
        }();
        if (!attachedToCurrent3D) {
            auto replacement = FindMouth(root);
            if (replacement.get() != mouth.get()) {
                logger::info("NiSurface Interaction: Actor {:X} mouth surface {}", ownerActor ? ownerActor->GetFormID() : 0, replacement ? "reacquired" : "lost");
            }
            mouth = std::move(replacement);
        }
        if (!mouth || !mouth->parent || !head || mouth->worldBound.radius <= FLT_EPSILON) {
            return std::nullopt;
        }

        auto axis = head->world.translate - mouth->worldBound.center;
        if (axis.SqrLength() <= FLT_EPSILON) {
            axis = -head->world.rotate.GetVectorY();
        }
        axis.Unitize();

        auto right = head->world.rotate.GetVectorX();
        right -= axis * right.Dot(axis);
        if (right.SqrLength() <= FLT_EPSILON) {
            return std::nullopt;
        }
        right.Unitize();

        auto up = right.Cross(axis);
        if (up.SqrLength() <= FLT_EPSILON) {
            return std::nullopt;
        }
        up.Unitize();
        return OpeningShape{ mouth->worldBound.center, head->world.translate, axis, right, up, mouth->worldBound.radius };
    }

    std::optional<OpeningShape> ActorGeometry::GetVaginalOpening()
    {
        if (trackedVagina) {
            if (auto opening = trackedVagina->Update()) {
                return opening;
            }
        }
        const auto segment = GetVaginalSegment();
        return segment && leftVagina && rightVagina ? MakeNodeOpening(*segment, leftVagina->world.translate, rightVagina->world.translate) : std::nullopt;
    }

    std::optional<OpeningShape> ActorGeometry::GetAnalOpening()
    {
        if (trackedAnus) {
            if (auto opening = trackedAnus->Update()) {
                return opening;
            }
        }
        const auto segment = GetAnalSegment();
        return segment && leftAnus && rightAnus ? MakeNodeOpening(*segment, leftAnus->world.translate, rightAnus->world.translate) : std::nullopt;
    }

    void ActorGeometry::UpdateShafts()
    {
        for (auto& shaft : shafts) {
            shaft.UpdateCollisionShape();
        }
    }

    GeometryMath::Segment ActorGeometry::GetCrotchSegment() const
    {
        assert(pelvis && lowerSpine);
        return { lowerSpine->world.translate, pelvis->world.translate };
    }

}
