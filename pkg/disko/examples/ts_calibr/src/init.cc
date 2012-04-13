#include <disko.h>
#include <cstdio>

#include "tscalibr.h"

int main(int argc, char *argv[]) {

    // initialize disko
    if(!mmsInit(MMSINIT_WINDOWS|MMSINIT_INPUTS, argc, argv, "rom/diskorc.xml",
            "Touchscreen calibration", ""))
    {
        printf("Disko init failed\n");
        return -1;
    }

    try {
        TSCalibr    *ts = new TSCalibr;

        //pause();
        while (1) sleep(1);
        return 0;
    }
    catch(MMSError *error) {
        printf("Abort due to: %s\n", error->getMessage().c_str());
        return 1;
    }
}

