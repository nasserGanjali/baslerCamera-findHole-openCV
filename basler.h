#ifndef BASLER_H
#define BASLER_H

#include <pylon/PylonIncludes.h>
#include <fstream>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#define WIDTH 800
#define HEIGHT 600

// Namespace for using pylon objects.
using namespace Pylon;
#include <pylon/gige/BaslerGigECamera.h>
typedef Pylon::CBaslerGigECamera Camera_t;
using namespace Basler_GigECameraParams;
using namespace Basler_GigEStreamParams;
using namespace Basler_GigETLParams;

// Namespace for using cout.
using namespace std;


class basler
{
public:
    basler();
    int start();
    int startTriggerMode();
    void loadConfig();
    uint8_t globalImageBuffer[WIDTH*HEIGHT];
    CInstantCamera *camera;
    int disconnect();
    //uint8_t *pImageBuffer ;
    bool connect();
    // Number of images to be grabbed.
    static const uint32_t c_countOfImagesToGrab = 1;
    void gpio(bool value);
};

#endif // BASLER_H
