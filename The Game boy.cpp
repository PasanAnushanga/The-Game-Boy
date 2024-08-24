#include <Arduino.h>
#include <U8g2lib.h>
#include <Encoder.h>

// Initialize the U8G2 library for a ST7920 128x64 display with hardware SPI
U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, /* CS=*/ 10, /* reset=*/ 9);

// Rotary Encoder
Encoder myEnc(6, 7);
const int encoderButtonPin = 8;

// Buttons connected to analog pins
const int buttonPinUp = A0;
const int buttonPinRight = A1;
const int buttonPinDown = A2;
const int buttonPinLeft = A3;

// Buzzer pin
const int buzzerPin = 5;

// Game Constants
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int GRID_SIZE = 4;
const int INITIAL_SNAKE_LENGTH = 5;
const int MIN_DELAY = 50;
const int DEBOUNCE_DELAY = 50;
const int MAX_VOLUME = 10;

// Flappy Bird Constants
const int BIRD_SIZE = 4;
const int PIPE_WIDTH = 10;
const int PIPE_GAP = 20;
const int GRAVITY = 1;
const int FLAP_STRENGTH = -3;
const int PIPE_SPEED = 2;

// Game Variables
int snakeX[32], snakeY[16]; // Reduced size to fit smaller screens
int snakeLength;
int foodX, foodY;
int direction;
bool gameOver = false;
bool gameStarted = false;
unsigned long moveDelay = 200;
unsigned long lastMoveTime = 0;
unsigned long lastDebounceTime = 0;
long oldPosition = -999;
int volumeLevel = 5; // Initial volume level (0-10)

// Flappy Bird Variables
int birdY, birdVelocity;
int pipeX, pipeY;
bool flappyGameOver = false;

// Menu Variables
enum Screen { TITLE, MAIN_MENU, VOLUME_MENU, GAME_MENU, GAME_SNAKE, GAME_FLAPPY, ABOUT_ME, EXIT };
Screen currentScreen = TITLE;
int menuSelection = 0;

// Debouncing variables
enum ButtonState { BUTTON_IDLE, BUTTON_DEBOUNCING, BUTTON_PRESSED };
ButtonState buttonState = BUTTON_IDLE;
unsigned long buttonPressTime = 0;

void setup() {
  // Initialize the display
  u8g2.begin();

  // Initialize buttons
  pinMode(buttonPinUp, INPUT_PULLUP);
  pinMode(buttonPinRight, INPUT_PULLUP);
  pinMode(buttonPinDown, INPUT_PULLUP);
  pinMode(buttonPinLeft, INPUT_PULLUP);
  pinMode(encoderButtonPin, INPUT_PULLUP);

  // Initialize buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Display title screen
  displayTitleScreen();
}

void loop() {
  long newPosition = -myEnc.read() / 4; // Reverse the direction of the rotary encoder
  bool encoderButtonState = digitalRead(encoderButtonPin) == LOW;
  bool flappyButtonState = digitalRead(buttonPinUp) == LOW; // Use button A0 for Flappy Bird

  handleButtonDebounce(encoderButtonState);

  switch (currentScreen) {
    case MAIN_MENU:
      handleMainMenu(newPosition);
      break;
    case VOLUME_MENU:
      handleVolumeMenu(newPosition);
      break;
    case GAME_MENU:
      handleGameMenu(newPosition);
      break;
    case GAME_SNAKE:
      handleSnakeGame();
      break;
    case GAME_FLAPPY:
      handleFlappyBirdGame(flappyButtonState);
      break;
    case ABOUT_ME:
      handleAboutMe();
      break;
    default:
      break;
  }
}

void handleButtonDebounce(bool buttonStateNow) {
  switch (buttonState) {
    case BUTTON_IDLE:
      if (buttonStateNow) {
        buttonState = BUTTON_DEBOUNCING;
        buttonPressTime = millis();
      }
      break;

    case BUTTON_DEBOUNCING:
      if ((millis() - buttonPressTime) > DEBOUNCE_DELAY) {
        if (buttonStateNow) {
          buttonState = BUTTON_PRESSED;
          handleButtonPress();
        } else {
          buttonState = BUTTON_IDLE;
        }
      }
      break;

    case BUTTON_PRESSED:
      if (!buttonStateNow) {
        buttonState = BUTTON_IDLE;
      }
      break;
  }
}

