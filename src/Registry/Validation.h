#pragma once

namespace Registry
{
    /// @brief Validate the given actor
    /// @return some code (-inf; 1]. 1 if the actor is valid. See implementation for details
    int32_t IsValidActorImpl(RE::Actor* a_actor);
    bool IsValidActor(RE::Actor* a_actor);
}  // namespace Registry
