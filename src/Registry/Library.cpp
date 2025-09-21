#include "Library.h"

#include "Define/RaceKey.h"

namespace Registry
{
	std::vector<const Scene*> Library::LookupScenes(const std::vector<RE::Actor*>& a_actors, const std::vector<std::string_view>& a_tags, const std::vector<RE::Actor*>& a_submissives) const
	{
		const auto tStart = std::chrono::high_resolution_clock::now();
		ActorFragment::FragmentHash hash;
#ifndef SKYRIMVR
		std::thread _hashbuilder{ [&]() {
#endif
			std::vector<ActorFragment> fragments;
			for (auto&& position : a_actors) {
				if (!position) {
					logger::warn("Warning: NULL Actor passed to LookupScenes");
					continue;
				}
				const auto submissive = std::ranges::contains(a_submissives, position);
				fragments.emplace_back(position, submissive);
			}
			hash = ActorFragment::MakeFragmentHash(fragments);
#ifndef SKYRIMVR
		} };
#endif
		TagDetails tags{ a_tags };
		const auto tagstr = a_tags.empty() ? "[]"s : std::format("[{}]", [&] {
			return std::accumulate(std::next(a_tags.begin()), a_tags.end(), std::string(a_tags[0]), [](std::string a, std::string_view b) {
				return std::move(a) + ", " + b.data();
			});
		}());
		_hashbuilder.join();

		const std::shared_lock lock{ _mScenes };
		const auto where = this->scenes.find(hash);
		if (where == this->scenes.end()) {
			logger::warn("Invalid query: [{} | {} | {}]; No animations for given actors", a_actors.size(), hash.to_string(), tagstr);
			return {};
		}
		const auto& rawScenes = where->second;

		std::vector<const Scene*> ret{};
		ret.reserve(rawScenes.size());
		std::copy_if(rawScenes.begin(), rawScenes.end(), std::back_inserter(ret), [&](Scene* a_scene) {
			return a_scene->IsEnabled() && !a_scene->IsPrivate();
		});
		if (ret.empty()) {
			logger::warn("Invalid query: [{} | {} | {}]; 0/{} animations are enabled", a_actors.size(), hash.to_string(), tagstr, where->second.size());
			return {};
		}
		const auto removed = std::erase_if(ret, [&](const Scene* a_scene) {
			return !a_scene->IsCompatibleTags(tags);
		});
		if (ret.empty()) {
			logger::warn("Invalid query: [{} | {} | {}]; 0/{} animations use requested tags", a_actors.size(), hash.to_string(), tagstr, removed);
			return {};
		}
		const auto tEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> ms = tEnd - tStart;
		logger::info("Found {} scenes for query [{} | {} | {}] actors in {}ms", ret.size(), a_actors.size(), hash.to_string(), tagstr, ms.count());
		return ret;
	}

	std::vector<const Scene*> Library::GetByTags(int32_t a_positions, const std::vector<std::string_view>& a_tags) const
	{
		TagDetails tags{ a_tags };
		std::vector<const Scene*> ret{};
		ret.reserve(sceneMap.size() >> 5);
		const std::shared_lock lock{ _mScenes };
		for (auto&& [key, scene] : sceneMap) {
			if (!scene->IsEnabled() || scene->IsPrivate())
				continue;
			if (scene->positions.size() != a_positions)
				continue;
			if (!scene->IsCompatibleTags(tags))
				continue;
			ret.push_back(scene);
		}
		return ret;
	}

	const AnimPackage* Library::GetPackageFromScene(const Scene* a_scene) const
	{
		std::shared_lock lock{ _mScenes };
		for (auto&& package : packages) {
			if (std::ranges::contains(package->scenes, a_scene, [](const auto& scenePtr) { return scenePtr.get(); })) {
				return package.get();
			}
		}
		return nullptr;
	}

	const Scene* Library::GetSceneById(const RE::BSFixedString& a_id) const
	{
		std::shared_lock lock{ _mScenes };
		const auto where = sceneMap.find(a_id);
		return where != sceneMap.end() ? where->second : nullptr;
	}

	const Scene* Library::GetSceneByName(const RE::BSFixedString& a_name) const
	{
		std::shared_lock lock{ _mScenes };
		for (auto&& package : packages) {
			for (auto&& scene : package->scenes) {
				if (a_name == RE::BSFixedString(scene->name))
					return scene.get();
			}
		}
		return nullptr;
	}

	size_t Library::GetSceneCount() const
	{
		std::shared_lock lock{ _mScenes };
		return sceneMap.size();
	}

