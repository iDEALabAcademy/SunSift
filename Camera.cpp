#include "Camera.h"

Camera::Camera(int model, int chip_select) 
{
    CS = chip_select;
    myCAM = new ArduCAM(model, CS);
    digitalWrite(CS, HIGH);
}

int Camera::Init()
{

  uint8_t vid, pid, temp;

  myCAM->write_reg(0x07, 0x80); delay(100);
  myCAM->write_reg(0x07, 0x00); delay(100);

  while (1) {
    myCAM->write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM->read_reg(ARDUCHIP_TEST1);
    delay(100);
    if (temp != 0x55) {
        Serial.println("Capturing image fff");
      return 0;
    } else {
        break;
    }
  }

  while (1) {
    myCAM->wrSensorReg8_8(0xff, 0x01);
    myCAM->rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM->rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26) || (pid != 0x42 && pid != 0x41)) {
      return 0;
    } else {
      break;
    }
  }

  myCAM->set_format(RAW);
  myCAM->InitCAM();
  myCAM->OV2640_set_Light_Mode(Auto);
  myCAM->OV2640_set_Color_Saturation(0);
  myCAM->OV2640_set_Brightness(0);
  myCAM->OV2640_set_Contrast(0);
  myCAM->OV2640_set_Special_effects(Normal);
  myCAM->OV2640_set_JPEG_size(OV2640_160x120);

  delay(1000);
  myCAM->clear_fifo_flag();
  return 1;
}

int Camera::CaptureImage(uint8_t* originalBuffer) {
  myCAM->flush_fifo();
  myCAM->clear_fifo_flag();
  myCAM->start_capture();
  // Serial.println("Capturing image...");
  while (!myCAM->get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  // Serial.println("Image captured.");

  uint32_t length = myCAM->read_fifo_length();
  myCAM->CS_LOW();
  myCAM->set_fifo_burst();

  // Read the image data into the buffer
  for (uint32_t i = 0; i < 160 * 120 * 2; i++) {
    originalBuffer[i] = SPI.transfer(0x00);
  }

  myCAM->CS_HIGH();
  myCAM->clear_fifo_flag();

  // Print first 20 values for debugging
  Serial.println("--- First 20 Image Bytes from RGB565 ---");
  for (int i = 0; i < 20; i++) {
    Serial.print(originalBuffer[i]);
    Serial.print(i < 19 ? "," : "\n"); // Comma-separated, newline at end
  }
  return 1;
}

// Function to convert RGB565 to RGB888
int Camera::ConvertRGB565toRGB888(uint8_t* originalBuffer, uint8_t* rgb888Image) {
  const float redGain = 1.1;      // Most optimal so far: R: 1.1, G: 0.6, B: Does not matter
  const float greenGain = 0.6;
  const float blueGain = 2;

  for (int i = 0; i < 160 * 120; i++) {
    uint16_t pixel = (originalBuffer[2 * i] << 8) | originalBuffer[2 * i + 1]; // Big endian

    uint8_t r = ((pixel >> 11) & 0x1F);
    uint8_t g = ((pixel >> 5) & 0x3F);
    uint8_t b = (pixel & 0x1F);

    r = (r << 3) | (r >> 2);    // 5-bit to 8-bit
    g = (g << 2) | (g >> 4);    // 6-bit to 8-bit
    b = (b << 3) | (b >> 2);    // 5-bit to 8-bit

    r = min(255, int(r * redGain));
    g = min(255, int(g * greenGain));
    b = min(255, int(b * blueGain));

    rgb888Image[3 * i] = r;
    rgb888Image[3 * i + 1] = g;
    rgb888Image[3 * i + 2] = b;
  }

  // Print first 20 values for debugging
  Serial.println("--- First 20 Image Bytes from RGB888 ---");
  for (int i = 0; i < 20; i++) {
    Serial.print(rgb888Image[i]);
    Serial.print(i < 19 ? "," : "\n"); // Comma-separated, newline at end
  }
  return 1;
}

// Function to downscale to 32,32 pixels, uses NN
int Camera::DownscaleImageNN(uint8_t* rgb888Image, uint8_t* downscaledImage) {
  const int srcWidth = 160;
  const int srcHeight = 120;
  const int dstWidth = 32;
  const int dstHeight = 32;

  for (int y = 0; y < dstHeight; y++) {
    for (int x = 0; x < dstWidth; x++) {
      int src_x = (x * srcWidth) / dstWidth;
      int src_y = (y * srcHeight) / dstHeight;

      int src_index = (src_y * srcWidth + src_x) * 3;
      int dst_index = (y * dstWidth + x) * 3;

      downscaledImage[dst_index] = rgb888Image[src_index];
      downscaledImage[dst_index + 1] = rgb888Image[src_index + 1];
      downscaledImage[dst_index + 2] = rgb888Image[src_index + 2];
    }
  }

  // Print first 20 values for debugging
  Serial.println("--- First 20 Image Bytes from downscaledImage ---");
  for (int i = 0; i < 20; i++) {
    Serial.print(downscaledImage[i]);
    Serial.print(i < 19 ? "," : "\n"); // Comma-separated, newline at end
  }
  
  return 32 * 32 * 3;
}

// Function to downscale to 32,32 pixels, uses AVG pooling
int Camera::DownscaleImageAVG(uint8_t* rgb888Image, uint8_t* downscaledImage) {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      int sumR = 0, sumG = 0, sumB = 0;
      for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 5; dx++) {
          int src_x = x * 5 + dx;
          int src_y = y * 4 + dy;
          int index = (src_y * 160 + src_x) * 3;
          sumR += rgb888Image[index];
          sumG += rgb888Image[index + 1];
          sumB += rgb888Image[index + 2];
        }
      }
      int avgIndex = (y * 32 + x) * 3;
      downscaledImage[avgIndex]     = sumR / 20;
      downscaledImage[avgIndex + 1] = sumG / 20;
      downscaledImage[avgIndex + 2] = sumB / 20;
    }
  }
  return 32 * 32 * 3; 
}

