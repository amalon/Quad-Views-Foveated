// Ported from XrMath.h by James Hogan
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <openxr/openxr.h>
#include <stdexcept>
#include <cmath>

#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace xr::math {
    constexpr float QuaternionEpsilon = 0.01f;

    // A large number that can be used as maximum finite depth value, beyond which a value can be treated as infinity
    constexpr float OneOverFloatEpsilon = 1.0f / std::numeric_limits<float>::epsilon();

    namespace Pose {
        constexpr XrPosef Identity();
        constexpr XrPosef Translation(const XrVector3f& translation);

        XrPosef LookAt(const XrVector3f& origin, const XrVector3f& forward, const XrVector3f& up);
        XrPosef Multiply(const XrPosef& a, const XrPosef& b);
        XrPosef Slerp(const XrPosef& a, const XrPosef& b, float alpha);
        XrPosef Invert(const XrPosef& pose);

        constexpr bool IsPoseValid(const XrSpaceLocation& location);
        constexpr bool IsPoseTracked(const XrSpaceLocation& location);
        constexpr bool IsPoseValid(const XrHandJointLocationEXT& jointLocation);
        constexpr bool IsPoseTracked(const XrHandJointLocationEXT& jointLocation);
        constexpr bool IsPoseValid(const XrViewState& viewState);
        constexpr bool IsPoseTracked(const XrViewState& viewState);

        template <typename Quaternion, typename Vector3>
        inline XrPosef MakePose(const Quaternion& orientation, const Vector3& position);
    } // namespace Pose

    namespace Quaternion {
        constexpr XrQuaternionf Identity();
        bool IsNormalized(const XrQuaternionf& quaternion);
        XrQuaternionf RotationAxisAngle(const XrVector3f& axis, float angleInRadians);
        XrQuaternionf RotationRollPitchYaw(const XrVector3f& eulerAnglesInRadians);
        XrQuaternionf Slerp(const XrQuaternionf& a, const XrQuaternionf& b, float alpha);
    } // namespace Quaternion

    struct NearFar {
        float Near;
        float Far;
    };

#if 0 // FIXME NearFar changes meaning
    struct ViewProjection {
        XrPosef Pose;
        XrFovf Fov;
        NearFar NearFar;
    };
#endif

    // Type conversion between math types.
    // If you get the error "attempting to reference a deleted function" then you need to implement
    // xr::math::detail::implement_math_cast for types X and Y. See examples further down.
    template <typename X, typename Y>
    constexpr const X& cast(const Y& value) = delete;

    // Projection matrix math
    glm::mat4 ComposeProjectionMatrix(const XrFovf& fov, const NearFar& nearFar);
    //NearFar GetProjectionNearFar(const glm::mat4& projectionMatrix);
    XrFovf DecomposeProjectionMatrix(const glm::mat4& projectionMatrix);
} // namespace xr::math

#pragma region Implementation

namespace xr::math {
    namespace detail {
        template <typename X, typename Y>
        constexpr const X& implement_math_cast(const Y& value) {
            static_assert(std::is_trivially_copyable<X>::value, "Unsafe to cast between non-POD types.");
            static_assert(std::is_trivially_copyable<Y>::value, "Unsafe to cast between non-POD types.");
            static_assert(!std::is_pointer<X>::value, "Incorrect cast between pointer types.");
            static_assert(!std::is_pointer<Y>::value, "Incorrect cast between pointer types.");
            static_assert(sizeof(X) == sizeof(Y), "Incorrect cast between types with different sizes.");
            return reinterpret_cast<const X&>(value);
        }

