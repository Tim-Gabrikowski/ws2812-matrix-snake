#include <Adafruit_NeoPixel.h>

#define LED_PIN D4
#define LED_COUNT 64
#define BRIGHTNESS 20
#define MATRIX_HEIGHT 8
#define MATRIX_WIDTH 8

Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

int posX = 4;
int posY = 4;
int dir = 0;
int length = 6;

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
      } else if (value == length) {
        pixels.setPixelColor(id, pixels.Color(255, 255, 0));
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

void death() {
  clearGameboard();
}

bool isWalkable(int x, int y) {
  bool self = gameboard[y][x] > 0 && gameboard[y][x] < 64;
  return (x < MATRIX_WIDTH) && (x >= 0) && (y < MATRIX_HEIGHT) && (y >= 0) && !self;
}

void randomTurn() {
  if(random(0, 100) > 50) {
    dir = (dir + 90);
    if(dir >= 360) {
      dir = 0;
    }
  } else {
    if(dir <= 0) {
      dir = 270;
    } else {
      dir -= 90;
    }
  }
}

void moveSnake() {
  int nX = posX;
  int nY = posY;

  randomTurn();

  switch (dir) {
    case 0:
      nY--;
      break;
    case 90:
      nX++;
      break;
    case 180:
      nY++;
      break;
    case 270:
      nX--;
      break;
    default:
      death();
      break;
  }
  if(isWalkable(nX, nY)) {
    posX = nX;
    posY = nY;
    gameboard[posY][posX] = length + 1;
  } else {
    // randomTurn();
    moveSnake();
  }
}

void removeSnakeEnd() {
  for(int y = 0; y < MATRIX_HEIGHT; y++) {
    for(int x = 0; x < MATRIX_WIDTH; x++) {
      if(gameboard[y][x] > 0 && gameboard[y][x] < 64) {
        gameboard[y][x] -= 1;
      }
    }
  }
}

void updateGameboard() {
  moveSnake();
  removeSnakeEnd();
  // apple?
}


void setup() {
  clearGameboard();

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(BRIGHTNESS);

  gameboard[posY][posX] = length;
}

void loop() {
  pixels.clear();
  renderGameBoard();
  pixels.show();

  updateGameboard();
  delay(500);
}
