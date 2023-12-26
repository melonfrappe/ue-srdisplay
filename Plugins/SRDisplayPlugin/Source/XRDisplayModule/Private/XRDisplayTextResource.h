/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

#pragma once

#include <string>

namespace xr_display
{
	const std::wstring SRD_ERROR_MESSAGE_C016 =
		L"Unknown error has occurred.";
	const std::wstring SRD_ERROR_MESSAGE_C017 =
		L"The application will be terminated.";
	const std::wstring SRD_ERROR_MESSAGE_C018 =
		L"Failed to detect Spatial Reality Display. Make sure Display cable is connected correctly between PC and Spatial Reality Display.";
	const std::wstring SRD_ERROR_MESSAGE_C019 =
		L"Failed to detect Spatial Reality Display. Make sure USB 3.0 cable is connected correctly between PC and Spatial Reality Display.";
	const std::wstring SRD_ERROR_MESSAGE_C019_2 =
		L"Failed to detect Spatial Reality Display. Make sure USB cable is connected correctly between PC and Spatial Reality Display, and Spatial Reality Display device is powered on.";
	const std::wstring SRD_ERROR_MESSAGE_C020 =
		L"Failed to find Spatial Reality Display device. Make sure Spatial Reality Display device is powered on.";
	const std::wstring SRD_ERROR_MESSAGE_C021 =
		L"USB connection has been interrupted. The USB cable may be disconnected or Spatial Reality Display device's power may be turned off.";
	const std::wstring SRD_ERROR_MESSAGE_C022 =
		L"Spatial Reality Display SDK is not found. Spatial Reality Display SDK may be not installed correctly. Try to re-install with Spatial Reality Display Settings installer.";
	const std::wstring SRD_ERROR_MESSAGE_C023 =
		L"Another Spatial Reality Display application is already running. Please close it and start this application again.";
	const std::wstring SRD_ERROR_MESSAGE_C030 =
		L"SR Display is not recognized correctly. "
		L"Please make sure SR Display and PC's USB 3.0 port are connected with USB3.0 cable. "
		L"Also, please try following steps.\n"
		L"\n"
		L"    1. Unplug USB cable from PC's USB 3.0 port.\n"
		L"    2. Turn SR Display's power off.\n"
		L"    3. Plug USB cable into PC's USB 3.0 port.\n"
		L"    4. Wait for 30 seconds.\n"
		L"    5. Turn SR Display's power on.\n"
		L"    6. Launch this application again.\n";

	const std::wstring SRD_ERROR_MESSAGE_C036 =
		L"The old version of the Spatial Reality Display SDK has been installed. "
		L"The Spatial Reality Display Settings Installer version 2.0 or later is required to run this application.";

	const std::wstring SRD_ERROR_MESSAGE_UE022 =
		L"SR Display Plugin does not support \"Mobile Preview ES3.1 (PIE)\".";
	const std::wstring SRD_ERROR_MESSAGE_UE023 =
		L"SR Display Plugin does not support graphics API other than D3D11. This project is using unsupported graphics API.";

} // namespace xr_display
