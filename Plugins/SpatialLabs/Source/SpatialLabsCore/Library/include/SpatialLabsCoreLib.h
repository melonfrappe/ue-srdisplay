#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

namespace SpatialLabsCoreLib
{
    struct ProjectionInfo
    {
        int EyeIndex; // 0 for left, 1 for right
        float LeftEyePos[3]; //FrameState.Views[0].pose.position
        float RightEyePos[3];// FrameState.Views[1].pose.position
        float CameraOffset;
        bool UseDynamicCameraFOV;
        bool UseFixedHeadPosition;
        float FixedHeadPosition[3];
        float FocalLengthPlayerCamera;
        bool UseDynamicCameraFOVAroundSceenCenter;
        float ScaledWorldToMetersScale;
        float ZNear;
    };

    class SpatialLabsCoreLibAPI
    {
    public:
        static void CheckSpatialLabsDevice();

        static bool IsSpatialLabsDeviceAvailable();

        static int GetErrorCode();

        static void GetProjectionMatrix(ProjectionInfo info, float* ProjectionMatrix);

        static void GetRelativeEyePosition(ProjectionInfo info, float* RelativeEyePosition);

        static void GetCachedEyePosition(int EyeIndex, float* EyePosition);

        static void GetCachedEyePositionScreenSpace(int EyeIndex, float* EyePosition);
    };
}