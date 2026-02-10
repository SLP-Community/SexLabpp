#pragma once

#include "NiMath.h"

namespace Thread::NiNode::Node
{
	static constexpr std::string_view HEAD{ "NPC Head [Head]"sv };		   // Back of throat
	static constexpr std::string_view PELVIS{ "NPC Pelvis [Pelv]"sv };	   // bottom mid (front)
	static constexpr std::string_view SPINELOWER{ "NPC Spine [Spn0]"sv };  // bottom mid (back)

	static constexpr std::string_view HANDLEFT{ "NPC L Hand [LHnd]"sv };  // Base hand
	static constexpr std::string_view HANDRIGHT{ "NPC R Hand [RHnd]"sv };
	static constexpr std::string_view HANDLEFTREF{ "SHIELD"sv };
	static constexpr std::string_view HANDRIGHTREF{ "WEAPON"sv };
	static constexpr std::string_view FINGERLEFT{ "NPC L Finger20 [LF20]"sv };	// Base of the middle finger
	static constexpr std::string_view FINGERRIGHT{ "NPC R Finger20 [RF20]"sv };
	static constexpr std::string_view THUMBLEFT{ "NPC L Finger02 [LF02]"sv };  // Thumb
	static constexpr std::string_view THUMBRIGHT{ "NPC R Finger02 [RF02]"sv };

	static constexpr std::string_view FOOTLEFT{ "NPC L Foot [Lft ]"sv };  // Ankle
	static constexpr std::string_view FOOTRIGHT{ "NPC R Foot [Rft ]"sv };
	static constexpr std::string_view TOELEFT{ "NPC L Toe0 [LToe]"sv };	 // base of middle toe
	static constexpr std::string_view TOERIGHT{ "NPC R Toe0 [RToe]"sv };

	static constexpr std::string_view CLITORIS{ "Clitoral1"sv };
	static constexpr std::string_view VAGINADEEP{ "VaginaDeep1"sv };
	static constexpr std::string_view VAGINAB{ "VaginaB1"sv };
	static constexpr std::string_view VAGINALLEFT{ "NPC L Pussy02"sv };
	static constexpr std::string_view VAGINALRIGHT{ "NPC R Pussy02"sv };
	static constexpr std::string_view ANALDEEP{ "NPC Anus Deep1"sv };
	static constexpr std::string_view ANALLEFT{ "NPC LT Anus2"sv };
	static constexpr std::string_view ANALRIGHT{ "NPC RT Anus2"sv };

	static constexpr std::string_view ANIMOBJECTA{ "AnimObjectA"sv };
	static constexpr std::string_view ANIMOBJECTB{ "AnimObjectB"sv };
	static constexpr std::string_view ANIMOBJECTR{ "AnimObjectR"sv };
	static constexpr std::string_view ANIMOBJECTL{ "AnimObjectL"sv };
	struct SchlongInfo
	{
		constexpr SchlongInfo(std::string_view a_base, std::string_view a_mid, std::string_view a_tip) :
		  base(a_base), mid(a_mid), tip(a_tip), rot(glm::mat3(1.0f)) {}
		constexpr SchlongInfo(std::string_view a_base, glm::mat3 a_rotation) :
		  base(a_base), mid(""), tip(""), rot(a_rotation) {}

		std::string_view base, mid, tip;
		glm::mat3 rot;
	};
	static constexpr std::array SCHLONG_NODES{
		SchlongInfo("NPC Genitals01 [Gen01]", "NPC Genitals04 [Gen04]", "NPC Genitals06 [Gen06]"),
		SchlongInfo("AH Base", "AH 3", "AH 6"),
		SchlongInfo("DD 2", "DD 3", "DD 6"),
		SchlongInfo("NPC IceGenital02", "NPC IceGenital03", ""),
		SchlongInfo("BearD 3", "BearD 6", "BearD 8"),
		SchlongInfo("GS 3", "GS 4", "GS 7"),
		SchlongInfo("BoarDick01", "BoarDick03", "BoarDick06"),
		SchlongInfo("RD 2", "RD 3", "RD 5"),
		SchlongInfo("CDPenis 2", "CDPenis 5", "CDPenis 7"),
		SchlongInfo("CO 2", "CO 5", "CO 9"),
		SchlongInfo("ElkD03", "ElkD04", "ElkD06"),
		SchlongInfo("DwarvenSpiderDildo01", "DwarvenSpiderDildo02", "DwarvenSpiderDildo03"),
		SchlongInfo("FD 3", "FD 4", "FD 7"),
		SchlongInfo("GD 3", "GD 4", "GD 7"),
		SchlongInfo("Goat_Penis02", "Goat_Penis04", "Goat_Penis06"),
		SchlongInfo("Horker_Penis04", "Horker_Penis06", "Horker_Penis10"),
		SchlongInfo("HS 3", "HS 6", "HS 7"),
		SchlongInfo("SCD 3", "SCD 4", "SCD 7"),
		SchlongInfo("SkeeverD 03", "SkeeverD 05", "SkeeverD 07"),
		SchlongInfo("TD 3", "TD 5", "TD 7"),
		SchlongInfo("VLDick03", "VLDick05", "VLDick06"),
		// Default Euler = (-158.18, -1.51, -54.54), facing Y at approx (0, 0, 90)
		SchlongInfo("NPC Torso Rock 01", glm::mat3{ 0.76184751, 0.28855865, 0.579933, 0.37156284, -0.92803376, -0.02635142, 0.53059347, 0.23555732, -0.81423788 }),
		// Default Euler = (-176.49, 22.60, -131.08), facing Y at approx (0, 0, 90)
		SchlongInfo("NPC Torso Rock 02", glm::mat3{ 0.76783908, -0.20590216, -0.60665266, 0.05652146, -0.92147839, 0.38429532, -0.63814456, -0.32936586, -0.69590923 }),
		// Default Euler = (-7.68, 0, 0), facing Y at approx (72.32, 0, 0)
		SchlongInfo("Torso Rock 2", glm::mat3{ 0.17364818, -0.98480775, 0, 0.90363453, 0.42830438, 0, 0, 0, 1 }),
		// Default Euler = (-7.68, 0, 0), facing Y at approx (43.32, 0, 0)
		SchlongInfo("Torso Rock 1", glm::mat3{ 0.62932039, -0.77714596, 0, 0.77714596, 0.62932039, 0, 0, 0, 1 }),
	};
	static constexpr std::array SCHLONG_ANGLES{
		25.0f, 32.0f, 39.0f, 46.0f, 53.0f, 60.0f, 67.0f, 74.0f, 81.0f, 88.0f, 95.0f, 102.0f, 109.0f, 116.0f, 123.0f, 130.0f, 137.0f, 144.0f, 151.0f
	};
	static constexpr float MIN_SCHLONG_LEN{ 13.0f };

