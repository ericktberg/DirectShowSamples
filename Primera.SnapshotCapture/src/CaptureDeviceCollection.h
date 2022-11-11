#pragma once
#include <VideoCaptureDevice.h>
#include <array>
#include "MaybeMessage.h"

MaybeMessage<std::array<VideoCaptureDevice, 10>> acquireCaptureDevices();