	bool Library::EditScene(const RE::BSFixedString& a_id, const std::function<void(Scene*)>& a_func)
	{
		auto scene = GetSceneById(a_id);
		if (!scene) {
			logger::error("Scene {} not found", a_id.c_str());
			return false;
		}
		EditScene(scene, a_func);
		return true;
	}

	void Library::EditScene(const Registry::Scene* a_scene, const std::function<void(Scene*)>& a_func)
	{
		std::unique_lock lock{ _mScenes };
		const auto scene = const_cast<Scene*>(a_scene);
		a_func(scene);
	}

	bool Library::ForEachPackage(std::function<bool(const AnimPackage*)> a_visitor) const
	{
		std::shared_lock lock{ _mScenes };
		for (auto&& package : packages) {
			if (a_visitor(package.get()))
				return true;
		}
		return false;
	}

	bool Library::ForEachScene(std::function<bool(const Scene*)> a_visitor) const
	{
		std::shared_lock lock{ _mScenes };
		for (auto&& [key, scene] : sceneMap) {
			if (a_visitor(scene))
				return true;
		}
		return false;
	}

	std::vector<RE::BSFixedString> Library::GetAllVoiceIds(RaceKey a_race) const
	{
		std::shared_lock lock{ _mVoice };
		return std::ranges::fold_left(voices, std::vector<RE::BSFixedString>{}, [&](auto acc, const auto& it) {
			const auto& [name, voice] = it;
			if (a_race.Is(RaceKey::None) || voice.HasRace(a_race))
				acc.push_back(name);
			return acc;
		});
	}

	bool Library::ForEachVoice(std::function<bool(const Voice&)> a_visitor) const
	{
		std::shared_lock lock{ _mVoice };
		for (auto&& [name, voice] : voices) {
			if (a_visitor(voice))
				return true;
		}
		return false;
	}

	const Voice* Library::GetVoice(RE::Actor* a_actor, const TagDetails& a_tags)
	{
		std::shared_lock lock{ _mVoice };
		if (auto saved = GetSavedVoice(a_actor->GetFormID())) {
			return saved;
		}
		const RaceKey actRace{ a_actor };
		if (!actRace.IsValid()) {
			logger::error("GetVoice: Actor {} has invalid racekey", a_actor->GetFormID());
			return nullptr;
		}
		const auto base = a_actor->GetActorBase();
		const auto sex = base ? base->GetSex() : RE::SEXES::kMale;
		std::vector<const Voice*> ret{};
		for (auto&& [name, voice] : voices) {
			if (!voice.enabled || voice.sex != RE::SEXES::kNone && voice.sex != sex)
				continue;
			if (!voice.HasRace(actRace) || !a_tags.MatchTags(voice.tags))
				continue;
			ret.push_back(&voice);
		}
		if (ret.empty())
			return nullptr;
		if (auto voiceForm = base->GetVoiceType()) {
			auto where = savedPitches.find(voiceForm->formID);
			if (where != savedPitches.end()) {
				const auto [pitch, voice] = where->second;
				if (voice != nullptr) return voice;
				const auto w = std::remove_if(ret.begin(), ret.end(), [&](const auto& v) {
					return v->pitch != Pitch::Unknown && v->pitch != pitch;
				});
				if (w != ret.begin() && w != ret.end()) {
					ret.erase(w, ret.end());
				}
			}
		}
		return savedVoices[a_actor->formID] = Random::draw(ret);
	}

	const Voice* Library::GetVoice(const TagDetails& tags) const
	{
		std::shared_lock lock{ _mVoice };
		std::vector<const Voice*> ret{};
		for (const auto& [_, voice] : voices) {
			if (!tags.MatchTags(voice.tags))
				continue;
			ret.push_back(&voice);
		}
		return ret.empty() ? nullptr : Random::draw(ret);
	}

	const Voice* Library::GetVoice(RaceKey a_race) const
	{
		std::shared_lock lock{ _mVoice };
		std::vector<const Voice*> ret{};
		for (auto&& [_, voice] : voices) {
			if (!voice.HasRace(a_race))
				continue;
			ret.push_back(&voice);
		}
		return ret.empty() ? nullptr : Random::draw(ret);
	}

	const Voice* Library::GetVoiceById(RE::BSFixedString a_voice) const
	{
		std::shared_lock lock{ _mVoice };
		auto v = voices.find(a_voice);
		return v == voices.end() ? nullptr : &v->second;
	}

	bool Library::CreateVoice(RE::BSFixedString a_voice)
	{
		std::unique_lock lock{ _mVoice };
		if (voices.contains(a_voice)) {
			logger::error("Voice {} has already been initialized", a_voice);
			return false;
		}
		voices.emplace(a_voice, Voice{ a_voice });
		return true;
	}

