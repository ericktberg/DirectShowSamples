#pragma once

#include <winnt.h>
#include <mfidl.h>

// The class that implements the callback interface.
class SampleGrabberCB : public IMFSampleGrabberSinkCallback
{
    long m_cRef;

    SampleGrabberCB() : m_cRef(1) {}

public:
    static HRESULT CreateInstance(SampleGrabberCB** ppCB);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFClockStateSink methods
    STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
    STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);

    // IMFSampleGrabberSinkCallback methods
    STDMETHODIMP OnSetPresentationClock(IMFPresentationClock* pClock);
    STDMETHODIMP OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
                                 LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE* pSampleBuffer,
                                 DWORD dwSampleSize);
    STDMETHODIMP OnShutdown();
};