	struct NodeData
	{
		struct Schlong
		{
			virtual NiMath::Segment GetReferenceSegment() const = 0;
			virtual RE::NiPointer<RE::NiNode> GetBaseReferenceNode() const = 0;
		};

		struct FakeSchlong : public Schlong
		{
			FakeSchlong(const NodeData& a_ownerNodes) :
			  ownerNodes(a_ownerNodes) {}
			~FakeSchlong() = default;

			virtual NiMath::Segment GetReferenceSegment() const override;
			virtual RE::NiPointer<RE::NiNode> GetBaseReferenceNode() const override;

		  private:
			const NodeData& ownerNodes;

			RE::NiPoint3 ApproximateTip() const;
			RE::NiPoint3 ApproximateMid() const;
			RE::NiPoint3 ApproximateBase() const;
			RE::NiPoint3 ApproximateNode(float a_forward, float a_upward) const;
		};

		struct SchlongData : public Schlong
		{
			SchlongData(RE::NiPointer<RE::NiNode> a_basenode, const glm::mat3& a_rot);
			~SchlongData() = default;

			virtual NiMath::Segment GetReferenceSegment() const override;
			virtual RE::NiPointer<RE::NiNode> GetBaseReferenceNode() const override;

		  private:
			std::vector<RE::NiPointer<RE::NiNode>> nodes{};
			RE::NiMatrix3 rot;

		  public:
			bool operator==(const SchlongData& a_rhs) const { return this->nodes.size() == a_rhs.nodes.size() && this->nodes.front() == a_rhs.nodes.front(); }
		};

	  public:
		NodeData(RE::Actor* a_actor, bool a_forceSchlong);
		~NodeData() = default;

		RE::NiPointer<RE::NiNode> head;
		RE::NiPointer<RE::NiNode> pelvis;
		RE::NiPointer<RE::NiNode> spine_lower;

		RE::NiPointer<RE::NiNode> hand_left;
		RE::NiPointer<RE::NiNode> hand_right;
		RE::NiPointer<RE::NiNode> thumb_left;
		RE::NiPointer<RE::NiNode> thumb_right;
		RE::NiPointer<RE::NiNode> foot_left;
		RE::NiPointer<RE::NiNode> foot_right;
		RE::NiPointer<RE::NiNode> toe_left;
		RE::NiPointer<RE::NiNode> toe_right;

		RE::NiPointer<RE::NiNode> clitoris;
		RE::NiPointer<RE::NiNode> vaginadeep;
		RE::NiPointer<RE::NiNode> vaginab;
		RE::NiPointer<RE::NiNode> vaginaleft;
		RE::NiPointer<RE::NiNode> vaginaright;
		RE::NiPointer<RE::NiNode> analdeep;
		RE::NiPointer<RE::NiNode> analleft;
		RE::NiPointer<RE::NiNode> analright;
		std::vector<std::shared_ptr<Schlong>> schlongs;

		RE::NiPointer<RE::NiNode> animobj_a;
		RE::NiPointer<RE::NiNode> animobj_b;
		RE::NiPointer<RE::NiNode> animobj_l;
		RE::NiPointer<RE::NiNode> animobj_r;

	  public:
		std::optional<NiMath::Segment> GetVaginalSegment() const;
		std::optional<NiMath::Segment> GetAnalSegment() const;
		std::optional<RE::NiPoint3> GetToeVectorLeft() const;
		std::optional<RE::NiPoint3> GetToeVectorRight() const;
		NiMath::Segment GetCrotchSegment() const;
	};

}
