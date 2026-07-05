#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

PSP_MODULE_INFO("HelloWorld", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}

int callback_thread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int main(void) {
    int thid = sceKernelCreateThread("cb", callback_thread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) sceKernelStartThread(thid, 0, 0);

    pspDebugScreenInit();
    pspDebugScreenPrintf("Hello World from PSP DevLink!\n");
    pspDebugScreenPrintf("If you see this, homebrew works.\n");
    pspDebugScreenPrintf("Press HOME to exit.\n");

    sceKernelSleepThread();
    return 0;
}