void handleButtonPress() {
  playSound(1000, 200); // Play a sound when a button is pressed
  
  switch (currentScreen) {
    case TITLE:
      displayMainMenu();
      currentScreen = MAIN_MENU;
      break;
    case MAIN_MENU:
      switch (menuSelection) {
        case 0:
          displayGameMenu();
          currentScreen = GAME_MENU;
          break;
        case 1:
          displayVolumeMenu();
          currentScreen = VOLUME_MENU;
          break;
        case 2:
          displayAboutMe();
          currentScreen = ABOUT_ME;
          break;
        case 3: // Exit selected
          displayTitleScreen();
          currentScreen = TITLE;
          break;
      }
      break;
    case GAME_MENU:
      if (menuSelection == 0) {
        initializeSnakeGame();
        currentScreen = GAME_SNAKE;
      } else if (menuSelection == 1) {
        initializeFlappyBirdGame();
        currentScreen = GAME_FLAPPY;
      }
      break;
    case VOLUME_MENU:
    case ABOUT_ME:
      displayMainMenu();
      currentScreen = MAIN_MENU;
      break;
    default:
      break;
  }
}

void displayTitleScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB10_tf);
  int widthTitle = u8g2.getStrWidth("The Game Boy");
  int xTitle = (SCREEN_WIDTH - widthTitle) / 2;
  u8g2.drawStr(xTitle, 25, "The Game Boy");
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int widthPrompt = u8g2.getStrWidth("Press to Start");
  int xPrompt = (SCREEN_WIDTH - widthPrompt) / 2;
  u8g2.drawStr(xPrompt, 45, "Press to Start");
  u8g2.sendBuffer();
}

void displayMainMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tf);
  int widthGames = u8g2.getStrWidth("1. Games");
  int widthVolume = u8g2.getStrWidth("2. Volume");
  int widthAboutMe = u8g2.getStrWidth("3. About Me");
  int widthExit = u8g2.getStrWidth("4. Exit");
  int xGames = (SCREEN_WIDTH - widthGames) / 2;
  int xVolume = (SCREEN_WIDTH - widthVolume) / 2;
  int xAboutMe = (SCREEN_WIDTH - widthAboutMe) / 2;
  int xExit = (SCREEN_WIDTH - widthExit) / 2;
  u8g2.drawStr(xGames, 15, "1. Games");
  u8g2.drawStr(xVolume, 30, "2. Volume");
  u8g2.drawStr(xAboutMe, 45, "3. About Me");
  u8g2.drawStr(xExit, 60, "4. Exit");
  drawMainMenuHighlight();
  u8g2.sendBuffer();
}

void drawMainMenuHighlight() {
  u8g2.drawFrame(10, 3 + 15 * menuSelection, 108, 16);
}

void handleMainMenu(long newPosition) {
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    menuSelection = newPosition % 4; // 4 options now
    if (menuSelection < 0) menuSelection = 3;
    updateMainMenuDisplay();
  }
}

void updateMainMenuDisplay() {
  displayMainMenu();
}

void displayVolumeMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tf);
  int widthVolumeControl = u8g2.getStrWidth("Volume Control");
  int xVolumeControl = (SCREEN_WIDTH - widthVolumeControl) / 2;
  u8g2.drawStr(xVolumeControl, 20, "Volume Control");
  drawVolumeBar();
  u8g2.sendBuffer();
}

void drawVolumeBar() {
  u8g2.drawFrame(20, 40, 80, 10);
  int volumeWidth = map(volumeLevel, 0, MAX_VOLUME, 0, 78);
  u8g2.drawBox(21, 41, volumeWidth, 8);
}

void handleVolumeMenu(long newPosition) {
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    volumeLevel = constrain(newPosition % (MAX_VOLUME + 1), 0, MAX_VOLUME); // Keep volume level between 0 and 10
    displayVolumeMenu();
  }
}

void displayGameMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tf);
  int widthSnake = u8g2.getStrWidth("1. Snake");
  int widthFlappy = u8g2.getStrWidth("2. Flappy Bird");
  int xSnake = (SCREEN_WIDTH - widthSnake) / 2;
  int xFlappy = (SCREEN_WIDTH - widthFlappy) / 2;
  u8g2.drawStr(xSnake, 20, "1. Snake");
  u8g2.drawStr(xFlappy, 40, "2. Flappy Bird");
  drawGameMenuHighlight();
  u8g2.sendBuffer();
}

void drawGameMenuHighlight() {
  u8g2.drawFrame(10, 7 + 20 * menuSelection, 108, 20);
}

void handleGameMenu(long newPosition) {
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    menuSelection = newPosition % 2;
    if (menuSelection < 0) menuSelection = 1;
    updateGameMenuDisplay();
  }
}

void updateGameMenuDisplay() {
  displayGameMenu();
}

