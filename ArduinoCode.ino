#include <Mouse.h>
#include <Keyboard.h>
// Pot/potentiometer Inputs
#define PotX A0
#define PotY A1
// Rotary Encoder Inputs
#define A A2
#define B A3
#define SW 4 // Not using this at the moment
#define Button1 14
#define Button2 15
//Buttons Inputs
const int SHORT_PRESS_TIME = 1000; // 1000 milliseconds
//Joystick movement
int responseDelay = 2;
int lastStateX = 0;
int stateX = 0;
int lastStateY = 0;
int stateY = 0;
int moving = 0;
int speedX = 0;
int speedY = 0;
int delayX = 0;
int delayY = 0;
int deadZone = 100;
int speedWhee = 1;
//vars
int lastStateButton1 = LOW;  // the previous state from the input pin
int currentStateButton1;     // the current reading from the input pin
int lastStateButton2 = LOW;
int currentStateButton2;
unsigned long pressedTime1  = 0;
unsigned long releasedTime1 = 0;
unsigned long pressedTime2  = 0;
unsigned long releasedTime2 = 0;
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;
int speedXFromApp;
String inputString = "";
boolean stringComplete = false;
String commandString = "";
boolean isConnected = false;
String inputs[6];
char commands[6] = {'1','1','1','F','F','F'};

void setup() {

  // Set encoder pins as inputs
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(PotX, INPUT);
  pinMode(PotY, INPUT);
  pinMode(Button1, INPUT);
  pinMode(Button2, INPUT);
  pinMode(SW, INPUT);

  // Setup Serial Monitor
  Serial.begin(9600);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(A);
  Mouse.begin();
  Keyboard.begin();
}

//Function for side scrolling
void sideScroll(int sX, bool up) {
  if (up == true) {
    Keyboard.press(KEY_LEFT_SHIFT);
    Mouse.move(0, 0, sX);
    Keyboard.releaseAll();
  }
  else if (up == false) {
    Keyboard.press(KEY_LEFT_SHIFT);
    Mouse.move(0, 0, -sX);
    Keyboard.releaseAll();
  }
}

bool readSerialData() {
  while (Serial.available()) {
    for (int i = 0; i < 6; i++) {
      commands[i] = (char) Serial.read();
    }
  }
  stringComplete = true;
  return stringComplete;
}




void loop() {
  if (readSerialData()) {
    stringComplete = false;
    String p = String(commands[0]);
    String q = String(commands[2]);
    if (p != 0) {
      speedXFromApp = p.toInt();
    }
    if (q != 0) {
      speedWhee = q.toInt();
    }
  }
  //Rotary Encoder part -----------------------------------------------------
  // Read the current state of CLK
  currentStateCLK = digitalRead(A);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(B) != currentStateCLK) {
      counter --;
      currentDir = "CCW";
      Mouse.move(0, 0, speedWhee);
    } else {
      // Encoder is rotating CW so increment
      counter ++;
      currentDir = "CW";
      Mouse.move(0, 0, -speedWhee);
    }
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  // End of rotary encorer part ---------------------------------------------
  // Joystick part ----------------------------------------------------------
  // Put in a slight delay to help debounce the reading
  stateX = analogRead(PotX);
  stateY = analogRead(PotY);

  double stateXGood = stateX - 512;
  double stateYGood = stateY - 512;

  double distance = sqrt((stateXGood * stateXGood) + (stateYGood * stateYGood));
  speedX = map(distance, 0, 725, 1, 5);
  delayX = map(abs(stateXGood), 0, 512, 100, 10);
  delayY = map(abs(stateYGood), 0, 512, 100, 10);

  if (String(commands[3]).equals("T")) {
    double temp;
    stateXGood = temp;
    stateXGood = stateYGood;
    stateYGood = temp;

    delayX = temp;
    delayX = delayY;
    delayY = temp;
  }
  else {
    double stateXGood = stateX - 512;
    double stateYGood = stateY - 512;

    double distance = sqrt((stateXGood * stateXGood) + (stateYGood * stateYGood));
    speedX = map(distance, 0, 725, 1, 5);
    delayX = map(abs(stateXGood), 0, 512, 100, 10);
    delayY = map(abs(stateYGood), 0, 512, 100, 10);
  }

  if (String(commands[4]).equals("T")) {
    speedX = String(commands[0]).toInt();
    speedY = String(commands[1]).toInt();
    delayX = map(abs(speedX), 0, 9, 20, 10);
    delayY = map(abs(speedY), 0, 9, 20, 10);
  }


  //SideScroll right
  if (stateXGood > deadZone && stateYGood <= deadZone && stateYGood >= -deadZone) {
    Mouse.move(0, 0, -speedX);
    delay(delayX);
  }
  //Up Scroll
  else if (stateYGood > deadZone && stateXGood <= deadZone && stateXGood >= -deadZone) {
    sideScroll(speedX, true);
    delay(delayY);
  }
  //Side Scroll left
  else if (stateXGood < -deadZone && stateYGood <= deadZone && stateYGood >= -deadZone) {
    Mouse.move(0, 0, speedX);
    delay(delayX);
  }
  //Down scroll
  else if (stateYGood < -deadZone && stateXGood <= deadZone && stateXGood >= -deadZone) {
    sideScroll(speedX, false);
    delay(delayY);
  }

  if (String(commands[5]).equals("F")) {
    // diagonals implementation
    if (stateXGood > deadZone && stateYGood > deadZone && distance >= 142) {
      sideScroll(speedX, true);
      delay(delayX);
    }

    else if (stateXGood < -deadZone && stateYGood > deadZone && distance >= 142) {
      Mouse.move(0, 0, speedX);
      sideScroll(speedX, true);
      delay(delayX);
    }

    else if (stateXGood > deadZone && stateYGood < -deadZone && distance >= 142) {
      Mouse.move(0, 0, -speedX);
      sideScroll(speedX, false);
      delay(delayX);
    }

    else if (stateXGood < -deadZone && stateYGood < -deadZone && distance >= 142) {
      Mouse.move(0, 0, speedX);
      sideScroll(speedX, false);
      delay(delayX);
    }
  }

  lastStateX = stateXGood;
  lastStateY = stateYGood;
  // End of Joystick part ---------------------------------------------------
  // Buttons part -----------------------------------------------------------
  currentStateButton1 = digitalRead(Button1);

  if (lastStateButton1 == HIGH && currentStateButton1 == LOW)       // button is pressed
    pressedTime1 = millis();
  else if (lastStateButton1 == LOW && currentStateButton1 == HIGH) { // button is released
    releasedTime1 = millis();
    long pressDuration1 = releasedTime1 - pressedTime1;
    if ( pressDuration1 < SHORT_PRESS_TIME )
      Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_RIGHT_CTRL);
    Keyboard.press('v');
    delay(100);
    Keyboard.releaseAll();
  }

  // save the the last state
  lastStateButton1 = currentStateButton1;
  // save the the last state

  currentStateButton2 = digitalRead(Button2);

  if (lastStateButton2 == HIGH && currentStateButton2 == LOW)       // button is pressed
    pressedTime2 = millis();
  else if (lastStateButton2 == LOW && currentStateButton2 == HIGH) { // button is released
    releasedTime2 = millis();
    long pressDuration2 = releasedTime2 - pressedTime2;
    if ( pressDuration2 < SHORT_PRESS_TIME )
      Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_RIGHT_CTRL);
    Keyboard.press('c');
    delay(100);
    Keyboard.releaseAll();
  }

  // save the the last state
  lastStateButton2 = currentStateButton2;
  // save the the last state
}
