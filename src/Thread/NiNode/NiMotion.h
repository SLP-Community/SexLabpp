#pragma once

#include "NiMath.h"
#include "Node.h"
#include "Registry/Util/RayCast/ObjectBound.h"

namespace Thread::NiNode
{
	constexpr size_t WINDOW_SIZE = 60;
	constexpr size_t MIN_MOMENTS = 6;

	struct MotionDescriptor
	{
		MotionDescriptor(const NiMath::Segment& a_traj) :
		  trajectory(a_traj) {}
		~MotionDescriptor() = default;

		/// @brief Get center of motion
		RE::NiPoint3 Mean() const { return (trajectory.first + trajectory.second) / 2.0f; }

		/// @brief Get length of motion trajectory
		float Length() const { return trajectory.Length(); }

		/// @brief Check if motion describes a valid movement
		bool DescribesMotion() const { return !trajectory.IsPoint(); }

	  public:
		NiMath::Segment trajectory;			// dominant motion segment
		float totalDistance{ 0.0f };		// accumulated path length
		float duration{ 0.0f };				// total time covered
		float avgSpeed{ 0.0f };				// totalDistance / duration
		float peakSpeed{ 0.0f };			// max instantaneous speed
		float positionalVariance{ 0.0f };	// scatter around trajectory
		float directionalVariance{ 0.0f };	// how stable direction is
		float oscillation{ 0.0f };			// 0 = monotonic, 1 = strong oscillation
		float impulse{ 0.0f };				// high for punch-like motion
	};

	class NiMotion
	{
	  public:
		enum Anchor
		{
			pHead,
			vHeadX,
			vHeadY,
			vHeadZ,
			pMouth,
			pThroat,

			pSchlongBase,
			pSchlongTip,
			pVaginalStart,
			pVaginalEnd,
			pClitoris,
			pAnalStart,
			pAnalEnd,

			pSpineLower,
			pPelvis,
		};
		constexpr static inline size_t NUM_ANCHORS = magic_enum::enum_count<Anchor>();

	  public:
		NiMotion(size_t capacity = WINDOW_SIZE, size_t minMoments = MIN_MOMENTS);
		~NiMotion() = default;

		void Push(const Node::NodeData& nodes, float timeStamp);
		bool HasMomentData(Anchor c) const { return _size > 0 && GetLatestMoment(c) != RE::NiPoint3::Zero(); }
		bool HasSufficientData() const { return _size >= _minMoments; }

		size_t Size() const { return _size; }
		size_t Length() const { return _size; }
		size_t Capacity() const { return _capacity; }

		/// @brief Get the nth most recent moment for the given anchor (0 = oldest, Size()-1 = latest)
		const RE::NiPoint3& GetNthMoment(Anchor c, size_t n) const;
		const ObjectBound& GetNthHeadBound(size_t n) const;
		float GetNthTimestamp(size_t n) const;
		
		/// @brief Get the most recent moment for the given anchor
		const RE::NiPoint3& GetLatestMoment(Anchor c) const { return GetNthMoment(c, _size - 1); }
		const ObjectBound& GetLatestHeadBound() const { return GetNthHeadBound(_size - 1); }
		float GetLatestTimestamp() const { return GetNthTimestamp(_size - 1); }

		/// @brief Get the oldest moment for the given anchor
		const RE::NiPoint3& GetFirstMoment(Anchor c) const { return GetNthMoment(c, 0); }
		const ObjectBound& GetFirstHeadBound() const { return GetNthHeadBound(0); }
		float GetFirstTimestamp() const { return GetNthTimestamp(0); }

		/// @brief Iterate through all stored moments for the given anchor, in chronological order
		/// @param c Anchor to iterate
		/// @param func Function (anchorPoint, timestamp) to call for each moment. If it returns true, iteration stops.
		void ForEachMoment(Anchor c, const std::function<bool(const RE::NiPoint3&, float)>& func) const;

		/// @brief Get best-fit motion segment for the given anchor
		/// @param c Anchor to analyze
		NiMath::Segment GetMotion(Anchor c) const;

		/// @brief Describe the motion for the given anchor
		/// @param c Anchor to analyze
		MotionDescriptor DescribeMotion(Anchor c) const;

	  private:
		size_t AbsoluteToRelativeIndex(size_t n) const;

	  private:
		const size_t _capacity;
		const size_t _minMoments;
		size_t _writeIndex{ 0 };
		size_t _size{ 0 };

		std::array<std::vector<RE::NiPoint3>, NUM_ANCHORS> _moments;
		std::vector<ObjectBound> _headBounds;
		std::vector<float> _timestamps;
	};

}  // namespace Thread::NiNode
