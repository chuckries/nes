#pragma once

namespace NesRuntimeComponent
{
    ref class RomFile;

    public ref class StandardController sealed
    {
    public:
        void A(bool state);
        void B(bool state);
        void Select(bool state);
        void Start(bool state);
        void Up(bool state);
        void Down(bool state);
        void Left(bool state);
        void Right(bool state);

    internal:
        StandardController(IStandardController* controller);

    private:
        IStandardController* _controller;
    };

    public ref class Nes sealed
    {
    public:
        virtual ~Nes();

    public:
        static IAsyncOperation<Nes^>^ Create(RomFile^ rom);
        void DoFrame(WriteOnlyArray<unsigned char>^ screen);
        StandardController^ GetStandardController(unsigned int port);

    private:
        Nes(::Nes* nes);

    private:
        NPtr<::Nes> _nes;
    };
}