void handleSnakeGame() {
  if (!gameOver) {
    handleButtonInputs();
    if (millis() - lastMoveTime >= moveDelay) {
      moveSnake();
      checkCollision();
      lastMoveTime = millis();
      u8g2.clearBuffer();
      drawSnake();
      drawFood();
      u8g2.sendBuffer();
    }
  } else {
    playCreativeGameOverSound(); // Play creative sound pattern when the game is over
    displayGameOverScreen();
  }
}

void displayGameOverScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB10_tf);
  int widthGameOver = u8g2.getStrWidth("Game Over!");
  int xGameOver = (SCREEN_WIDTH - widthGameOver) / 2;
  u8g2.drawStr(xGameOver, 32, "Game Over!");
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int widthPressKey = u8g2.getStrWidth("Press any key");
  int widthToRestart = u8g2.getStrWidth("to restart");
  int xPressKey = (SCREEN_WIDTH - widthPressKey) / 2;
  int xToRestart = (SCREEN_WIDTH - widthToRestart) / 2;
  u8g2.drawStr(xPressKey, 50, "Press any key");
  u8g2.drawStr(xToRestart, 60, "to restart");
  u8g2.sendBuffer();

  if (isAnyButtonPressed()) {
    delay(DEBOUNCE_DELAY);
    if (isAnyButtonPressed()) {
      gameOver = false;
      initializeSnakeGame();
    }
  }
}

bool isAnyButtonPressed() {
  return analogRead(buttonPinUp) < 512 || analogRead(buttonPinRight) < 512 ||
         analogRead(buttonPinDown) < 512 || analogRead(buttonPinLeft) < 512;
}

void drawSnake() {
  for (int i = 0; i < snakeLength; i++) {
    u8g2.drawBox(snakeX[i], snakeY[i], GRID_SIZE, GRID_SIZE);
  }
}

void drawFood() {
  u8g2.setDrawColor(1);
  u8g2.drawBox(foodX, foodY, GRID_SIZE, GRID_SIZE);
}

void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }
  switch (direction) {
    case 0: snakeY[0] -= GRID_SIZE; break;
    case 1: snakeX[0] += GRID_SIZE; break;
    case 2: snakeY[0] += GRID_SIZE; break;
    case 3: snakeX[0] -= GRID_SIZE; break;
  }
}

void checkCollision() {
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;
    generateFood();
    increaseSpeed();
  }
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true;
    }
  }
  if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH || snakeY[0] < 0 || snakeY[0] >= SCREEN_HEIGHT) {
    gameOver = true;
  }
}

void generateFood() {
  do {
    foodX = random(SCREEN_WIDTH / GRID_SIZE) * GRID_SIZE;
    foodY = random(SCREEN_HEIGHT / GRID_SIZE) * GRID_SIZE;
  } while (isPositionOccupied(foodX, foodY));
}

bool isPositionOccupied(int x, int y) {
  for (int i = 0; i < snakeLength; i++) {
    if (snakeX[i] == x && snakeY[i] == y) {
      return true;
    }
  }
  return false;
}

void handleButtonInputs() {
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (analogRead(buttonPinUp) < 512 && direction != 2) direction = 0;
    if (analogRead(buttonPinRight) < 512 && direction != 3) direction = 1;
    if (analogRead(buttonPinDown) < 512 && direction != 0) direction = 2;
    if (analogRead(buttonPinLeft) < 512 && direction != 1) direction = 3;
    lastDebounceTime = millis();
  }
}

void initializeSnakeGame() {
  snakeLength = INITIAL_SNAKE_LENGTH;
  int startX = SCREEN_WIDTH / 2;
  int startY = SCREEN_HEIGHT / 2;
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = startX - i * GRID_SIZE;
    snakeY[i] = startY;
  }
  direction = 1;
  generateFood();
  moveDelay = 200;
}

void increaseSpeed() {
  if (moveDelay > MIN_DELAY) {
    moveDelay -= 10;
  }
}

void handleFlappyBirdGame(bool buttonState) {
  if (!flappyGameOver) {
    if (buttonState) {
      birdVelocity = FLAP_STRENGTH;
    }
    birdVelocity += GRAVITY;
    birdY += birdVelocity;

    pipeX -= PIPE_SPEED;
    if (pipeX < -PIPE_WIDTH) {
      pipeX = SCREEN_WIDTH;
      pipeY = random(SCREEN_HEIGHT / 4, 3 * SCREEN_HEIGHT / 4);
    }

    if (birdY <= 0 || birdY >= SCREEN_HEIGHT - BIRD_SIZE ||
        (pipeX <= BIRD_SIZE + 10 && (birdY <= pipeY - PIPE_GAP / 2 || birdY >= pipeY + PIPE_GAP / 2))) {
      flappyGameOver = true;
    }

    u8g2.clearBuffer();
    drawFlappyBackground();
    drawBird();
    drawPipe();
    u8g2.sendBuffer();
  } else {
    playCreativeGameOverSound(); // Play creative sound pattern when the game is over
    displayFlappyGameOverScreen();
  }
}

