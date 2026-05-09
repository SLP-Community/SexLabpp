#pragma once


namespace SKEE
{
    class IPluginInterface
    {
      public:
        IPluginInterface(){};
        virtual ~IPluginInterface(){};

        virtual uint32_t GetVersion() = 0;
        virtual void Revert() = 0;
    };

    class IInterfaceMap
    {
      public:
        virtual IPluginInterface* QueryInterface(const char* name) = 0;
        virtual bool AddInterface(const char* name, IPluginInterface* pluginInterface) = 0;
        virtual IPluginInterface* RemoveInterface(const char* name) = 0;
    };

    struct InterfaceExchangeMessage
    {
        enum : uint32_t
        {
            kMessage_ExchangeInterface = 0x9E3779B9
        };

        IInterfaceMap* interfaceMap = NULL;
    };

    class IAddonAttachmentInterface
    {
      public:
        virtual void OnAttach(RE::TESObjectREFR* refr, RE::TESObjectARMO* armor, RE::TESObjectARMA* addon, RE::NiAVObject* object, bool isFirstPerson, RE::NiNode* skeleton, RE::NiNode* root) = 0;
    };

    class IBodyMorphInterface : public IPluginInterface
    {
      public:
        class MorphKeyVisitor
        {
          public:
            virtual void Visit(const char*, float) = 0;
        };

        class StringVisitor
        {
          public:
            virtual void Visit(const char*) = 0;
        };

        class ActorVisitor
        {
          public:
            virtual void Visit(RE::TESObjectREFR*) = 0;
        };

        class MorphValueVisitor
        {
          public:
            virtual void Visit(RE::TESObjectREFR*, const char*, const char*, float) = 0;
        };

        class MorphVisitor
        {
          public:
            virtual void Visit(RE::TESObjectREFR*, const char*) = 0;
        };

        virtual void SetMorph(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey, float relative) = 0;
        virtual float GetMorph(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey) = 0;
        virtual void ClearMorph(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey) = 0;

        virtual float GetBodyMorphs(RE::TESObjectREFR* actor, const char* morphName) = 0;
        virtual void ClearBodyMorphNames(RE::TESObjectREFR* actor, const char* morphName) = 0;

        virtual void VisitMorphs(RE::TESObjectREFR* actor, MorphVisitor& visitor) = 0;
        virtual void VisitKeys(RE::TESObjectREFR* actor, const char* name, MorphKeyVisitor& visitor) = 0;
        virtual void VisitMorphValues(RE::TESObjectREFR* actor, MorphValueVisitor& visitor) = 0;

        virtual void ClearMorphs(RE::TESObjectREFR* actor) = 0;

        virtual void ApplyVertexDiff(RE::TESObjectREFR* refr, RE::NiAVObject* rootNode, bool erase = false) = 0;

        virtual void ApplyBodyMorphs(RE::TESObjectREFR* refr, bool deferUpdate = true) = 0;
        virtual void UpdateModelWeight(RE::TESObjectREFR* refr, bool immediate = false) = 0;

        virtual void SetCacheLimit(size_t limit) = 0;
        virtual bool HasMorphs(RE::TESObjectREFR* actor) = 0;
        virtual uint32_t EvaluateBodyMorphs(RE::TESObjectREFR* actor) = 0;

        virtual bool HasBodyMorph(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey) = 0;
        virtual bool HasBodyMorphName(RE::TESObjectREFR* actor, const char* morphName) = 0;
        virtual bool HasBodyMorphKey(RE::TESObjectREFR* actor, const char* morphKey) = 0;
        virtual void ClearBodyMorphKeys(RE::TESObjectREFR* actor, const char* morphKey) = 0;
        virtual void VisitStrings(StringVisitor& visitor) = 0;
        virtual void VisitActors(ActorVisitor& visitor) = 0;
        virtual size_t ClearMorphCache() = 0;
    };

    class INiTransformInterface : public IPluginInterface
    {
      public:
        struct Position
        {
            float x, y, z;
        };
        struct Rotation
        {
            float heading, attitude, bank;
        };

        // Visits all overrides within a set
        class NodeVisitor
        {
          public:
            virtual bool VisitPosition(const char* node, const char* key, Position& position) = 0;
            virtual bool VisitRotation(const char* node, const char* key, Rotation& rotation) = 0;
            virtual bool VisitScale(const char* node, const char* key, float scale) = 0;
            virtual bool VisitScaleMode(const char* node, const char* key, uint32_t scaleMode) = 0;
        };

