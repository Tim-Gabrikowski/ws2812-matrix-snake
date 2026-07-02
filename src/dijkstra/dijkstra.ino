#include <Adafruit_NeoPixel.h>
#include <math.h>

#define LED_PIN D4
#define LED_COUNT 64
#define BRIGHTNESS 1
#define MATRIX_HEIGHT 8
#define MATRIX_WIDTH 8

#define GROW 15

Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

int length = 1;

uint8_t targetX = 6;
uint8_t targetY = 6;

uint8_t startX = 2;
uint8_t startY = 1;

uint8_t gameboard[MATRIX_HEIGHT][MATRIX_WIDTH];

typedef struct {
  uint8_t posX;
  uint8_t posY;
  uint8_t parentX;
  uint8_t parentY;
  uint8_t cost;
} Node;

Node openList[LED_COUNT];
int openSize = 0;
Node closeList[LED_COUNT];
int closedSize = 0;

Node start = { startX, startY, startX, startY, 0 };
Node target = { targetX, targetY, startX, startY, 255 };

void clearGameboard() {
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      gameboard[y][x] = 0;
    }
  }
}

void renderGameBoard() {
  for (int i = 0; i < openSize; i++) {
    Node n = openList[i];
    uint16_t id = n.posY * MATRIX_WIDTH + n.posX;
    pixels.setPixelColor(id, pixels.Color(0, 0, 255));
  }
  for (int i = 0; i < closedSize; i++) {
    Node n = closeList[i];
    uint16_t id = n.posY * MATRIX_WIDTH + n.posX;
    pixels.setPixelColor(id, pixels.Color(0, 255, 0));
  }

  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      uint16_t id = y * MATRIX_WIDTH + x;

      uint8_t value = gameboard[y][x];

      if (value == 0) {
        // pixels.setPixelColor(id, pixels.Color(0, 0, 0));
      } else if (value == length) {
        pixels.setPixelColor(id, pixels.Color(255, 255, 0));
      } else if (value < LED_COUNT) {
        pixels.setPixelColor(id, pixels.Color(0, 255, 255));
      } else if (value == 255) {
        pixels.setPixelColor(id, pixels.Color(255, 0, 0));
      } else {
        pixels.setPixelColor(id, pixels.Color(0, 0, 255));
      }
    }
  }

  
}

bool isWalkable(int x, int y) {
  if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
    return false;

  uint8_t v = gameboard[y][x];
  return !(v > 1 && v < 64);
}

Node getSmallestOpenAndRemove() {
  // Find smallest
  int minIndex = 0;
  for (int i = 1; i < openSize; i++) {
    if (openList[i].cost < openList[minIndex].cost) {
      minIndex = i;
    }
  }
  Node smallest = openList[minIndex];
  // remove by shifting
  for (int i = minIndex; i < openSize - 1; i++) {
    openList[i] = openList[i + 1];
  }
  openSize--;
  return smallest;
}

bool addNodeToClosed(Node item) {
  if (closedSize >= LED_COUNT) {
    return false;  // Array is full
  }

  closeList[closedSize] = item;
  closedSize++;

  return true;
}

bool updateOrExists(Node n) {
  for (int i = 0; i < openSize; i++) {
    if (openList[i].posX == n.posX && openList[i].posY == n.posY) {
      if (n.cost < openList[i].cost)
        openList[i] = n;

      return true;  // already existed
    }
  }
  return false;
}
bool addNodeToOpen(Node n) {
  if (openSize >= LED_COUNT) {
    return false;  // Array is full
  }

  openList[openSize] = n;
  openSize++;

  return true;
}

Node getNeighbour(Node curr, int dir) {
  int x = curr.posX;
  int y = curr.posY;

  switch (dir) {
    case 0: y--; break;  // 0°
    case 1: x++; break;  // 90°
    case 2: y++; break;  // 180°
    case 3: x--; break;  // 270°
  }

  return Node{ (uint8_t)x, (uint8_t)y, curr.posX, curr.posY, curr.cost + 1 };
}

bool isClosed(uint8_t x, uint8_t y) {
  for (int i = 0; i < closedSize; i++) {
    if (closeList[i].posX == x && closeList[i].posY == y) return true;
  }
  return false;
}

bool dijkstra(Node t) {
  if (openSize == 0) return false;

  Node curr = getSmallestOpenAndRemove();

  addNodeToClosed(curr);

  if (curr.posX == t.posX && curr.posY == t.posY) {
    // target found.
    target = curr;
    return true;
  }

  // get neighbours
  for (int i = 0; i < 4; i++) {
    Node n = getNeighbour(curr, i);

    if (!isWalkable(n.posX, n.posY)) continue;
    if (isClosed(n.posX, n.posY)) continue;

    if (!updateOrExists(n))
      addNodeToOpen(n);
  }
  return false;
}

int buildPath(Node path[]) {
  // Find target node in closeList
  Node current;
  bool found = false;

  for (int i = 0; i < closedSize; i++) {
    if (closeList[i].posX == targetX && closeList[i].posY == targetY) {
      current = closeList[i];
      found = true;
      break;
    }
  }

  if (!found)
    return 0;

  int length = 0;

  while (true) {
    path[length++] = current;

    // reached start
    if (current.posX == startX && current.posY == startY)
      break;

    found = false;

    // find parent in closed list
    for (int i = 0; i < closedSize; i++) {
      if (closeList[i].posX == current.parentX && closeList[i].posY == current.parentY) {
        current = closeList[i];
        found = true;
        break;
      }
    }

    // broken parent chain
    if (!found)
      return 0;
  }

  // Reverse the array so it goes start -> target
  for (int i = 0; i < length / 2; i++) {
    Node tmp = path[i];
    path[i] = path[length - 1 - i];
    path[length - 1 - i] = tmp;
  }

  return length;
}

void setup() {
  clearGameboard();

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(BRIGHTNESS);

  // set start and end
  gameboard[startY][startX] = 1;
  gameboard[targetY][targetX] = 255;

  gameboard[4][4] = 5;
  gameboard[4][5] = 5;
  gameboard[4][6] = 5;
  gameboard[3][6] = 5;


  gameboard[5][6] = 5;
  gameboard[5][5] = 5;
  gameboard[5][8] = 5;
  gameboard[5][3] = 5;
  gameboard[5][2] = 5;


  addNodeToOpen(start);
  pixels.show();
  delay(1000);
}


bool search = true;

void loop() {
  while (search) {
    pixels.clear();

    search = !dijkstra(target);

    renderGameBoard();
    pixels.show();
    delay(100);
  }
  Node path[LED_COUNT];

  int pathLength = buildPath(path);

  for (int i = 0; i < pathLength; i++) {
    Node n = path[i];
    uint16_t id = n.posY * MATRIX_WIDTH + n.posX;
    pixels.setPixelColor(id, pixels.Color(255, 0, 255));
    pixels.show();
    delay(50);
  }
}
