#include <Adafruit_NeoPixel.h>

#define LED_PIN D4
#define LED_COUNT 64
#define BRIGHTNESS 20
#define MATRIX_HEIGHT 8
#define MATRIX_WIDTH 8

Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

uint8_t gameboard[MATRIX_HEIGHT][MATRIX_WIDTH];

void clearGameboard() {
  for(int y = 0; y < MATRIX_HEIGHT; y++) {
    for(int x = 0; x < MATRIX_WIDTH; x++) {
      gameboard[y][x] = 0;
    }
  }
}

void renderGameBoard() {
  for(int y = 0; y < MATRIX_HEIGHT; y++) {
    for(int x = 0; x < MATRIX_WIDTH; x++) {
      uint16_t id = y * MATRIX_WIDTH + x;
      
      uint8_t value = gameboard[y][x];

      if(value == 0) {
        pixels.setPixelColor(id, pixels.Color(0, 0, 0));
      } else if (value < LED_COUNT) {
        pixels.setPixelColor(id, pixels.Color(0, 255, 0));
      } else if (value == 255) {
        pixels.setPixelColor(id, pixels.Color(255, 0, 0));
      } else {
        pixels.setPixelColor(id, pixels.Color(0, 0, 255));
      }
    }
  }
}

void setup() {
  clearGameboard();

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(BRIGHTNESS);

  Serial.begin(9600);
}

void loop() {
  delay(500);
  pixels.clear();
  renderGameBoard();
  pixels.show();
}