void drawFlappyBackground() {
  u8g2.setDrawColor(1);
  for (int i = 0; i < SCREEN_HEIGHT; i += 2) {
    u8g2.drawPixel(0, i);
    u8g2.drawPixel(SCREEN_WIDTH - 1, i);
  }
}

void displayFlappyGameOverScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB10_tf);
  int widthGameOver = u8g2.getStrWidth("Game Over!");
  int xGameOver = (SCREEN_WIDTH - widthGameOver) / 2;
  u8g2.drawStr(xGameOver, 32, "Game Over!");
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int widthPressKey = u8g2.getStrWidth("Press any key");
  int widthToRestart = u8g2.getStrWidth("to restart");
  int xPressKey = (SCREEN_WIDTH - widthPressKey) / 2;
  int xToRestart = (SCREEN_WIDTH - widthToRestart) / 2;
  u8g2.drawStr(xPressKey, 50, "Press any key");
  u8g2.drawStr(xToRestart, 60, "to restart");
  u8g2.sendBuffer();

  if (isAnyButtonPressed()) {
    delay(DEBOUNCE_DELAY);
    if (isAnyButtonPressed()) {
      flappyGameOver = false;
      initializeFlappyBirdGame();
    }
  }
}

void drawBird() {
  u8g2.setDrawColor(1); // Draw filled shapes
  u8g2.drawDisc(10, birdY, BIRD_SIZE / 2);
}

void drawPipe() {
  u8g2.setDrawColor(1);
  u8g2.drawBox(pipeX, 0, PIPE_WIDTH, pipeY - PIPE_GAP / 2);
  u8g2.drawBox(pipeX, pipeY + PIPE_GAP / 2, PIPE_WIDTH, SCREEN_HEIGHT - pipeY - PIPE_GAP / 2);
}

void initializeFlappyBirdGame() {
  birdY = SCREEN_HEIGHT / 2;
  birdVelocity = 0;
  pipeX = SCREEN_WIDTH;
  pipeY = random(SCREEN_HEIGHT / 4, 3 * SCREEN_HEIGHT / 4);
  flappyGameOver = false;
}

void displayAboutMe() {
  u8g2.clearBuffer();
  
  // Set font and calculate widths
  u8g2.setFont(u8g2_font_helvB08_tf);
  int widthTitle = u8g2.getStrWidth("About Me");
  int xTitle = (SCREEN_WIDTH - widthTitle) / 2;
  u8g2.drawStr(xTitle, 10, "About Me");
  
  // Set font and calculate widths
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int widthLine1 = u8g2.getStrWidth("Game Boy Version 1.0");
  int widthLine2 = u8g2.getStrWidth("Developed by Pasan");
  int widthLine3 = u8g2.getStrWidth("Enjoy your game!");
  
  int xLine1 = (SCREEN_WIDTH - widthLine1) / 2;
  int xLine2 = (SCREEN_WIDTH - widthLine2) / 2;
  int xLine3 = (SCREEN_WIDTH - widthLine3) / 2;
  
  // Draw strings
  u8g2.drawStr(xLine1, 35, "Game Boy Version 1.0");
  u8g2.drawStr(xLine2, 45, "Developed by Pasan.");
  u8g2.drawStr(xLine3, 55, "Enjoy your game!");
  
  // Send buffer to display
  u8g2.sendBuffer();
}

void handleAboutMe() {
  if (digitalRead(encoderButtonPin) == LOW) {
    displayMainMenu();
    currentScreen = MAIN_MENU;
  }
}

// Play sound on the buzzer
void playSound(int frequency, int duration) {
  int adjustedFrequency = frequency * (volumeLevel + 1) / MAX_VOLUME; // Adjust frequency based on volume level
  tone(buzzerPin, adjustedFrequency, duration);
}

// Play creative game over sound pattern on the buzzer
void playCreativeGameOverSound() {
  int frequencies[] = { 400, 800, 600, 1000, 500 }; // Define a pattern of frequencies
  int durations[] = { 200, 200, 200, 200, 500 };  // Define the corresponding durations
  
  for (int i = 0; i < 5; i++) {
    playSound(frequencies[i], durations[i]);
    delay(durations[i]); // Wait for the duration of the note before playing the next
  }
}