// Optional; functiont to convert image to grayscale
int Camera::ConvertToGrayscale(uint8_t* downscaledImage) {
  for (int i = 0; i < 32 * 32; i++) {
    int index = i * 3;
    uint8_t r = downscaledImage[index];
    uint8_t g = downscaledImage[index + 1];
    uint8_t b = downscaledImage[index + 2];

    uint8_t gray = (uint8_t)(0.35 * r + 0.6 * g + 0.05 * b); 

    downscaledImage[index] = gray;
    downscaledImage[index + 1] = gray;
    downscaledImage[index + 2] = gray;
  }
  return 32 * 32;
}


// Wrapper: returns a flat 1D image buffer [32x32x3]
// Caller is responsible for freeing the returned buffer
int Camera::CaptureAndProcessImage(uint8_t* downscaledImage, int width, int height, Mode mode) {
  // Ensure memory is allocated before continuing
  if (!allocateMemory()) {
    return 0;
  }

  // Process the image
  CaptureImage(originalBuffer);
  ConvertRGB565toRGB888(originalBuffer, rgb888Image);
  DownscaleImageNN(rgb888Image, downscaledImage);

  // Clean up temporary buffers
  if (originalBuffer) {
    free(originalBuffer);
    originalBuffer = nullptr;  // To avoid dangling pointer
  }
  if (rgb888Image) {
    free(rgb888Image);
    rgb888Image = nullptr;  // To avoid dangling pointer
  }

  // Return the size of the downscaled image
  return width * height * 3;  // For RGB888, return width * height * 3 bytes
}

int Camera::allocateMemory()
{
  if (!originalBuffer) {
    originalBuffer = (uint8_t*)malloc(160 * 120 * 2);  // RGB565
  }
  if (!rgb888Image) {
    rgb888Image = (uint8_t*)malloc(160 * 120 * 3);  // RGB888
  }

  if (!originalBuffer || !rgb888Image) {
    if (originalBuffer) {
      Serial.println("[originalBuffer] Memory allocation FAILED!");
      free(originalBuffer); // Free if allocated
      originalBuffer = NULL;
    }
    if (rgb888Image) {
      Serial.println("[rgb888Image] Memory allocation FAILED!");
      free(rgb888Image); // Free if allocated
      rgb888Image = NULL;
    }
    return 0;  // Return failure if either allocation fails
  }

  return 1;  // Success if both allocations succeed
}

void Camera::FlushAndClearFIFO() {
    myCAM->flush_fifo();
    myCAM->clear_fifo_flag();
}

