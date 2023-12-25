/**
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
#include "XrRuntime/XrRuntimeWrapper.h"

#include "xr-runtime-common/xr_windows.h"
#include "xr_api_wrapper.h"
#include "xr_api_wrapper_utility.h"
#include "load_library_utility.h"

#include <stdexcept>

namespace xr_display
{
#define RETURN_IF(func)                      \
	{                                        \
		SonyOzResult Result = (func);        \
		if (Result != SonyOzResult::SUCCESS) \
		{                                    \
			return Result;                   \
		}                                    \
    }

	FXrRuntimeWrapper::FXrRuntimeWrapper()
	{
		TargetMonitorRectangle = SonyOzRect{ 0, 0, 0, 0 };
	}

	FXrRuntimeWrapper::~FXrRuntimeWrapper() {}

	SonyOzResult FXrRuntimeWrapper::InitializeRuntime(SonyOzPlatformId PlatformId)
	{
		SonyOzResult ret = sony::oz::xr_runtime::LinkXrLibrary(PlatformId);
		if (ret == SonyOzResult::SUCCESS) {
			ret = LinkSrdLibrary();
		}
		return ret;
	}

	void FXrRuntimeWrapper::FinalizeRuntime()
	{
		sony::oz::xr_runtime::UnlinkXrLibrary();
	}

	SonyOzResult FXrRuntimeWrapper::LinkSrdLibrary(void)
	{
		PlatformAPI.GetPerformancePriorityEnabled = (GetPerformancePriorityEnabled_t)GetProcAddress(sony::oz::xr_runtime::utility::GetXrRuntimeDLL(), "GetPerformancePriorityEnabled");
		PlatformAPI.GetLensShiftEnabled = (GetLensShiftEnabled_t)GetProcAddress(sony::oz::xr_runtime::utility::GetXrRuntimeDLL(), "GetLensShiftEnabled");
		PlatformAPI.SetLensShiftEnabled = (SetLensShiftEnabled_t)GetProcAddress(sony::oz::xr_runtime::utility::GetXrRuntimeDLL(), "SetLensShiftEnabled");

		return SonyOzResult::SUCCESS;
	}

	SonyOzResult FXrRuntimeWrapper::GetDeviceNum(SonyOzPlatformId PlatformId, uint64_t* Num)
	{
		return sony::oz::xr_runtime::GetDeviceNum(PlatformId, Num);
	}

	SonyOzResult FXrRuntimeWrapper::EnumerateDevices(SonyOzPlatformId PlatformId, uint64_t Size,
		SonyOzDeviceInfo* DeviceList)
	{
		return sony::oz::xr_runtime::EnumerateDevices(PlatformId, Size, DeviceList);
	}

	SonyOzResult FXrRuntimeWrapper::InitializeSession(SonyOzPlatformId PlatformId, SonyOzDeviceInfo* DeviceInfo,
		SonyOzLogSettings_LogCallback callback)
	{
		RETURN_IF(sony::oz::xr_runtime::SetDebugLogCallback(PlatformId, callback));

		RUNTIME_OPTION_BIT_FLAG runtime_option_bit_flag = RUNTIME_OPTION_IS_XR_CONTENT;
		PLATFORM_OPTION_BIT_FLAG platform_option_bit_flag = PLATFORM_OPTION_APPLICATION_IS_UNREALENGINE;
		RETURN_IF(sony::oz::xr_runtime::CreateSession(PlatformId, DeviceInfo, runtime_option_bit_flag, platform_option_bit_flag, &Handle));
		RETURN_IF(sony::oz::xr_runtime::BeginSession(Handle));

		if (sony::oz::xr_runtime::utility::WaitUntilRunningState(Handle) == false)
		{
			return SonyOzResult::ERROR_SESSION_NOT_RUNNING;
		}

		RETURN_IF(sony::oz::xr_runtime::EnableStereo(Handle, true));
		RETURN_IF(sony::oz::xr_runtime::UpdateTrackingResultCache(Handle));

		SonyOzRect Rect;
		RETURN_IF(sony::oz::xr_runtime::GetTargetMonitorRectangle(Handle, &Rect));
		TargetMonitorRectangle.top = Rect.top;
		TargetMonitorRectangle.bottom = Rect.bottom;
		TargetMonitorRectangle.left = Rect.left;
		TargetMonitorRectangle.right = Rect.right;

		return SonyOzResult::SUCCESS;
	}

	void FXrRuntimeWrapper::FinalizeSession()
	{
		sony::oz::xr_runtime::EnableStereo(Handle, false);
		sony::oz::xr_runtime::EndSession(Handle);
		sony::oz::xr_runtime::DestroySession(&Handle);
	}

	namespace
	{
		// ˆÈ‰º„‘ª
		// R = Right
		// Y = XYZÀ•W‚ÌY
		// u = up
		// Z = XYZÀ•W‚ÌZ
		// f = front
		SonyOzPosef Convert_RYuZf_To_LZuXf(const SonyOzPosef& from)
		{
			SonyOzPosef To;
			To.position.x = from.position.z;
			To.position.y = -from.position.x;
			To.position.z = from.position.y;

			To.orientation.x = -from.orientation.z;
			To.orientation.y = from.orientation.x;
			To.orientation.z = -from.orientation.y;
			To.orientation.w = from.orientation.w;
			return To;
		}

		// 1m == 100cm
		float ConvertCentiMeterFromMeter(float from)
		{
			return from * 100.f;
		}

		SonyOzVector3f ConvertCentiMeterFromMeter(const SonyOzVector3f& from)
		{
			SonyOzVector3f To;
			To.x = ConvertCentiMeterFromMeter(from.x);
			To.y = ConvertCentiMeterFromMeter(from.y);
			To.z = ConvertCentiMeterFromMeter(from.z);
			return To;
		}

	}	 // namespace

	SonyOzResult FXrRuntimeWrapper::UpdatePoses()
	{
		SonyOzResult Result;
		Result = sony::oz::xr_runtime::UpdateTrackingResultCache(Handle);
		if (Result != SonyOzResult::SUCCESS)
		{
			return Result;
		}

		// SRDisplayDef.h
		// PoseId::HEAD = 0;
		const auto GetProjection = [this](SonyOzPoseId pose_id)
		{
			SonyOzProjection Projection;
			sony::oz::xr_runtime::GetProjection(Handle, pose_id, &Projection);
			return Projection;
		};

		const auto ConvertPoseToUnrealEngine = [this](SonyOzPosef target) -> SonyOzPosef
		{
			auto Pose = Convert_RYuZf_To_LZuXf(target);
			Pose.position = ConvertCentiMeterFromMeter(Pose.position);
			return Pose;
		};

		bool is_valid = false;
		{
			const SonyOzPoseId head_id = SonyOzPoseId::HEAD;
			Result = sony::oz::xr_runtime::GetCachedPose(Handle, head_id, &CachedHeadPose.Pose, &is_valid);
			if (Result != SonyOzResult::SUCCESS)
			{
				return Result;
			}
			CachedHeadPose.HalfAngles = GetProjection(head_id);
			UePoses.Head = ConvertPoseToUnrealEngine(CachedHeadPose.Pose);
		}
		{
			const SonyOzPoseId left_eye_id = SonyOzPoseId::LEFT_EYE;
			Result = sony::oz::xr_runtime::GetCachedPose(Handle, left_eye_id, &CachedLeftEyePose.Pose, &is_valid);
			if (Result != SonyOzResult::SUCCESS)
			{
				return Result;
			}
			CachedLeftEyePose.HalfAngles = GetProjection(left_eye_id);
			UePoses.LeftEye = ConvertPoseToUnrealEngine(CachedLeftEyePose.Pose);
		}
		const SonyOzPoseId right_eye_id = SonyOzPoseId::RIGHT_EYE;
		{
			Result = sony::oz::xr_runtime::GetCachedPose(Handle, right_eye_id, &CachedRightEyePose.Pose, &is_valid);
			if (Result != SonyOzResult::SUCCESS)
			{
				return Result;
			}
			CachedRightEyePose.HalfAngles = GetProjection(right_eye_id);
			UePoses.RightEye = ConvertPoseToUnrealEngine(CachedRightEyePose.Pose);
		}

		return Result;
	}

	void FXrRuntimeWrapper::SetColorSpace(int InputGammaCount, int OutputGammaCount, float Gamma) const
	{
		sony::oz::xr_runtime::SetColorSpace(Handle, InputGammaCount, OutputGammaCount, Gamma);
	}

	SonyOzResult FXrRuntimeWrapper::SubmitD3d11(ID3D11Device* Device, ID3D11Texture2D* SideBySide, ID3D11Texture2D* Target) const
	{
		return sony::oz::xr_runtime::SubmitD3d11(Handle, Device, SideBySide, false, Target);
	}

	SonyOzResult FXrRuntimeWrapper::SubmitD3d12(ID3D12CommandQueue* CommandQueue, uint32_t NodeMask, ID3D12Resource* SideBySide,
		DXGI_FORMAT Format, ID3D12Resource* Target) const
	{
		return sony::oz::xr_runtime::SubmitD3d12(Handle, CommandQueue, NodeMask, SideBySide, false, Format, Target);
	}

	const SonyOzPosef& FXrRuntimeWrapper::GetCachedPose(SonyOzPoseId PoseId) const
	{
		switch (PoseId)
		{
		case SonyOzPoseId::HEAD:
			return UePoses.Head;
		case SonyOzPoseId::LEFT_EYE:
			return UePoses.LeftEye;
		case SonyOzPoseId::RIGHT_EYE:
			return UePoses.RightEye;
		default:
			throw std::runtime_error("Unexpected argument.");
		}
	}

	auto FXrRuntimeWrapper::GetHalfAngles(SonyOzPoseId PoseId) const -> const SonyOzProjection&
	{
		switch (PoseId)
		{
		case SonyOzPoseId::LEFT_EYE:
			return CachedLeftEyePose.HalfAngles;
		case SonyOzPoseId::RIGHT_EYE:
			return CachedRightEyePose.HalfAngles;
		default:
			throw std::runtime_error("Unexpected argument.");
		}
	}

	auto FXrRuntimeWrapper::GetTargetMonitorRectangle() const -> const SonyOzRect&
	{
		return TargetMonitorRectangle;
	}

	SonyOzResult FXrRuntimeWrapper::GetState(SonyOzSessionState* State) const
	{
		return sony::oz::xr_runtime::GetSessionState(Handle, State);
	}

	SonyOzResult FXrRuntimeWrapper::GetDisplaySpec(SonyOzDisplaySpec* DisplaySpec) const
	{
		return sony::oz::xr_runtime::GetDisplaySpec(Handle, DisplaySpec);
	}

	SonyOzXrSystemError FXrRuntimeWrapper::GetXrSystemError() const
	{
		SonyOzXrSystemError error;
		sony::oz::xr_runtime::GetXrSystemError(Handle, &error);
		return error;
	}

	SonyOzSessionHandle FXrRuntimeWrapper::GetSessionHandle()
	{
		return Handle;
	}

	SonyOzResult FXrRuntimeWrapper::GetXrSystemError(SonyOzXrSystemError* Error)
	{
		return sony::oz::xr_runtime::GetXrSystemError(Handle, Error);
	}

	SonyOzResult FXrRuntimeWrapper::GetXrSystemErrorNum(uint16_t* num)
	{
		return sony::oz::xr_runtime::GetXrSystemErrorNum(Handle, num);
	}

	SonyOzResult FXrRuntimeWrapper::GetXrSystemErrorList(uint16_t num, SonyOzXrSystemError* Errors)
	{
		return sony::oz::xr_runtime::GetXrSystemErrorList(Handle, num, Errors);
	}

	SonyOzResult FXrRuntimeWrapper::EnableStereo(bool enable)
	{
		return sony::oz::xr_runtime::EnableStereo(Handle, enable);
	}
	SonyOzResult FXrRuntimeWrapper::GetPerformancePriorityEnabled(bool* enable)
	{
		if (!PlatformAPI.GetPerformancePriorityEnabled)
			return SonyOzResult::ERROR_FUNCTION_UNSUPPORTED;
	
		return PlatformAPI.GetPerformancePriorityEnabled(Handle, enable);
	}
	SonyOzResult FXrRuntimeWrapper::GetLensShiftEnabled(bool* enable)
	{
		if (!PlatformAPI.GetLensShiftEnabled)
			return SonyOzResult::ERROR_FUNCTION_UNSUPPORTED;

		return PlatformAPI.GetLensShiftEnabled(Handle, enable);
	}
	SonyOzResult FXrRuntimeWrapper::SetLensShiftEnabled(bool enable)
	{
		if (!PlatformAPI.SetLensShiftEnabled)
			return SonyOzResult::ERROR_FUNCTION_UNSUPPORTED;

		return PlatformAPI.SetLensShiftEnabled(Handle, enable);
	}

}	 // namespace xr_display
