#pragma once

#include "VideoCaptureDevice.h"
#include "MaybeMessage.h"


class VideoDeviceCollection {
public:
    VideoDeviceCollection(VideoCaptureDevice** devices, int length);
    ~VideoDeviceCollection();

    VideoCaptureDevice** allDevices();
    int length();

private:
    VideoCaptureDevice** _devices;
    int _length;
};


MaybeMessage<VideoDeviceCollection> acquireVideoDevices();