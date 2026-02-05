#include "NiUpdate.h"

namespace Thread::NiNode
{
	void NiUpdate::Install()
	{
		const auto address = REL::RelocationID(35565, 36564).address();
		const auto offset = REL::VariantOffset(0x53, 0x6E, 0x68).offset(); 
		REL::Relocation<std::uintptr_t> update{ address, offset };
		_OnFrameUpdate = trampoline.write_call<5>(update.address(), OnFrameUpdate);
	}
	
	float NiUpdate::GetDeltaTime()
	{
		static REL::Relocation<float*> deltaTime{ REL::VariantID(523660, 410199, 0x30C3A08) };
		return *deltaTime.get();
	}

	void NiUpdate::OnFrameUpdate(RE::PlayerCharacter* a_this)
	{
		_OnFrameUpdate(a_this);

		std::scoped_lock lk{ _m };
		time += GetDeltaTime();
		for (auto&& [_, process] : _instances) {
			process->Update(time);
		}
	}

	std::shared_ptr<NiInstance> NiUpdate::Register(RE::FormID a_id, std::vector<RE::Actor*> a_positions, const Registry::Scene* a_scene) noexcept
	{
		try {
			std::scoped_lock lk{ _m };
			const auto where = std::ranges::find(_instances, a_id, [](auto& it) { return it.first; });
			if (where != _instances.end()) {
				logger::info("Object with ID {:X} already registered. Resetting NiInstance.", a_id);
				std::swap(*where, _instances.back());
				_instances.pop_back();
			}
			auto process = std::make_shared<NiInstance>(a_positions, a_scene);
			return _instances.emplace_back(a_id, process).second;
		} catch (const std::exception& e) {
			logger::error("Failed to register NiInstance: {}", e.what());
			return nullptr;
		} catch (...) {
			logger::error("Failed to register NiInstance: Unknown error");
			return nullptr;
		}
	}

	void NiUpdate::Unregister(RE::FormID a_id) noexcept
	{
		std::scoped_lock lk{ _m };
		const auto where = std::ranges::find(_instances, a_id, [](auto& it) { return it.first; });
		if (where == _instances.end()) {
			logger::error("No object registered using ID {:X}", a_id);
			return;
		}
		_instances.erase(where);
	}

}  // namespace Thread::NiNode

