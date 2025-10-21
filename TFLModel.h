#pragma once

#ifdef swap
#undef swap
#endif

#include <Chirale_TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h" 
#include <Arduino.h>


class TFLModel {
private:
    const tflite::Model* model; 
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input;
    TfLiteTensor* output;
    static constexpr int kTensorArenaSize = 84 * 1024;
    alignas(16) uint8_t tensor_arena[kTensorArenaSize];
    
    int input_size;
    int output_size;                 

public:
    TFLModel(const unsigned char*,int input_size, int output_size); // Constructor 
    ~TFLModel();  // Destructor
    int Init(); // Load model, allocate memory, build interpreter 
    void Evaluate(const float* input, float* output); // Gets layer's input and returns output (next layer's input)        
    void normalizeImage(const uint8_t* input, float* normalized_image); // Converts image pixels to float [-1, 1]   
};