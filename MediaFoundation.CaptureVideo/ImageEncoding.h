#pragma once

#include <wincodec.h>
#include <wincodecsdk.h>



int EncodeImage()
{
    // Initialize COM.
    HRESULT hr = S_OK;

    IWICImagingFactory* piFactory = NULL;
    IWICBitmapDecoder* piDecoder = NULL;

    // Create the COM imaging factory.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_WICImagingFactory,
                              NULL, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&piFactory));
    }

    // Create the decoder.
    if (SUCCEEDED(hr))
    {
        hr = piFactory->CreateDecoderFromFilename(L"C:\\Users\\eberg\\Pictures\\Camera Roll\\WIN_20221108_17_21_07_Pro.jpg", NULL, GENERIC_READ,
                                                  WICDecodeMetadataCacheOnDemand, //For JPEG lossless decoding/encoding.
                                                  &piDecoder);
    }

    // Variables used for encoding.
    IWICStream* piFileStream = NULL;
    IWICBitmapEncoder* piEncoder = NULL;
    IWICMetadataBlockWriter* piBlockWriter = NULL;
    IWICMetadataBlockReader* piBlockReader = NULL;

    WICPixelFormatGUID pixelFormat = { 0 };
    UINT count = 0;
    double dpiX = 0.0, dpiY = 0.0;
    UINT width = 0, height = 0;

    // Create a file stream.
    if (SUCCEEDED(hr))
    {
        hr = piFactory->CreateStream(&piFileStream);
    }

    // Initialize our new file stream.
    if (SUCCEEDED(hr))
    {
        hr = piFileStream->InitializeFromFilename(L"C:\\Users\\eberg\\Pictures\\Camera Roll\\test.png", GENERIC_WRITE);
    }

    // Create the encoder.
    if (SUCCEEDED(hr))
    {
        hr = piFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &piEncoder);
    }
    // Initialize the encoder
    if (SUCCEEDED(hr))
    {
        hr = piEncoder->Initialize(piFileStream, WICBitmapEncoderNoCache);
    }

    if (SUCCEEDED(hr))
    {
        hr = piDecoder->GetFrameCount(&count);
    }

    if (SUCCEEDED(hr))
    {
        // Process each frame of the image.
        for (UINT i = 0; i < count && SUCCEEDED(hr); i++)
        {
            // Frame variables.
            IWICBitmapFrameDecode* piFrameDecode = NULL;
            IWICBitmapFrameEncode* piFrameEncode = NULL;
            IWICMetadataQueryReader* piFrameQReader = NULL;
            IWICMetadataQueryWriter* piFrameQWriter = NULL;

            // Get and create the image frame.
            if (SUCCEEDED(hr))
            {
                hr = piDecoder->GetFrame(i, &piFrameDecode);
            }
            if (SUCCEEDED(hr))
            {
                hr = piEncoder->CreateNewFrame(&piFrameEncode, NULL);
            }

            // Initialize the encoder.
            if (SUCCEEDED(hr))
            {
                hr = piFrameEncode->Initialize(NULL);
            }
            // Get and set the size.
            if (SUCCEEDED(hr))
            {
                hr = piFrameDecode->GetSize(&width, &height);
            }

            if (SUCCEEDED(hr))
            {
                hr = piFrameEncode->SetSize(width, height);
            }
            // Get and set the resolution.
            if (SUCCEEDED(hr))
            {
                piFrameDecode->GetResolution(&dpiX, &dpiY);
            }
            if (SUCCEEDED(hr))
            {
                if (dpiX && dpiY) {
                    hr = piFrameEncode->SetResolution(dpiX, dpiY);
                }
            }
            // Set the pixel format.
            if (SUCCEEDED(hr))
            {
                piFrameDecode->GetPixelFormat(&pixelFormat);
            }
            if (SUCCEEDED(hr))
            {
                hr = piFrameEncode->SetPixelFormat(&pixelFormat);
            }

            // Check that the destination format and source formats are the same.
            bool formatsEqual = FALSE;
            if (SUCCEEDED(hr))
            {
                GUID srcFormat;
                GUID destFormat;

                hr = piDecoder->GetContainerFormat(&srcFormat);
                if (SUCCEEDED(hr))
                {
                    hr = piEncoder->GetContainerFormat(&destFormat);
                }
                if (SUCCEEDED(hr))
                {
                    if (srcFormat == destFormat)
                        formatsEqual = true;
                    else
                        formatsEqual = false;
                }
            }

            if (SUCCEEDED(hr) && formatsEqual)
            {
                // Copy metadata using metadata block reader/writer.
                if (SUCCEEDED(hr))
                {
                    piFrameDecode->QueryInterface(IID_PPV_ARGS(&piBlockReader));
                }
                if (SUCCEEDED(hr))
                {
                    piFrameEncode->QueryInterface(IID_PPV_ARGS(&piBlockWriter));
                }
                if (SUCCEEDED(hr))
                {
                    piBlockWriter->InitializeFromBlockReader(piBlockReader);
                }

                if (SUCCEEDED(hr))
                {
                    hr = piFrameEncode->GetMetadataQueryWriter(&piFrameQWriter);
                }

                if (SUCCEEDED(hr))
                {
                    // Add additional metadata.
                    PROPVARIANT    value;
                    value.vt = VT_LPWSTR;
                    value.pwszVal = L"Metadata Test Image.";
                    hr = piFrameQWriter->SetMetadataByName(L"/xmp/dc:title", &value);
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = piFrameEncode->WriteSource(
                    static_cast<IWICBitmapSource*> (piFrameDecode),
                    NULL); // Using NULL enables JPEG loss-less encoding.
            }

            // Commit the frame.
            if (SUCCEEDED(hr))
            {
                hr = piFrameEncode->Commit();
            }

            if (piFrameDecode)
            {
                piFrameDecode->Release();
            }

            if (piFrameEncode)
            {
                piFrameEncode->Release();
            }

            if (piFrameQReader)
            {
                piFrameQReader->Release();
            }

            if (piFrameQWriter)
            {
                piFrameQWriter->Release();
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        piEncoder->Commit();
    }

    if (SUCCEEDED(hr))
    {
        piFileStream->Commit(STGC_DEFAULT);
    }

    if (piFileStream)
    {
        piFileStream->Release();
    }
    if (piEncoder)
    {
        piEncoder->Release();
    }
    if (piBlockWriter)
    {
        piBlockWriter->Release();
    }
    if (piBlockReader)
    {
        piBlockReader->Release();
    }
    return 0;
}