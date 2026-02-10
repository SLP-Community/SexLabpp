#include "NiMotion.h"

namespace Thread::NiNode
{
	NiMotion::NiMotion(size_t capacity, size_t minMoments) :
	  _capacity(capacity), _minMoments(minMoments)
	{
		for (auto& moment : _moments) {
			moment.resize(_capacity);
		}
		_headBounds.resize(_capacity);
		_timestamps.resize(_capacity);
	}

	void NiMotion::Push(const Node::NodeData& nodes, float timeStamp)
	{
		const size_t idx = _writeIndex;
		_timestamps[idx] = timeStamp;

		if (const auto niHead = nodes.head) {
			_moments[Anchor::vHeadX][idx] = niHead->world.rotate.GetVectorX();
			_moments[Anchor::vHeadY][idx] = niHead->world.rotate.GetVectorY();
			_moments[Anchor::vHeadZ][idx] = niHead->world.rotate.GetVectorZ();
			_moments[Anchor::pHead][idx] = nodes.head->world.translate;
			if (auto opt = ObjectBound::MakeBoundingBox(niHead.get())) {
				_headBounds[idx] = *opt;
				const auto down = _headBounds[idx].boundMin.z * 0.17f;
				const auto forward = _headBounds[idx].boundMax.y * 0.88f;
				_moments[Anchor::pThroat][idx] = (_moments[Anchor::vHeadZ][idx] * down) + _moments[Anchor::pHead][idx];
				_moments[Anchor::pMouth][idx] = (_moments[Anchor::vHeadY][idx] * forward) + _moments[Anchor::pThroat][idx];
			} else {
				_headBounds[idx] = ObjectBound{};
				logger::warn("Failed to get head bounding box");
			}
		}

		if (!nodes.schlongs.empty()) {
			const auto sSchlong = nodes.schlongs.front()->GetReferenceSegment();
			_moments[Anchor::pSchlongBase][idx] = sSchlong.first;
			_moments[Anchor::pSchlongTip][idx] = sSchlong.second;
		}

		if (const auto sVaginal = nodes.GetVaginalSegment()) {
			_moments[Anchor::pVaginalStart][idx] = sVaginal->first;
			_moments[Anchor::pVaginalEnd][idx] = sVaginal->second;
		}
		if (const auto& niClitoris = nodes.clitoris) {
			_moments[Anchor::pClitoris][idx] = niClitoris->world.translate;
		}

		if (const auto sAnal = nodes.GetAnalSegment()) {
			_moments[Anchor::pAnalStart][idx] = sAnal->first;
			_moments[Anchor::pAnalEnd][idx] = sAnal->second;
		}

		const auto sCrotch = nodes.GetCrotchSegment();
		_moments[Anchor::pSpineLower][idx] = sCrotch.first;
		_moments[Anchor::pPelvis][idx] = sCrotch.second;
		
		_writeIndex = (_writeIndex + 1) % _capacity;
		_size = std::min(_size + 1, _capacity);
	}

	void NiMotion::ForEachMoment(Anchor c, const std::function<bool(const RE::NiPoint3&, float)>& func) const
	{
		for (size_t i = 0; i < _size; i++) {
			if (func(GetNthMoment(c, i), GetNthTimestamp(i))) {
				break;
			}
		}
	}

	NiMath::Segment NiMotion::GetMotion(Anchor c) const
	{
		if (_size < 2) {
			return NiMath::Segment(_size == 1 ? GetNthMoment(c, 0) : RE::NiPoint3::Zero());
		}
		return NiMath::BestFit(_moments[c]);
	}

	MotionDescriptor NiMotion::DescribeMotion(Anchor c) const
	{
		MotionDescriptor out{ GetMotion(c) };

		if (!HasSufficientData()) {
			return out;
		}

		out.duration = GetNthTimestamp(_size - 1) - GetNthTimestamp(0);
		auto axis = out.trajectory.Vector();
		axis.Unitize();
		const auto mean = out.Mean();

		// Initialize accumulators
		float totalDist = 0.0f;
		float peakSpeed = 0.0f;
		float posVar = 0.0f;
		float impulse = 0.0f;
		float prevProj = 0.0f;
		int signChanges = 0;

		RE::NiPoint3 avgDir{};
		std::vector<RE::NiPoint3> dirs;
		dirs.reserve(_size - 1);

		// Cached values for pairwise/triple calculations
		const RE::NiPoint3 *p0 = nullptr, *p1 = nullptr;
		float t0, t1;

		ForEachMoment(c, [&](const RE::NiPoint3& p, float t) {
			// Positional variance (scatter around trajectory)
			const RE::NiPoint3 rel = p - mean;
			const float proj = rel.Dot(axis);
			const RE::NiPoint3 closest = mean + axis * proj;
			posVar += p.GetDistance(closest);

			// Oscillation (sign changes along trajectory axis)
			if (p1 != nullptr && (proj * prevProj < 0.0f))
				signChanges++;
			prevProj = proj;

			// Dependent calculations
			if (p1 != nullptr) {
				const float dt1 = t - t1;
				if (dt1 > 0.0f) {
					// Path length & speed
					const float d = p.GetDistance(*p1);
					totalDist += d;
					peakSpeed = std::max(peakSpeed, d / dt1);

					// Directional variance
					RE::NiPoint3 dir = p - *p1;
					if (dir.Length() > FLT_EPSILON) {
						dir.Unitize();
						avgDir += dir;
						dirs.push_back(dir);
					}

					// Impulse
					if (p0 != nullptr) {
						const float dt0 = t1 - t0;
						if (dt0 > 0.0f) {
							const RE::NiPoint3 v0 = (*p1 - *p0) / dt0;
							const RE::NiPoint3 v1 = (p - *p1) / dt1;
							impulse = std::max(impulse, (v1 - v0).Length());
						}
					}
				}
			}

			p0 = p1;
			p1 = &p;
			t0 = t1;
			t1 = t;

			return false;
		});

		// Finalize calculations
		out.totalDistance = totalDist;
		out.avgSpeed = totalDist / out.duration;
		out.peakSpeed = peakSpeed;
		out.positionalVariance = posVar / static_cast<float>(_size);
		out.oscillation = static_cast<float>(signChanges) / static_cast<float>(_size - 1);
		out.impulse = impulse;

		// Directional variance
		float dirVar = 0.0f;
		if (!dirs.empty()) {
			avgDir.Unitize();
			for (const auto& d : dirs) {
				dirVar += 1.0f - std::abs(d.Dot(avgDir));
			}
			dirVar /= static_cast<float>(dirs.size());
		}
		out.directionalVariance = dirVar;

		return out;
	}

	size_t NiMotion::AbsoluteToRelativeIndex(size_t n) const
	{
		assert(n < _size);
		// If buffer is not full, physical index equals logical index
		// If buffer is full, account for the ring buffer wraparound
		return (_size < _capacity) ? n : ((_writeIndex + n) % _capacity);
	}

	const RE::NiPoint3& NiMotion::GetNthMoment(Anchor c, size_t n) const
	{
		assert(c < NUM_ANCHORS);
		assert(n < _size);
		return _moments[c][AbsoluteToRelativeIndex(n)];
	}

	const ObjectBound& NiMotion::GetNthHeadBound(size_t n) const
	{
		assert(n < _size);
		return _headBounds[AbsoluteToRelativeIndex(n)];
	}

	float NiMotion::GetNthTimestamp(size_t n) const
	{
		assert(n < _size);
		return _timestamps[AbsoluteToRelativeIndex(n)];
	}

}  // namespace Thread::NiNode
