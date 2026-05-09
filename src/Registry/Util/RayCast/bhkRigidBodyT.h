#pragma once

class bhkRigidBodyT : public RE::bhkRigidBody
{
  public:
    inline static constexpr auto RTTI = RE::RTTI_bhkRigidBodyT;
    inline static constexpr auto Ni_RTTI = RE::NiRTTI_bhkRigidBodyT;

    ~bhkRigidBodyT() override;  // 00

    RE::hkQuaternion rotation;  // 40
    RE::hkVector4 translation;  // 50
};

static_assert(offsetof(bhkRigidBodyT, rotation) == 0x40);
static_assert(offsetof(bhkRigidBodyT, translation) == 0x50);
static_assert(sizeof(bhkRigidBodyT) == 0x60);
