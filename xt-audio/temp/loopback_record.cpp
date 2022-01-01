#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <atlbase.h>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <fstream>

static const int HNS_PER_MS = 10000;
static const int BUFFER_SIZE_MS = 10;
static const int BUFFER_SIZE_HNS = BUFFER_SIZE_MS * HNS_PER_MS;

static const int CHANNELS = 2;
static const int RECORD_SECONDS = 5;
static const int SAMPLE_RATE = 48000;
static float RECORDING[SAMPLE_RATE * CHANNELS * RECORD_SECONDS];

#define VERIFY(hr) if(FAILED(hr)) { std::cout << "Error: " << hr << "\n"; goto error; }

int main(int argc, char** argv)
{
  DWORD flags;
  HANDLE event;
  BYTE* buffer;
  UINT32 locked;
  UINT32 padding;
  int recorded = 0;
  UINT64 devicePos;
  UINT64 counterPos;
  UINT32 renderBufferSize;
  UINT32 captureBufferSize;
  CComPtr<IMMDevice> device;
  CComPtr<IAudioClient> render;
  CComPtr<IAudioClient> capture;
  CComPtr<IMMDeviceEnumerator> enumerator;
  CComPtr<IAudioRenderClient> renderClient;
  CComPtr<IAudioCaptureClient> captureClient;
  std::ofstream out("out.raw", std::ios::out | std::ios::binary);

  event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  assert(event != nullptr);

  WAVEFORMATEXTENSIBLE wfx;
  wfx.dwChannelMask = 0;
  wfx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
  wfx.Samples.wValidBitsPerSample = sizeof(float) * 8;
  wfx.Format.nChannels = CHANNELS;
  wfx.Format.wBitsPerSample = sizeof(float) * 8;
  wfx.Format.nSamplesPerSec = SAMPLE_RATE;
  wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
  wfx.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);
  wfx.Format.nBlockAlign = wfx.Format.wBitsPerSample/8 * wfx.Format.nChannels;
  wfx.Format.nAvgBytesPerSec = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;

  VERIFY(CoInitializeEx(0, COINIT_MULTITHREADED));
  VERIFY(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
    __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator)));
  VERIFY(enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device));

  VERIFY(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 
    nullptr, reinterpret_cast<void**>(&render)));
  VERIFY(render->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
    BUFFER_SIZE_HNS, 0, reinterpret_cast<WAVEFORMATEX*>(&wfx), nullptr));
  VERIFY(render->SetEventHandle(event));
  VERIFY(render->GetBufferSize(&renderBufferSize));
  VERIFY(render->GetService(__uuidof(IAudioRenderClient),
    reinterpret_cast<void**>(&renderClient)));
  
  VERIFY(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
    nullptr, reinterpret_cast<void**>(&capture)));
  VERIFY(capture->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
    BUFFER_SIZE_HNS, 0, reinterpret_cast<WAVEFORMATEX*>(&wfx), nullptr));
  VERIFY(capture->GetBufferSize(&captureBufferSize));
  VERIFY(capture->GetService(__uuidof(IAudioCaptureClient),
    reinterpret_cast<void**>(&captureClient)));

  VERIFY(render->Start());
  VERIFY(capture->Start());

  while(recorded < RECORD_SECONDS * SAMPLE_RATE)
  {
    VERIFY(render->GetCurrentPadding(&padding));
    locked = renderBufferSize - padding;
    VERIFY(renderClient->GetBuffer(locked, &buffer));
    memset(buffer, 0, locked * CHANNELS * sizeof(float));
    VERIFY(renderClient->ReleaseBuffer(locked, 0));

    VERIFY(captureClient->GetBuffer(&buffer, &locked, &flags, &devicePos, &counterPos));
    for(UINT32 i = 0; i < locked; i++)
    {
      RECORDING[recorded * 2] = reinterpret_cast<float*>(buffer)[i * 2];
      RECORDING[recorded * 2 + 1] = reinterpret_cast<float*>(buffer)[i * 2 + 1];
      recorded++;
    }
    VERIFY(captureClient->ReleaseBuffer(locked));
  }

  VERIFY(capture->Stop());
  VERIFY(render->Stop());

  out.write(reinterpret_cast<char*>(RECORDING), sizeof(RECORDING));
  return 0;
error:
  CloseHandle(event);
  CoUninitialize();
  return 1;
}