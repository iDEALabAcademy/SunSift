#include "TFLModel.h"

//alignas(16) uint8_t TFLModel::tensor_arena[TFLModel::kTensorArenaSize];


TFLModel::TFLModel(const unsigned char* model_data_array,int input_s, int output_s) 
{
  model = tflite::GetModel(model_data_array);
  if (!model || model->version() != TFLITE_SCHEMA_VERSION) {
      Serial.println("[Init] ERROR: Model schema mismatch!");
  }
  input_size = input_s;
  output_size = output_s;
}

TFLModel::~TFLModel() {
  if (interpreter != nullptr) {
    delete interpreter;
    interpreter = nullptr; 
    Serial.println("[Destructor] MicroInterpreter deleted."); // For confirmation
  }

}

int TFLModel::Init() {

    Serial.println("[Init] Model Init....");

    uint8_t* arena_ptr = tensor_arena;

    static tflite::AllOpsResolver resolver;
    interpreter = new tflite::MicroInterpreter(model, resolver, arena_ptr, kTensorArenaSize);
    if (!interpreter) {
      Serial.println("[Init] Interpreter allocation failed.");
    }
    Serial.println("[Init] Interpreter is built.");

    Serial.println("[Init] About to allocate tensors...");
    TfLiteStatus status = interpreter->AllocateTensors();
    Serial.println("[Init] Arena used: ");
    Serial.println(interpreter->arena_used_bytes());
    Serial.println("[Init] Tensor allocation complete");

    if (status != kTfLiteOk) {
        Serial.println("[Init] AllocateTensors is failed.");
        return 0;
    }    

    input = interpreter->input(0);
    output = interpreter->output(0);

    return 1;
}


void TFLModel::Evaluate(const float* in,float* result) {

  for (int i = 0; i < input->bytes / sizeof(float); i++) {
      input->data.f[i] = in[i];
  }

  Serial.println("[evaluate] Invoking interpreter...");
  interpreter->Invoke();
  Serial.println("[evaluate] Invoking interpreter is done.");

  for (int i = 0; i < output->bytes / sizeof(float); i++) {
      result[i] = output->data.f[i];
  }

  Serial.println("[evaluate] output is written.");

}

void TFLModel::normalizeImage(const uint8_t* input, float* output) {
  
  for (int i = 0; i < input_size; ++i) {
    output[i] = (input[i] / 127.5f) - 1.0f;  // Scale 0–255 → -1 to 1
  }
  Serial.println("[normalizeImage] Image normalized.");
  
}