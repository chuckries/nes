#include "pch.h"
#include "NesRuntimeComponent.h"
#include "Rom.h"
#include "XA2AudioProvider.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

namespace NesRuntimeComponent
{
    StandardController::StandardController(::IStandardController* controller)
        :_controller(controller)
    {
    }

    void StandardController::A(bool state)
    {
        _controller->A(state);
    }

    void StandardController::B(bool state)
    {
        _controller->B(state);
    }

    void StandardController::Select(bool state)
    {
        _controller->Select(state);
    }

    void StandardController::Start(bool state)
    {
        _controller->Start(state);
    }

    void StandardController::Up(bool state)
    {
        _controller->Up(state);
    }

    void StandardController::Down(bool state)
    {
        _controller->Down(state);
    }

    void StandardController::Left(bool state)
    {
        _controller->Left(state);
    }

    void StandardController::Right(bool state)
    {
        _controller->Right(state);
    }

    Nes::Nes(::Nes* nes)
        : _nes(nes)
    {
    }

    Nes::~Nes()
    {
    }

    IAsyncOperation<Nes^>^ Nes::Create(RomFile^ romFile)
    {
        return create_async([=]() {
            NPtr<XA2AudioProvider> audioProvider;
            audioProvider.Attach(new XA2AudioProvider(44100));
            NPtr<::Nes> nes;
            ::Nes::Create(static_cast<IRomFile*>(romFile->Native), static_cast<IAudioProvider*>(audioProvider), &nes);
            return ref new Nes(nes);
        });
    }

    void Nes::DoFrame(WriteOnlyArray<byte>^ screen)
    {
        _nes->DoFrame(screen->Data);
    }

    StandardController^ Nes::GetStandardController(unsigned int port)
    {
        ::IStandardController* controller = _nes->GetStandardController(port);
        return ref new StandardController(controller);
    }
}
