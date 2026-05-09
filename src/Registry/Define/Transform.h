#pragma once

namespace Registry
{
    enum CoordinateType : uint8_t
    {
        X = 0,
        Y,
        Z,
        R,

        Total
    };

    struct Coordinate
    {
        Coordinate() = default;
        Coordinate(const RE::TESObjectREFR* a_ref);
        Coordinate(const RE::NiPoint3& a_point, float a_rotation);
        Coordinate(float a_x, float a_y, float a_z, float a_rotation);
        Coordinate(const std::vector<float>& a_coordinates);
        Coordinate(std::ifstream& a_stream);
        ~Coordinate() = default;

        void Apply(Coordinate& a_coordinate) const;
        Coordinate ApplyReturn(const Coordinate& a_coordinate) const;

        RE::NiPoint3 AsNiPoint() const { return { location.x, location.y, location.z }; }
        glm::vec4 AsVec4(float w = 0.0f) const { return { location.x, location.y, location.z, w }; }
        std::vector<float> AsVector() const { return { location.x, location.y, location.z, rotation }; }
        float GetDistance(const Coordinate& a_other) const { return glm::distance(location, a_other.location); }

      public:
        bool operator==(const Coordinate& a_rhs) const { return location == a_rhs.location && rotation == a_rhs.rotation; }

      public:
        glm::vec3 location;
        float rotation;
    };

    class Transform
    {
      public:
        Transform(const Coordinate& a_rawcoordinates);
        Transform(std::ifstream& a_binarystream);
        Transform() = default;
        ~Transform() = default;

      public:
        void Apply(Coordinate& a_coordinate) const { _offset.Apply(a_coordinate); }
        Coordinate ApplyReturn(const Coordinate& a_coordinate) const;
        bool HasChanges() const;

        const Coordinate& GetRawOffset() const;
        const Coordinate& GetOffset() const;
        float GetOffset(CoordinateType a_type) const;
        void SetOffset(float a_value, CoordinateType a_type);
        void SetOffset(const Coordinate& a_coordinate);
        void SetOffset(float x, float y, float z, float rot);
        void ResetOffset();

        void Save(YAML::Node& a_node) const;
        void Load(const YAML::Node& a_node);

      private:
        Coordinate _raw;
        Coordinate _offset;
    };

}  // namespace Registry