	void Library::WriteVoiceToFile(RE::BSFixedString a_voice) const
	{
		auto voice = GetVoiceById(a_voice);
		if (!voice) {
			logger::error("Voice {} not found", a_voice);
			return;
		}
		voice->SaveToFile(VOICE_PATH);
	}

	std::vector<RE::Actor*> Library::GetSavedActors() const
	{
		std::shared_lock lock{ _mVoice };
		return std::ranges::fold_left(savedVoices, std::vector<RE::Actor*>{}, [&](auto acc, const auto& it) {
			auto act = RE::TESForm::LookupByID<RE::Actor>(it.first);
			if (act) acc.push_back(act);
			return acc;
		});
	}

	const Voice* Library::GetSavedVoice(RE::FormID a_key) const
	{
		std::shared_lock lock{ _mVoice };
		auto w = savedVoices.find(a_key);
		return w == savedVoices.end() ? nullptr : w->second;
	}

	void Library::SaveVoice(RE::FormID a_key, RE::BSFixedString a_voice)
	{
		auto v = GetVoiceById(a_voice);
		std::unique_lock lock{ _mVoice };
		if (v) {
			savedVoices.insert_or_assign(a_key, v);
		} else {
			savedVoices.erase(a_key);
		}
	}

	void Library::ClearVoice(RE::FormID a_key)
	{
		std::unique_lock lock{ _mVoice };
		savedVoices.erase(a_key);
	}

	RE::TESSound* Library::PickSound(RE::BSFixedString a_voice, LegacyVoice a_legacysetting) const
	{
		auto voice = GetVoiceById(a_voice);
		if (!voice) {
			logger::error("Voice {} not found", a_voice);
			return nullptr;
		}
		std::shared_lock lock{ _mVoice };
		return voice->PickSound(a_legacysetting);
	}

	RE::TESSound* Library::PickSound(RE::BSFixedString a_voice, uint32_t a_excitement, REX::EnumSet<VoiceAnnotation> a_annotation) const
	{
		auto voice = GetVoiceById(a_voice);
		if (!voice) {
			logger::error("Voice {} not found", a_voice);
			return nullptr;
		}
		std::shared_lock lock{ _mVoice };
		return voice->PickSound(a_excitement, a_annotation);
	}

	RE::TESSound* Library::PickOrgasmSound(RE::BSFixedString a_voice, REX::EnumSet<VoiceAnnotation> a_annotation) const
	{
		auto voice = GetVoiceById(a_voice);
		if (!voice) {
			logger::error("Voice {} not found", a_voice);
			return nullptr;
		}
		std::shared_lock lock{ _mVoice };
		return voice->PickOrgasmSound(a_annotation);
	}

	void Library::SetVoiceEnabled(RE::BSFixedString a_voice, bool a_enabled)
	{
		std::unique_lock lock{ _mVoice };
		auto v = voices.find(a_voice);
		if (v == voices.end()) {
			logger::error("Voice {} not found", a_voice);
			return;
		}
		v->second.enabled = a_enabled;
	}

	void Library::SetVoiceSound(RE::BSFixedString a_voice, LegacyVoice a_legacysetting, RE::TESSound* a_sound)
	{
		std::unique_lock lock{ _mVoice };
		auto v = voices.find(a_voice);
		if (v == voices.end()) {
			logger::error("Voice {} not found", a_voice);
			return;
		}
		auto& voice = v->second;
		if (voice.extrasets.empty()) {
			logger::error("Voice {} has no extrasets", a_voice);
			return;
		}
		switch (a_legacysetting) {
		case LegacyVoice::Mild:
			voice.defaultset.SetSound(true, a_sound);
			break;
		case LegacyVoice::Medium:
			voice.extrasets.front().SetSound(true, a_sound);
			break;
		case LegacyVoice::Hot:
			voice.defaultset.SetSound(false, a_sound);
			voice.extrasets.front().SetSound(false, a_sound);
			break;
		}
	}

	void Library::SetVoiceTags(RE::BSFixedString a_voice, const std::vector<RE::BSFixedString>& a_tags)
	{
		std::unique_lock lock{ _mVoice };
		auto v = voices.find(a_voice);
		if (v == voices.end()) {
			logger::error("Voice {} not found", a_voice);
			return;
		}
		v->second.tags.AddTag(a_tags);
	}

	void Library::SetVoiceRace(RE::BSFixedString a_voice, const std::vector<RaceKey>& a_races)
	{
		std::unique_lock lock{ _mVoice };
		auto v = voices.find(a_voice);
		if (v == voices.end()) {
			logger::error("Voice {} not found", a_voice);
			return;
		}
		auto& voice = v->second;
		if (!a_races.empty() && !std::ranges::contains(a_races, RaceKey::Human, [](auto& it) { return it.value; })) {
			voice.tags.AddTag("Creature");
		} else {
			voice.tags.RemoveTag("Creature");
		}
		voice.races = a_races;
	}

