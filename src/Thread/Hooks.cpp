#include "Hooks.h"

#include "Thread.h"

namespace Thread::Hooks
{
    namespace
    {
        struct FrameUpdateHook
        {
            static void Install()
            {
                auto& trampoline = SKSE::GetTrampoline();
                const auto address = REL::VariantID(35565, 36564, 0x5BAB10).address();
                uint64_t offset_AE_1p6=0xC26;
                uint64_t offset_AE_1p7=0xC38;
                bool is_AE_1p7=(REL::Module::get().version() >= REL::Version(1, 7, 0, 0));
                const auto offset = REL::VariantOffset(0x748, (is_AE_1p7 ? offset_AE_1p7 : offset_AE_1p6), 0x7EE).offset();
                original = trampoline.write_call<5>(address + offset, Thunk);
                logger::info("Installed frame hook");
            }

            static void Thunk()
            {
                static REL::Relocation<float*> deltaTime{ REL::VariantID(523660, 410199, 0x30C3A08) };
                Instance::UpdateAnimations(*deltaTime);
                original();
            }

            static inline REL::Relocation<decltype(Thunk)> original;
        };

        struct DrawWeaponHook
        {
            static void Install()
            {
                REL::Relocation<std::uintptr_t> vtable{ RE::VTABLE_PlayerCharacter[0] };
                const auto slot = REL::Module::IsVR() ? 0xA8 : 0xA6;

                original = vtable.write_vfunc(slot, Thunk);
                logger::info("Installed player draw weapon hook");
            }

			// This is called when either a mod, or the player tries to unsheathe their weapon
            static void Thunk(RE::PlayerCharacter* a_player, bool a_draw)
            {
                if (!a_draw || !blockDraw.load(std::memory_order_relaxed)) {
                    original(a_player, a_draw);
                }
            }

            static inline std::atomic_bool blockDraw{ false };
            static inline REL::Relocation<decltype(Thunk)> original;
        };
    }

    void Install()
    {
        FrameUpdateHook::Install();
        DrawWeaponHook::Install();
    }

    void SetWeaponDrawBlocked(bool a_blocked) noexcept
    {
        DrawWeaponHook::blockDraw.store(a_blocked, std::memory_order_relaxed);
    }
}
