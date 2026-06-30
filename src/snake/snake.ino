#include <Adafruit_NeoPixel.h>
#include <math.h>

#define LED_PIN D4
#define LED_COUNT 64
#define BRIGHTNESS 2
#define MATRIX_HEIGHT 8
#define MATRIX_WIDTH 8

#define GROW 15

Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

int posX = 4;
int posY = 4;
int dir = 0;
int length = 1;

int count = 0;

int appleX = 8;
int appleY = 0;

int bestDirections[4];

uint8_t gameboard[MATRIX_HEIGHT][MATRIX_WIDTH];

void clearGameboard() {
  for(int y = 0; y < MATRIX_HEIGHT; y++) {
    for(int x = 0; x < MATRIX_WIDTH; x++) {
      gameboard[y][x] = 0;
    }
  }
}

void resetGame() {
  clearGameboard();

  posX = 4;
  posY = 4;
  length = 1;
  dir = 0;
  count = 0;

  gameboard[posY][posX] = length;

  placeApple();

  renderGameBoard();
}

void placeApple() {
  int nX = random(0, MATRIX_WIDTH - 1);
  int nY = random(0, MATRIX_HEIGHT - 1);

  if(isWalkable(nX, nY)) {
    appleX = nX;
    appleY = nY;

    gameboard[nY][nX] = 255;
  } else {
    placeApple();
  }

}
bool onApple() {
  return posX == appleX && posY == appleY;
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
  // animation
  for(int i = 0; i < LED_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    pixels.show();
    delay(10);
  }

  resetGame();
}

bool isWalkable(int x, int y) {
  bool self = gameboard[y][x] > 0 && gameboard[y][x] < 64;
  return (x < MATRIX_WIDTH) && (x >= 0) && (y < MATRIX_HEIGHT) && (y >= 0) && !self;
}


// Basically AI of the game
void setBestDirections() {
    int bestDist[4] = {32767, 32767, 32767, 32767};

    for (int i = 0; i < 4; i++) {
        int x = posX;
        int y = posY;

        switch (i) {
            case 0: y--; break; // 0°
            case 1: x++; break; // 90°
            case 2: y++; break; // 180°
            case 3: x--; break; // 270°
        }

        int dx = appleX - x;
        int dy = appleY - y;
        int dist = dx * dx + dy * dy;

        // Insert into sorted position
        for (int j = 0; j < 4; j++) {
            if (dist < bestDist[j]) {
                for (int k = 3; k > j; k--) {
                    bestDist[k] = bestDist[k - 1];
                    bestDirections[k] = bestDirections[k - 1];
                }
                bestDist[j] = dist;
                bestDirections[j] = i * 90;
                break;
            }
        }
    }
}

void randomTurn() {
  if(random(0, 10) > 5) return;
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

void moveSnake(int dirId = 0) {
  int sX = posX;
  int sY = posY;
  int nX = posX;
  int nY = posY;

  if(dirId == 0) setBestDirections();

  dir = bestDirections[dirId];

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

    // check for dead ends
    if(movePossible()) {
      gameboard[posY][posX] = length + 1;
    } else {
      posX = sX;
      posY = sY;
      moveSnake(dirId - 1);
    }
  } else {
    moveSnake(dirId - 1);
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

bool movePossible() {
  return isWalkable(posX + 1, posY) || isWalkable(posX - 1, posY) || isWalkable(posX, posY + 1) || isWalkable(posX, posY - 1);
}

void updateGameboard() {
  if(!movePossible()) {
    death();
    return;
  } 
  moveSnake();
  removeSnakeEnd();
  // apple?
  if(onApple()) {
    length++;
    placeApple();
  }
}


void setup() {
  clearGameboard();

  resetGame();

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(BRIGHTNESS);
}

void loop() {
  pixels.clear();
  renderGameBoard();
  pixels.show();

  updateGameboard();
  delay(500);
}
