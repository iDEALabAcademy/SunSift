import asyncio
from bleak import BleakClient, BleakScanner
import numpy as np
from PIL import Image

# --- BLE configuration ---
DEVICE_NAME = "NanoESP32_ImageTX"  # must match BLEDevice::init() in Arduino
CHAR_UUID   = "abcdef01-1234-5678-1234-56789abcdef0"  # same as IMAGE_CHAR_UUID in Arduino

# --- Image configuration ---
IMG_WIDTH  = 32
IMG_HEIGHT = 32
CHANNELS   = 3
EXPECTED_SIZE = IMG_WIDTH * IMG_HEIGHT * CHANNELS  # 3072 bytes for 32x32x3 RGB

# Buffer to accumulate incoming bytes
image_bytes = bytearray()

def handle_notification(_: int, data: bytearray):
    """Called every time a BLE notification arrives."""
    image_bytes.extend(data)
    print(f"Received {len(data)} bytes, total so far: {len(image_bytes)}")

async def main():
    print(f"Scanning for device named '{DEVICE_NAME}' ...")
    device = await BleakScanner.find_device_by_name(DEVICE_NAME)

    if not device:
        print("Device not found. Make sure the ESP32 is powered and advertising.")
        return

    print(f"Connecting to {device.name} ({device.address}) ...")
    async with BleakClient(device) as client:
        await client.start_notify(CHAR_UUID, handle_notification)
        print("Connected. Waiting for full image...")

        # Wait until the entire 3072-byte image arrives
        while len(image_bytes) < EXPECTED_SIZE:
            await asyncio.sleep(0.1)

        await client.stop_notify(CHAR_UUID)

    print(f"Full image received: {len(image_bytes)} bytes")

    # Convert to NumPy array and reshape to (H, W, 3)
    img_array = np.frombuffer(image_bytes, dtype=np.uint8).reshape((IMG_HEIGHT, IMG_WIDTH, CHANNELS))

    # Save and display the image
    img = Image.fromarray(img_array, mode="RGB")
    img.save("received_image.png")
    img.show()
    print("Image saved as received_image.png")

if __name__ == "__main__":
    asyncio.run(main())
