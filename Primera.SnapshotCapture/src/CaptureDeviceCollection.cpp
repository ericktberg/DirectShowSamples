#include <comdef.h>
#include <iomanip>
#include <sstream>
#include <strmif.h>
#include <uuids.h>

#include "CaptureDeviceCollection.h"

template <typename T>
MaybeMessage<T> maybeFailComMessage(std::string description, HRESULT errorCode) {
    std::stringstream builder;
    builder << ": " << std::hex << errorCode;

    return maybeFail<T>(description.append(builder.str()));
}

MaybeMessage<std::array<VideoCaptureDevice, 10>> acquireVideoDevices() {
    HRESULT result;

    ICreateDevEnum* createDeviceEnumerator = nullptr;
    result = CoCreateInstance(
        CLSID_SystemDeviceEnum,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum,
        (void**) &createDeviceEnumerator);

    if (result != NOERROR) {
        return maybeFailComMessage<std::array<VideoCaptureDevice, 10>>("Error creating device enumerator", result);
    }

    IEnumMoniker* videoEnumerator = nullptr;
    result = createDeviceEnumerator->CreateClassEnumerator(
        CLSID_VideoInputDeviceCategory,
        &videoEnumerator,
        0);

    if (result != NOERROR) {
        createDeviceEnumerator->Release();

        return maybeFailComMessage<std::array<VideoCaptureDevice, 10>>("Error enumerating video devices", result);
    }

    createDeviceEnumerator->Release();
    videoEnumerator->Release();

    return maybeFail<std::array<VideoCaptureDevice, 10>>("Not implemented");
}