        template <typename X, typename Y>
        constexpr X& implement_math_cast(Y& value) {
            static_assert(std::is_trivially_copyable<X>::value, "Unsafe to cast between non-POD types.");
            static_assert(std::is_trivially_copyable<Y>::value, "Unsafe to cast between non-POD types.");
            static_assert(!std::is_pointer<X>::value, "Incorrect cast between pointer types.");
            static_assert(!std::is_pointer<Y>::value, "Incorrect cast between pointer types.");
            static_assert(sizeof(X) == sizeof(Y), "Incorrect cast between types with different sizes.");
            return reinterpret_cast<X&>(value);
        }
    } // namespace detail

#define VECTOR2F_OPERATOR(op)                                                    \
    constexpr XrVector2f operator op(const XrVector2f& a, const XrVector2f& b) { \
        return XrVector2f{a.x op b.x, a.y op b.y};                               \
    }
    VECTOR2F_OPERATOR(+);
    VECTOR2F_OPERATOR(-);
    VECTOR2F_OPERATOR(*);
    VECTOR2F_OPERATOR(/);
#undef VECTOR2F_OPERATOR

#define VECTOR2F_OPERATOR(op)                                        \
    constexpr XrVector2f operator op(const XrVector2f& a, float s) { \
        return XrVector2f{a.x op s, a.y op s};                       \
    }
    VECTOR2F_OPERATOR(+);
    VECTOR2F_OPERATOR(-);
    VECTOR2F_OPERATOR(*);
    VECTOR2F_OPERATOR(/);
#undef VECTOR2F_OPERATOR

#define VECTOR2F_OPERATOR(op)                                        \
    constexpr XrVector2f operator op(float s, const XrVector2f& a) { \
        return XrVector2f{s op a.x, s op a.y};                       \
    }
    VECTOR2F_OPERATOR(+);
    VECTOR2F_OPERATOR(-);
    VECTOR2F_OPERATOR(*);
    VECTOR2F_OPERATOR(/);
#undef VECTOR2F_OPERATOR

#define VECTOR3F_OPERATOR(op)                                                    \
    constexpr XrVector3f operator op(const XrVector3f& a, const XrVector3f& b) { \
        return XrVector3f{a.x op b.x, a.y op b.y, a.z op b.z};                   \
    }
    VECTOR3F_OPERATOR(+);
    VECTOR3F_OPERATOR(-);
    VECTOR3F_OPERATOR(*);
    VECTOR3F_OPERATOR(/);
#undef VECTOR3F_OPERATOR

#define VECTOR3F_OPERATOR(op)                                        \
    constexpr XrVector3f operator op(const XrVector3f& a, float s) { \
        return XrVector3f{a.x op s, a.y op s, a.z op s};             \
    }
    VECTOR3F_OPERATOR(+);
    VECTOR3F_OPERATOR(-);
    VECTOR3F_OPERATOR(*);
    VECTOR3F_OPERATOR(/);
#undef VECTOR3F_OPERATOR

#define VECTOR3F_OPERATOR(op)                                        \
    constexpr XrVector3f operator op(float s, const XrVector3f& a) { \
        return XrVector3f{s op a.x, s op a.y, s op a.z};             \
    }
    VECTOR3F_OPERATOR(+);
    VECTOR3F_OPERATOR(-);
    VECTOR3F_OPERATOR(*);
    VECTOR3F_OPERATOR(/);
#undef VECTOR3F_OPERATOR

    inline glm::vec2 LoadXrVector2(const XrVector2f& vector) {
        return glm::vec2(vector.x, vector.y);
    }

    inline glm::vec3 LoadXrVector3(const XrVector3f& vector) {
        return glm::vec3(vector.x, vector.y, vector.z);
    }

    inline glm::vec4 LoadXrVector4(const XrVector4f& vector) {
        return glm::vec4(vector.x, vector.y, vector.z, vector.w);
    }

