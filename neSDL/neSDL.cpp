#include "stdafx.h"
#include "sdlAudio.h"
#include "sdlGfx.h"
#include "sdlInput.h"
#include "stdStreamRom.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Must provide path to ROM file.\n");
        return -1;
    }

    NPtr<SdlAudioProvider> audioProvider;
    audioProvider.Attach(new SdlAudioProvider(44100));

    NPtr<StdStreamRomFile> romFile;
    romFile.Attach(new StdStreamRomFile(argv[1]));

    NPtr<INes> nes;
    if (!Nes_Create(romFile, audioProvider, &nes))
    {
        return 1;
    }

    IStandardController* controller0 = nes->GetStandardController(0);
    SdlInput input(controller0);

    SdlGfx gfx(3);

    unsigned char screen[256 * 240 * 4];
    for (;;)
    {
        memset(screen, 0, sizeof(screen));

        InputResult result = input.CheckInput();

        if (result == InputResult::SaveState)
        {
            NPtr<IWriteStream> ostream;
            if (romFile->GetSaveStateStream(&ostream))
            {
                nes->SaveState(ostream);
                printf("State Saved!\n");
            }
            else
            {
                printf("Save State Failed!\n");
            }
        }
        else if (result == InputResult::LoadState)
        {
            NPtr<IReadStream> istream;
            if (romFile->GetLoadStateStream(&istream))
            {
                nes->LoadState(istream);
                printf("State Loaded!\n");
            }
            else
            {
                printf("Load State Failed!\n");
            }
        }
        else if (result == InputResult::Quit)
        {
            break;
        }

        nes->DoFrame(screen);
        gfx.Blit(screen);
    }

    nes->Dispose();
}