        virtual bool HasNodeTransformPosition(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual bool HasNodeTransformRotation(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual bool HasNodeTransformScale(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual bool HasNodeTransformScaleMode(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;

        virtual void AddNodeTransformPosition(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name, Position& position) = 0;  // X,Y,Z
        virtual void AddNodeTransformRotation(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name, Rotation& rotation) = 0;  // Euler angles
        virtual void AddNodeTransformScale(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name, float scale) = 0;
        virtual void AddNodeTransformScaleMode(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name, uint32_t scaleMode) = 0;

        virtual Position GetNodeTransformPosition(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual Rotation GetNodeTransformRotation(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual float GetNodeTransformScale(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual uint32_t GetNodeTransformScaleMode(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;

        virtual bool RemoveNodeTransformPosition(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual bool RemoveNodeTransformRotation(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual bool RemoveNodeTransformScale(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual bool RemoveNodeTransformScaleMode(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;

        virtual bool RemoveNodeTransform(RE::TESObjectREFR* refr, bool firstPerson, bool isFemale, const char* node, const char* name) = 0;
        virtual void RemoveAllReferenceTransforms(RE::TESObjectREFR* refr) = 0;

        virtual bool GetOverrideNodeTransform(RE::TESObjectREFR* refr, bool firstPerson, bool isFemale, const char* node, const char* name, uint16_t key, RE::NiTransform* result) = 0;

        virtual void UpdateNodeAllTransforms(RE::TESObjectREFR* ref) = 0;

        virtual void VisitNodes(RE::TESObjectREFR* refr, bool firstPerson, bool isFemale, NodeVisitor& visitor) = 0;
        virtual void UpdateNodeTransforms(RE::TESObjectREFR* ref, bool firstPerson, bool isFemale, const char* node) = 0;
    };

    class IAttachmentInterface : public IPluginInterface
    {
      public:
        virtual bool AttachMesh(RE::TESObjectREFR* ref, const char* nifPath, const char* name, bool firstPerson, bool replace, const char** filter, uint32_t filterSize, RE::NiAVObject*& out, char* err, size_t errBufLen) = 0;
        virtual bool DetachMesh(RE::TESObjectREFR* ref, const char* name, bool firstPerson) = 0;
    };

    class IItemDataInterface : public IPluginInterface
    {
      public:
        // Use this data structure to form an item query, this will identify a specific item through various means
        struct Identifier
        {
            enum
            {
                kTypeNone = 0,
                kTypeRank = (1 << 0),
                kTypeUID = (1 << 1),
                kTypeSlot = (1 << 2),
                kTypeSelf = (1 << 3),
                kTypeDirect = (1 << 4)
            };

            enum
            {
                kHandSlot_Left = 0,
                kHandSlot_Right
            };

            uint16_t type = kTypeNone;
            uint16_t uid = 0;
            uint32_t ownerForm = 0;
            uint32_t weaponSlot = 0;
            uint32_t slotMask = 0;
            uint32_t rankId = 0;
            RE::TESForm* form = nullptr;
            RE::BaseExtraList* extraData = nullptr;

            void SetRankID(uint32_t _rank)
            {
                type |= kTypeRank;
                rankId = _rank;
            }
            void SetSlotMask(uint32_t _slotMask, uint32_t _weaponSlot = 0)
            {
                type |= kTypeSlot;
                slotMask = _slotMask;
                weaponSlot = _weaponSlot;
            }
            void SetUniqueID(uint16_t _uid, uint32_t _ownerForm)
            {
                type |= kTypeUID;
                uid = _uid;
                ownerForm = _ownerForm;
            }
            void SetDirect(RE::TESForm* _baseForm, RE::BaseExtraList* _extraData)
            {
                type |= kTypeDirect;
                form = _baseForm;
                extraData = _extraData;
            }

            bool IsDirect()
            {
                return (type & kTypeDirect) == kTypeDirect;
            }

            bool IsSelf()
            {
                return (type & kTypeSelf) == kTypeSelf;
            }

            void SetSelf()
            {
                type |= kTypeSelf;
            }
        };

        class StringVisitor
        {
          public:
            virtual void Visit(const char*) = 0;
        };

        virtual uint32_t GetItemUniqueID(RE::TESObjectREFR* reference, Identifier& identifier, bool makeUnique) = 0;  // Make unique will create an identifier if it does not exist for the specified item
        virtual void SetItemTextureLayerColor(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex, uint32_t color) = 0;
        virtual void SetItemTextureLayerType(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex, uint32_t type) = 0;
        virtual void SetItemTextureLayerBlendMode(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex, const char* blendMode) = 0;
        virtual void SetItemTextureLayerTexture(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex, const char* texture) = 0;

        virtual uint32_t GetItemTextureLayerColor(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex) = 0;
        virtual uint32_t GetItemTextureLayerType(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex) = 0;
        virtual bool GetItemTextureLayerBlendMode(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex, StringVisitor& visitor) = 0;
        virtual bool GetItemTextureLayerTexture(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex, StringVisitor& visitor) = 0;

        virtual void ClearItemTextureLayerColor(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex) = 0;
        virtual void ClearItemTextureLayerType(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex) = 0;
        virtual void ClearItemTextureLayerBlendMode(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex) = 0;
        virtual void ClearItemTextureLayerTexture(uint32_t uniqueID, int32_t textureIndex, int32_t layerIndex) = 0;
        virtual void ClearItemTextureLayer(uint32_t uniqueID, int32_t textureIndex) = 0;

        virtual RE::TESForm* GetFormFromUniqueID(uint32_t uniqueID) = 0;
        virtual RE::TESForm* GetOwnerOfUniqueID(uint32_t uniqueID) = 0;

        // Generic key-value pair string interface
        virtual bool HasItemData(uint32_t uniqueID, const char* key) = 0;
        virtual bool GetItemData(uint32_t uniqueID, const char* key, StringVisitor& visitor) = 0;
        virtual void SetItemData(uint32_t uniqueID, const char* key, const char* value) = 0;
        virtual void ClearItemData(uint32_t uniqueID, const char* key) = 0;
    };

    class ICommandInterface : public IPluginInterface
    {
      public:
        // Return true indicates callback was handled and not to proceed to next command with the same command name
        using CommandCallback = bool (*)(RE::TESObjectREFR* ref, const char* argumentString);
        virtual bool RegisterCommand(const char* command, const char* desc, CommandCallback cb) = 0;
    };

    class IOverlayInterface : public IPluginInterface, public IAddonAttachmentInterface
    {
      public:
        virtual uint32_t GetVersion() = 0;

        virtual void Save(SKSE::SerializationInterface* intfc, uint32_t kVersion) = 0;
        virtual bool Load(SKSE::SerializationInterface* intfc, uint32_t kVersion) = 0;
        virtual void Revert() = 0;

        virtual bool HasOverlays(RE::TESObjectREFR* reference) = 0;
        virtual void AddOverlays(RE::TESObjectREFR* reference) = 0;
        virtual void RemoveOverlays(RE::TESObjectREFR* reference) = 0;
        virtual void InstallOverlay(const char* nodeName, const char* path, RE::TESObjectREFR* refr, RE::BSGeometry* source, RE::NiNode* destination, RE::BSTextureSet* textureSet = NULL) = 0;
    };

    class IActorUpdateManager : public IPluginInterface
    {
      public:
        virtual void AddBodyUpdate(uint32_t formId) = 0;
        virtual void AddTransformUpdate(uint32_t formId) = 0;
        virtual void AddOverlayUpdate(uint32_t formId) = 0;
        virtual void AddNodeOverrideUpdate(uint32_t formId) = 0;
        virtual void AddWeaponOverrideUpdate(uint32_t formId) = 0;
        virtual void AddAddonOverrideUpdate(uint32_t formId) = 0;
        virtual void AddSkinOverrideUpdate(uint32_t formId) = 0;
        virtual void Flush() = 0;
        virtual void AddInterface(IAddonAttachmentInterface* observer) = 0;
        virtual void RemoveInterface(IAddonAttachmentInterface* observer) = 0;
    };

    inline IInterfaceMap* GetInterfaceMap()
    {
        InterfaceExchangeMessage msg;
        SKSE::GetMessagingInterface()->Dispatch(
            InterfaceExchangeMessage::kMessage_ExchangeInterface,
            &msg,
            sizeof(InterfaceExchangeMessage*),
            "SKEE");
        return msg.interfaceMap;
    }

    // g_interfaceMap.AddInterface("Override", &g_overrideInterface);
    // g_interfaceMap.AddInterface("Overlay", &g_overlayInterface);
    // g_interfaceMap.AddInterface("NiTransform", &g_transformInterface);
    // g_interfaceMap.AddInterface("BodyMorph", &g_bodyMorphInterface);
    // g_interfaceMap.AddInterface("ItemData", &g_itemDataInterface);
    // g_interfaceMap.AddInterface("TintMask", &g_tintMaskInterface);
    // g_interfaceMap.AddInterface("FaceMorph", &g_morphInterface);
    // g_interfaceMap.AddInterface("ActorUpdateManager", &g_actorUpdateManager);
    // g_interfaceMap.AddInterface("Attachment", &g_attachmentInterface);
    // g_interfaceMap.AddInterface("Command", &g_commandInterface);

    inline IBodyMorphInterface* GetBodyMorphInterface(IInterfaceMap* a_map)
    {
        auto intfc = a_map->QueryInterface("BodyMorph");
        return static_cast<IBodyMorphInterface*>(intfc);
    }

    inline INiTransformInterface* GetNiTransformInterface(IInterfaceMap* a_map)
    {
        auto intfc = a_map->QueryInterface("NiTransform");
        return static_cast<INiTransformInterface*>(intfc);
    }

    inline IOverlayInterface* GetOverlayInterface(IInterfaceMap* a_map)
    {
        auto intfc = a_map->QueryInterface("Overlay");
        return static_cast<IOverlayInterface*>(intfc);
    }

    inline IAttachmentInterface* GetAttachmentInterface(IInterfaceMap* a_map)
    {
        auto intfc = a_map->QueryInterface("Attachment");
        return static_cast<IAttachmentInterface*>(intfc);
    }

    inline IActorUpdateManager* GetActorUpdateManager(IInterfaceMap* a_map)
    {
        auto intfc = a_map->QueryInterface("ActorUpdateManager");
        return static_cast<IActorUpdateManager*>(intfc);
    }

}  // namespace SKEE