	void Library::SetVoiceSex(RE::BSFixedString a_voice, RE::SEXES::SEX a_sex)
	{
		std::unique_lock lock{ _mVoice };
		auto v = voices.find(a_voice);
		if (v == voices.end()) {
			logger::error("Voice {} not found", a_voice);
			return;
		}
		auto& voice = v->second;
		switch (a_sex) {
		case RE::SEXES::kFemale:
			voice.tags.RemoveTag("Male");
			voice.tags.AddTag("Female");
			break;
		case RE::SEXES::kMale:
			voice.tags.AddTag("Male");
			voice.tags.RemoveTag("Female");
			break;
		default:
			voice.tags.AddTag("Female");
			voice.tags.AddTag("Male");
			break;
		}
		voice.sex = a_sex;
	}

	const Expression* Library::GetExpressionById(const RE::BSFixedString& a_id) const
	{
		std::shared_lock lock{ _mExpressions };
		auto where = expressions.find(a_id);
		return where == expressions.end() ? nullptr : &where->second;
	}

	const Expression* Library::GetExpression(const TagDetails& a_details) const
	{
		std::shared_lock lock{ _mExpressions };
		std::vector<const Expression*> ret{};
		for (auto&& [id, expression] : expressions) {
			if (a_details.MatchTags(expression.GetTags())) {
				ret.push_back(&expression);
			}
		}
		return ret.empty() ? nullptr : Random::draw(ret);
	}

	bool Library::ForEachExpression(std::function<bool(const Expression&)> a_func) const
	{
		std::shared_lock lock{ _mExpressions };
		for (auto&& [id, expression] : expressions) {
			if (a_func(expression)) {
				return true;
			}
		}
		return false;
	}

	bool Library::CreateExpression(const RE::BSFixedString& a_id)
	{
		std::unique_lock lock{ _mExpressions };
		if (expressions.contains(a_id)) {
			logger::error("Expression {} has already been initialized", a_id);
			return false;
		}
		expressions.emplace(a_id, Expression{ a_id });
		return true;
	}

	void Library::UpdateExpressionValues(RE::BSFixedString a_id, bool a_female, int a_level, std::vector<float> a_values)
	{
		std::unique_lock lock{ _mExpressions };
		auto w = expressions.find(a_id);
		if (w == expressions.end()) {
			logger::error("Expression {} not found", a_id);
			return;
		}
		w->second.UpdateValues(a_female, a_level, a_values);
	}

	void Library::UpdateExpressionTags(RE::BSFixedString a_id, const TagData& a_newtags)
	{
		std::unique_lock lock{ _mExpressions };
		auto w = expressions.find(a_id);
		if (w == expressions.end()) {
			logger::error("Expression {} not found", a_id);
			return;
		}
		w->second.UpdateTags(a_newtags);
	}

	void Library::SetExpressionScaling(RE::BSFixedString a_id, Expression::Scaling a_scaling)
	{
		std::unique_lock lock{ _mExpressions };
		auto w = expressions.find(a_id);
		if (w == expressions.end()) {
			logger::error("Expression {} not found", a_id);
			return;
		}
		w->second.SetScaling(a_scaling);
	}

	void Library::SetExpressionEnabled(RE::BSFixedString a_id, bool a_enabled)
	{
		std::unique_lock lock{ _mExpressions };
		auto w = expressions.find(a_id);
		if (w == expressions.end()) {
			logger::error("Expression {} not found", a_id);
			return;
		}
		w->second.SetEnabled(a_enabled);
	}

	const FurnitureDetails* Library::GetFurnitureDetails(const RE::TESObjectREFR* a_ref) const
	{
		if (a_ref->Is(RE::FormType::ActorCharacter)) {
			return nullptr;
		}
		const auto ref = a_ref->GetObjectReference();
		if (const auto tesmodel = ref ? ref->As<RE::TESModel>() : nullptr) {
			std::shared_lock lock{ _mFurniture };
			const auto where = furnitures.find(tesmodel->model);
			if (where != furnitures.end()) {
				return where->second.get();
			}
		}
		switch (FurnitureType::GetBedType(a_ref)) {
		case FurnitureType::BedSingle:
			return &offsetDefaultBedsingle;
		case FurnitureType::BedDouble:
			return &offsetDefaultBeddouble;
		case FurnitureType::BedRoll:
			return &offsetDefaultBedroll;
		}
		return nullptr;
	}
}
