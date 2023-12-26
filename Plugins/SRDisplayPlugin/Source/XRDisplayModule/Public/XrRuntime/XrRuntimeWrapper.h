/**
 * @file xr_api_unrealengine_wrapper.h
 * @copyright
 *
 * Sony CONFIDENTIAL
 *
 * Copyright 2022 Sony Corporation
 *
 * DO NOT COPY AND/OR REDISTRIBUTE WITHOUT PERMISSION.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#pragma once

#include "xr-runtime-common/xr_api_defs.h"
#include "xr-runtime-common/xr_platform_option.h"
#include "log-settings/log_defs.h"

XR_DEFINE_HANDLE(SonyOzSessionHandle);

/**
 * @namespace xr_display
 * @brief API
 */
namespace xr_display
{
	using GetPerformancePriorityEnabled_t = SonyOzResult(*) (SonyOzSessionHandle, bool*);
	using GetLensShiftEnabled_t = SonyOzResult(*) (SonyOzSessionHandle, bool*);
	using SetLensShiftEnabled_t = SonyOzResult(*) (SonyOzSessionHandle, bool);

	class FXrRuntimeWrapper
	{
	public:
		FXrRuntimeWrapper();
		~FXrRuntimeWrapper();

		SonyOzResult InitializeRuntime(SonyOzPlatformId PlatformId);
		void FinalizeRuntime();
		SonyOzResult LinkSrdLibrary(void);

		SonyOzResult GetDeviceNum(SonyOzPlatformId PlatformId, uint64_t* Num);
		SonyOzResult EnumerateDevices(SonyOzPlatformId PlatformId, uint64_t Size,
			SonyOzDeviceInfo* DeviceList);

		SonyOzResult InitializeSession(SonyOzPlatformId PlatformId, SonyOzDeviceInfo* DeviceInfo, SonyOzLogSettings_LogCallback callback);
		void FinalizeSession();

		const SonyOzPosef& GetCachedPose(SonyOzPoseId PoseId) const;
		bool IsDetectedFace() const;

		SonyOzResult UpdatePoses();

		void SetColorSpace(int InputGammaCount, int OutputGammaCount, float Gamma) const;
		SonyOzResult SubmitD3d11(ID3D11Device* Device, ID3D11Texture2D* SideBySide, ID3D11Texture2D* Target) const;
		SonyOzResult SubmitD3d12(ID3D12CommandQueue* CommandQueue, uint32_t NodeMask, ID3D12Resource* SideBySide,
			DXGI_FORMAT Format, ID3D12Resource* Target) const;

		const SonyOzProjection& GetHalfAngles(SonyOzPoseId PoseId) const;

		const SonyOzRect& GetTargetMonitorRectangle() const;

		SonyOzResult GetState(SonyOzSessionState* State) const;

		SonyOzResult GetDisplaySpec(SonyOzDisplaySpec* DisplaySpec) const;

		SonyOzXrSystemError GetXrSystemError() const;

		SonyOzSessionHandle GetSessionHandle();

		SonyOzResult GetXrSystemError(SonyOzXrSystemError* Error);
		SonyOzResult GetXrSystemErrorNum(uint16_t* num);
		SonyOzResult GetXrSystemErrorList(uint16_t num, SonyOzXrSystemError* Errors);

		SonyOzResult EnableStereo(bool enable);

		struct {
			GetPerformancePriorityEnabled_t GetPerformancePriorityEnabled;
			GetLensShiftEnabled_t GetLensShiftEnabled;
			SetLensShiftEnabled_t SetLensShiftEnabled;
		} PlatformAPI;
		SonyOzResult GetPerformancePriorityEnabled(bool* enable);
		SonyOzResult GetLensShiftEnabled(bool* enable);
		SonyOzResult SetLensShiftEnabled(bool enable);

	private:
		SonyOzSessionHandle Handle = nullptr;

		struct FCachedPose
		{
			SonyOzPosef Pose;
			SonyOzProjection HalfAngles;
		};
		FCachedPose CachedHeadPose;
		FCachedPose CachedLeftEyePose;
		FCachedPose CachedRightEyePose;

		struct FUnrealEnginePoseList
		{
			SonyOzPosef Head;
			SonyOzPosef LeftEye;
			SonyOzPosef RightEye;
		};
		FUnrealEnginePoseList UePoses;

		SonyOzRect TargetMonitorRectangle;
	};

}	 // namespace xr_display