    inline glm::quat LoadXrQuaternion(const XrQuaternionf& quaternion) {
        return glm::quat(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
    }

#if 0
    inline DirectX::XMVECTOR XM_CALLCONV LoadXrExtent(const XrExtent2Df& extend) {
        return DirectX::XMLoadFloat2(&xr::math::cast(extend));
    }
#endif

    inline glm::mat4 LoadXrPose(const XrPosef& pose) {
        glm::quat orientation = LoadXrQuaternion(pose.orientation);
        glm::vec3 position = LoadXrVector3(pose.position);
        glm::mat4 matrix = glm::mat4_cast(orientation);
        matrix[3] += glm::vec4(position, 0.0f);
        return matrix;
    }

#ifdef _WIN32
    inline DirectX::XMMATRIX XM_CALLCONV LoadInvertedXrPose(const XrPosef& pose) {
        return LoadXrPose(Pose::Invert(pose));
    }
#endif

    inline void StoreXrVector2(XrVector2f* outVec, const glm::vec2 &inVec) {
        outVec->x = inVec[0];
        outVec->y = inVec[1];
    }

    inline void StoreXrVector3(XrVector3f* outVec, const glm::vec3 &inVec) {
        outVec->x = inVec[0];
        outVec->y = inVec[1];
        outVec->z = inVec[2];
    }

    inline void StoreXrVector4(XrVector4f* outVec, const glm::vec4 &inVec) {
        outVec->x = inVec[0];
        outVec->y = inVec[1];
        outVec->z = inVec[2];
        outVec->w = inVec[3];
    }

    inline void StoreXrQuaternion(XrQuaternionf* outQuat, const glm::quat &inQuat) {
        outQuat->x = inQuat[0];
        outQuat->y = inQuat[1];
        outQuat->z = inQuat[2];
        outQuat->w = inQuat[3];
    }

#if _WIN32
    inline void XM_CALLCONV StoreXrExtent(XrExtent2Df* outVec, DirectX::FXMVECTOR inVec) {
        DirectX::XMStoreFloat2(&detail::implement_math_cast<DirectX::XMFLOAT2>(*outVec), inVec);
    }
#endif // _WIN32

    inline bool StoreXrPose(XrPosef* out, const glm::mat4 &matrix) {
        glm::vec3 scale;
        glm::quat orientation;
        glm::vec3 position;
        glm::vec3 skew;
        glm::vec4 perspective;

        if (!glm::decompose(matrix, scale, orientation, position, skew, perspective)) {
            return false; // Non-SRT matrix encountered
        }

        StoreXrQuaternion(&out->orientation, orientation);
        StoreXrVector3(&out->position, position);
        return true;
    }

    namespace Pose {
        constexpr XrPosef Identity() {
            return {{0, 0, 0, 1}, {0, 0, 0}};
        }

        constexpr XrPosef Translation(const XrVector3f& translation) {
            XrPosef pose = Identity();
            pose.position = translation;
            return pose;
        }

#ifdef _WIN32
        inline XrPosef LookAt(const XrVector3f& origin, const XrVector3f& forward, const XrVector3f& up) {
            DirectX::XMMATRIX virtualToGazeOrientation =
                DirectX::XMMatrixLookToRH(xr::math::LoadXrVector3(origin), xr::math::LoadXrVector3(forward), xr::math::LoadXrVector3(up));
            XrPosef pose;
            xr::math::StoreXrPose(&pose, DirectX::XMMatrixInverse(nullptr, virtualToGazeOrientation));

            return pose;
        }
#endif

        inline XrPosef Slerp(const XrPosef& a, const XrPosef& b, float alpha) {
            return MakePose(Quaternion::Slerp(a.orientation, b.orientation, alpha), a.position + (b.position - a.position) * alpha);
        }

#ifdef _WIN32
        inline XrPosef Invert(const XrPosef& pose) {
            const DirectX::XMVECTOR orientation = LoadXrQuaternion(pose.orientation);
            const DirectX::XMVECTOR invertOrientation = DirectX::XMQuaternionConjugate(orientation);

            const DirectX::XMVECTOR position = LoadXrVector3(pose.position);
            const DirectX::XMVECTOR invertPosition = DirectX::XMVector3Rotate(DirectX::XMVectorNegate(position), invertOrientation);

            XrPosef result;
            StoreXrQuaternion(&result.orientation, invertOrientation);
            StoreXrVector3(&result.position, invertPosition);
            return result;
        }

        inline XrPosef Multiply(const XrPosef& a, const XrPosef& b) {
            // Q: Quaternion, P: Position, R:Rotation, T:Translation
            //   (Qa Pa) * (Qb Pb)
            //   = Ra * Ta * Rb * Tb
            //   = Ra * (Ta * Rb) * Tb
            //   = Ra * RotationOf(Ta * Rb) * TranslationOf(Ta * Rb) * Tb
            // => Rc = Ra * RotationOf(Ta * Rb)
            //    Qc = Qa * Qb;
            // => Tc = TranslationOf(Ta * Rb) * Tb
            //    Pc = XMVector3Rotate(Pa, Qb) + Pb;

            const DirectX::XMVECTOR pa = LoadXrVector3(a.position);
            const DirectX::XMVECTOR qa = LoadXrQuaternion(a.orientation);
            const DirectX::XMVECTOR pb = LoadXrVector3(b.position);
            const DirectX::XMVECTOR qb = LoadXrQuaternion(b.orientation);

            XrPosef c;
            StoreXrQuaternion(&c.orientation, DirectX::XMQuaternionMultiply(qa, qb));
            StoreXrVector3(&c.position, DirectX::XMVectorAdd(DirectX::XMVector3Rotate(pa, qb), pb));
            return c;
        }
#endif

        constexpr bool IsPoseValid(XrSpaceLocationFlags locationFlags) {
            constexpr XrSpaceLocationFlags PoseValidFlags = XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
            return (locationFlags & PoseValidFlags) == PoseValidFlags;
        }

        constexpr bool IsPoseTracked(XrSpaceLocationFlags locationFlags) {
            constexpr XrSpaceLocationFlags PoseTrackedFlags =
                XR_SPACE_LOCATION_POSITION_TRACKED_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
            return (locationFlags & PoseTrackedFlags) == PoseTrackedFlags;
        }

        constexpr bool IsPoseValid(const XrSpaceLocation& spaceLocation) {
            return IsPoseValid(spaceLocation.locationFlags);
        }

        constexpr bool IsPoseTracked(const XrSpaceLocation& spaceLocation) {
            return IsPoseTracked(spaceLocation.locationFlags);
        }

        constexpr bool IsPoseValid(const XrHandJointLocationEXT& jointLocation) {
            return IsPoseValid(jointLocation.locationFlags);
        }

        constexpr bool IsPoseTracked(const XrHandJointLocationEXT& jointLocation) {
            return IsPoseTracked(jointLocation.locationFlags);
        }

        constexpr bool IsPoseValid(const XrViewState& viewState) {
            constexpr XrViewStateFlags PoseValidFlags = XR_VIEW_STATE_POSITION_VALID_BIT | XR_VIEW_STATE_ORIENTATION_VALID_BIT;
            return (viewState.viewStateFlags & PoseValidFlags) == PoseValidFlags;
        }

        constexpr bool IsPoseTracked(const XrViewState& viewState) {
            constexpr XrViewStateFlags PoseTrackedFlags = XR_VIEW_STATE_POSITION_TRACKED_BIT | XR_VIEW_STATE_ORIENTATION_TRACKED_BIT;
            return (viewState.viewStateFlags & PoseTrackedFlags) == PoseTrackedFlags;
        }

        template <typename Quaternion, typename Vector3>
        inline XrPosef MakePose(const Quaternion& orientation, const Vector3& position) {
            return XrPosef{{orientation.x, orientation.y, orientation.z, orientation.w}, {position.x, position.y, position.z}};
        }
    } // namespace Pose

#ifdef _WIN32
    namespace Quaternion {
        constexpr inline XrQuaternionf Identity() {
            return {0, 0, 0, 1};
        }

        inline float Length(const XrQuaternionf& quaternion) {
            DirectX::XMVECTOR vector = LoadXrQuaternion(quaternion);
            return DirectX::XMVectorGetX(DirectX::XMVector4Length(vector));
        }

        inline bool IsNormalized(const XrQuaternionf& quaternion) {
            return fabs(1 - Length(quaternion)) <= QuaternionEpsilon;
        }

        inline XrQuaternionf RotationAxisAngle(const XrVector3f& axis, float angleInRadians) {
            XrQuaternionf q;
            StoreXrQuaternion(&q, DirectX::XMQuaternionRotationAxis(LoadXrVector3(axis), angleInRadians));
            return q;
        }

        inline XrQuaternionf RotationRollPitchYaw(const XrVector3f& anglesInRadians) {
            XrQuaternionf q;
            StoreXrQuaternion(&q, DirectX::XMQuaternionRotationRollPitchYaw(anglesInRadians.x, anglesInRadians.y, anglesInRadians.z));
            return q;
        }

        inline XrQuaternionf Slerp(const XrQuaternionf& a, const XrQuaternionf& b, float alpha) {
            DirectX::XMVECTOR qa = LoadXrQuaternion(a);
            DirectX::XMVECTOR qb = LoadXrQuaternion(b);
            DirectX::XMVECTOR qr = DirectX::XMQuaternionSlerp(qa, qb, alpha);
            XrQuaternionf result;
            StoreXrQuaternion(&result, qr);
            return result;
        }

    } // namespace Quaternion
#endif // _WIN32

    inline XrPosef operator*(const XrPosef& a, const XrPosef& b) {
        return Pose::Multiply(a, b);
    }

    inline float Dot(const XrVector3f& a, const XrVector3f& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline float Length(const XrVector3f& v) {
        return std::sqrt(Dot(v, v));
    }

    inline XrVector3f Normalize(const XrVector3f& a) {
        return a / std::sqrt(Dot(a, a));
    }

    // 2 * n / (r - l)    0                  0                    0
    // 0                  2 * n / (t - b)    0                    0
    // (r + l) / (r - l)  (t + b) / (t - b)  f / (n - f)         -1
    // 0                  0                  n*f / (n - f)        0
    inline glm::mat4 ComposeProjectionMatrix(const XrFovf& fov, const NearFar& nearFar) {
        const float tanAngleLeft = tanf(fov.angleLeft);
        const float tanAngleRight = tanf(fov.angleRight);

        const float tanAngleDown = tanf(fov.angleDown);
        const float tanAngleUp = tanf(fov.angleUp);

        const float tanAngleWidth = tanAngleRight - tanAngleLeft;

        // Set to tanAngleDown - tanAngleUp for a clip space with positive Y
        // down (Vulkan). Set to tanAngleUp - tanAngleDown for a clip space with
        // positive Y up (OpenGL / D3D / Metal).
        const float tanAngleHeight = tanAngleUp - tanAngleDown;

        // Set to nearFar.Near for a [-1,1] Z clip space (OpenGL / OpenGL ES).
        // Set to zero for a [0,1] Z clip space (Vulkan / D3D / Metal).
        const float offsetZ = nearFar.Near;

        glm::mat4 result;
        // FIXME isinf(nearFar.Far)?
        if (nearFar.Far <= nearFar.Near)
        {
            // place the far plane at infinity
            result[0][0] = 2 / tanAngleWidth;
            result[1][0] = 0;
            result[2][0] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
            result[3][0] = 0;

            result[0][1] = 0;
            result[1][1] = 2 / tanAngleHeight;
            result[2][1] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
            result[3][1] = 0;

            result[0][2] = 0;
            result[1][2] = 0;
            result[2][2] = -1;
            result[3][2] = -(nearFar.Near + offsetZ);

            result[0][3] = 0;
            result[1][3] = 0;
            result[2][3] = -1;
            result[3][3] = 0;
        } else {
            // normal projection
            result[0][0] = 2 / tanAngleWidth;
            result[1][0] = 0;
            result[2][0] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
            result[3][0] = 0;

            result[0][1] = 0;
            result[1][1] = 2 / tanAngleHeight;
            result[2][1] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
            result[3][1] = 0;

            result[0][2] = 0;
            result[1][2] = 0;
            result[2][2] = -(nearFar.Far + offsetZ) / (nearFar.Far - nearFar.Near);
            result[3][2] = -(nearFar.Far * (nearFar.Near + offsetZ)) / (nearFar.Far - nearFar.Near);

            result[0][3] = 0;
            result[1][3] = 0;
            result[2][3] = -1;
            result[3][3] = 0;
        }

        return result;
    }

    inline bool IsInfiniteNearPlaneProjectionMatrix(const glm::mat4& p) {
        return (p[2][2] == 0);
    }

    inline bool IsInfiniteFarPlaneProjectionMatrix(const glm::mat4& p) {
        return (p[2][2] == -1);
    }

    inline void ValidateProjectionMatrix(const glm::mat4& p) {
        // Reference equations on top of ComposeProjectionMatrix() above.
        if (p[0][1] != 0 || p[0][2] != 0 || p[0][3] != 0 ||
            // p[1][0] is not 0 on old MR devices, but small enough to be ignored. For future MR devices, it should be 0 (no shear)
            p[1][2] != 0 || p[1][3] != 0 ||
            // When near or far plane is infinite, p[2][2] is 0 or -1, respectively. They are valid cases.
            p[2][3] != -1 || p[3][0] != 0 || p[3][1] != 0 || p[3][3] != 0) {
            throw std::runtime_error("Invalid projection matrix");
        }
    }

#if 0
    inline NearFar GetProjectionNearFar(const glm::mat4& p) {
        ValidateProjectionMatrix(p);

        NearFar d;

        if (IsInfiniteNearPlaneProjectionMatrix(p)) {
            d.Near = std::numeric_limits<float>::infinity();
            d.Far = p[3][2]/2;
        } else if (IsInfiniteFarPlaneProjectionMatrix(p)) {
            d.Near = -p[3][2]/2;
            d.Far = std::numeric_limits<float>::infinity();
        } else {
            //p[2][2] = -(Far + Near) / (Far - Near);
            //p[3][2] = -Far * 2Near / (Far - Near);
            //p[3][2] / p[2][2] = -(Far * 2Near) / -(Far + Near)
            //                  =  Far * 2Near / (Far + Near)
            //                  =  
            //
            // Reference equations on top of ComposeProjectionMatrix() above.
            // FIXME
            d.Near = p[3][2] / p[2][2];
            d.Far = p[2][3] / (1 + p[2][2]);
        }

        return d;
    }
#endif

    inline XrFovf DecomposeProjectionMatrix(const glm::mat4& p) {
        ValidateProjectionMatrix(p);

        // n = m43 / m33
        // f = m43 / (1 + m33)
        // l = n * (m31 - 1) / m11  => angle left = atan2(l, n) => atan2(m31 - 1, m11)
        // r = n * (m31 + 1) / m11  => so on
        // b = n * (m32 - 1) / m22  => and
        // t = n * (m32 + 1) / m22  => so forth
        XrFovf fov;
        // FIXME correct?
        fov.angleLeft = atan2(p[2][0] - 1, p[0][0]);
        fov.angleRight = atan2(p[2][0] + 1, p[0][0]);
        fov.angleDown = atan2(p[2][1] - 1, p[1][1]);
        fov.angleUp = atan2(p[2][1] + 1, p[1][1]);
        return fov;
    }

    template <uint32_t alignment>
    inline constexpr uint32_t AlignTo(uint32_t n) {
        static_assert((alignment & (alignment - 1)) == 0); // must be power-of-two
        return (n + alignment - 1) & ~(alignment - 1);
    }

    inline constexpr uint32_t DivideRoundingUp(uint32_t x, uint32_t y) {
        return (x + y - 1) / y;
    }
} // namespace xr::math

#pragma endregion
