#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

// values smaller / larger than this will not be counted as valid by IsValid
inline constexpr float POSITIVE_INVALID_THRESHHOLD = 999999.0f;
inline constexpr float NEGATIVE_INVALID_THRESHHOLD = -999999.0f;

namespace Misc::Math
{
    glm::vec3 QuatToEuler(glm::quat q)
    {
        auto matrix = glm::toMat4(q);

        glm::vec3 rotOut;
        glm::extractEulerAngleXYZ(matrix, rotOut.x, rotOut.y, rotOut.z);

        return rotOut;
    }

    glm::vec3 RotMatrixToEuler(RE::NiMatrix3 matrixIn)
    {
        auto ent = matrixIn.entry;
        auto rotMat = glm::mat4(
            { ent[0][0], ent[1][0], ent[2][0],
                ent[0][1], ent[1][1], ent[2][1],
                ent[0][2], ent[1][2], ent[2][2] });

        glm::vec3 rotOut;
        glm::extractEulerAngleXYZ(rotMat, rotOut.x, rotOut.y, rotOut.z);

        return rotOut;
    }

    bool IsValid(float numIn)
    {
        return !(numIn < NEGATIVE_INVALID_THRESHHOLD ||
                 numIn > POSITIVE_INVALID_THRESHHOLD ||
                 std::isinf(numIn) ||
                 std::isinf(-numIn) ||
                 std::isnan(numIn) ||
                 std::isnan(-numIn));
    }

    bool IsValid(glm::vec3 vecIn)
    {
        return IsValid(vecIn.x) && IsValid(vecIn.y) && IsValid(vecIn.z);
    }

    float MakeValid(float numIn, float setInvalidTo)
    {
        if (IsValid(numIn))
            return numIn;
        return setInvalidTo;
    }

    glm::vec3 MakeValid(glm::vec3 vecIn, float setInvalidTo)
    {
        return glm::vec3(
            MakeValid(vecIn.x, setInvalidTo),
            MakeValid(vecIn.y, setInvalidTo),
            MakeValid(vecIn.z, setInvalidTo));
    }

    glm::vec3 RotateVector(glm::quat quatIn, glm::vec3 vecIn)
    {
        float num = quatIn.x * 2.0f;
        float num2 = quatIn.y * 2.0f;
        float num3 = quatIn.z * 2.0f;
        float num4 = quatIn.x * num;
        float num5 = quatIn.y * num2;
        float num6 = quatIn.z * num3;
        float num7 = quatIn.x * num2;
        float num8 = quatIn.x * num3;
        float num9 = quatIn.y * num3;
        float num10 = quatIn.w * num;
        float num11 = quatIn.w * num2;
        float num12 = quatIn.w * num3;
        glm::vec3 result;
        result.x = (1.0f - (num5 + num6)) * vecIn.x + (num7 - num12) * vecIn.y + (num8 + num11) * vecIn.z;
        result.y = (num7 + num12) * vecIn.x + (1.0f - (num4 + num6)) * vecIn.y + (num9 - num10) * vecIn.z;
        result.z = (num8 - num11) * vecIn.x + (num9 + num10) * vecIn.y + (1.0f - (num4 + num5)) * vecIn.z;
        return result;
    }

    glm::vec3 RotateVector(glm::vec3 eulerIn, glm::vec3 vecIn)
    {
        glm::vec3 glmVecIn(vecIn.x, vecIn.y, vecIn.z);
        glm::mat3 rotationMatrix = glm::eulerAngleXYZ(eulerIn.x, eulerIn.y, eulerIn.z);

        return rotationMatrix * glmVecIn;
    }
}
