#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

PSP_MODULE_INFO("PSPDevLink", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

int callback_thread(SceSize args, void *argp)
{
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int setup_callbacks(void)
{
    int thid = sceKernelCreateThread(
        "update_thread",
        callback_thread,
        0x11,
        0xFA0,
        THREAD_ATTR_USER,
        0);

    if (thid >= 0)
        sceKernelStartThread(thid, 0, 0);

    return thid;
}

int main(void)
{
    setup_callbacks();

    pspDebugScreenInit();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    pspDebugScreenPrintf("=================================\n");
    pspDebugScreenPrintf("        PSP DevLink\n");
    pspDebugScreenPrintf("=================================\n\n");

    pspDebugScreenPrintf("Milestone 0 Verification\n\n");

    pspDebugScreenPrintf("[OK] Display Initialized\n");
    pspDebugScreenPrintf("[OK] Controller Initialized\n");
    pspDebugScreenPrintf("[OK] PSP SDK Working\n\n");

    pspDebugScreenPrintf("Press START to exit...\n");

    while (1)
    {
        SceCtrlData pad;

        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_START)
            break;
    }

    sceKernelExitGame();
    return 0;
}