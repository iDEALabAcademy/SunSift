#pragma once
#include <ArduCAM.h>  
#include <SPI.h>         

class Camera {
private:
    ArduCAM* myCAM;
    int CS;
    uint8_t* originalBuffer;
    uint8_t* rgb888Image;

public:
    enum Mode {
        RGB,
        GrayScale
    };
    Camera(int,int);
    int Init();
    int CaptureImage(uint8_t*);
    int ConvertRGB565toRGB888(uint8_t* , uint8_t*);
    int DownscaleImageNN(uint8_t*, uint8_t*);
    int DownscaleImageAVG(uint8_t*, uint8_t*);
    int ConvertToGrayscale(uint8_t*);
    int CaptureAndProcessImage(uint8_t* downscaledImage, int width, int height, Mode mode);
    int allocateMemory();
    void FlushAndClearFIFO();
};