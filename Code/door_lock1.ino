#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// ==========================================
// CONSTANTS & PIN DEFINITIONS
// ==========================================
constexpr byte BUZZER_PIN   = 11;
constexpr byte SERVO_PIN    = 10;

constexpr byte SERVO_OPEN_ANGLE   = 50;
constexpr byte SERVO_LOCKED_ANGLE = 110;

constexpr byte MAX_PASS_LEN = 20;
const char DEFAULT_PASSWORD[] = "0123";

// Keypad Configuration
constexpr byte ROWS = 4;
constexpr byte COLS = 4;

char keys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

// ==========================================
// PASSWORD CLASS
// ==========================================
class Password {
public:
  Password(const char* pass) { set(pass); reset(); }

  void set(const char* pass) { target = pass; }

  void reset() {
    currentIndex = 0;
    guess[0] = '\0';
  }

  bool append(char character) {
    if (currentIndex + 1 >= MAX_PASS_LEN) return false;
    guess[currentIndex++] = character;
    guess[currentIndex] = '\0';
    return true;
  }

  bool evaluate() {
    byte i = 0;
    while (target[i] != '\0' || guess[i] != '\0') {
      if (target[i] != guess[i]) return false;
      i++;
    }
    return true;
  }

private:
  const char* target;
  char guess[MAX_PASS_LEN];
  byte currentIndex;
};

// ==========================================
// OBJECT INSTANTIATION & GLOBALS
// ==========================================
Servo doorServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Password password(DEFAULT_PASSWORD);

bool isLocked = false;
byte currentInputLength = 0;
const byte targetPassLength = 4;  // Length of "0123"

// ==========================================
// HELPER FUNCTIONS
// ==========================================
void beepBuzzer(int durationMs) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(durationMs);
  digitalWrite(BUZZER_PIN, LOW);
}

void playSuccessBeep() {
  beepBuzzer(300);
}

void playErrorBeeps() {
  for (int i = 0; i < 3; i++) {
    beepBuzzer(200);
    delay(200);
  }
}

void displayMessage(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void resetInput() {
  password.reset();
  currentInputLength = 0;
  lcd.clear();
}

void unlockDoor() {
  doorServo.write(SERVO_OPEN_ANGLE);
  isLocked = false;
  playSuccessBeep();
  displayMessage("CORRECT PASSWORD", "DOOR UNLOCKED");
  delay(2000);
  resetInput();
}

void lockDoor() {
  doorServo.write(SERVO_LOCKED_ANGLE);
  isLocked = true;
  playSuccessBeep();
  displayMessage("CORRECT PASSWORD", "  DOOR LOCKED");
  delay(2000);
  resetInput();
}

void handlePasswordSubmit() {
  if (password.evaluate()) {
    if (isLocked) {
      unlockDoor();
    } else {
      lockDoor();
    }
  } else {
    playErrorBeeps();
    displayMessage("WRONG PASSWORD!", "PLEASE TRY AGAIN");
    delay(2000);
    resetInput();
  }
}

void processKeyInput(char key) {
  // Print masking character '*'
  lcd.setCursor(5 + currentInputLength, 1);
  lcd.print("*");
  
  password.append(key);
  currentInputLength++;

  if (currentInputLength == targetPassLength) {
    handlePasswordSubmit();
  }
}

// ==========================================
// SETUP & MAIN LOOP
// ==========================================
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  doorServo.attach(SERVO_PIN);
  doorServo.write(SERVO_OPEN_ANGLE);

  lcd.init();
  lcd.backlight();
  displayMessage("  WELCOME TO", "DOOR LOCK SYSTEM");
  delay(3000);
  lcd.clear();
}

void loop() {
  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD");

  char key = keypad.getKey();
  if (key != NO_KEY) {
    delay(60); // Debounce delay

    if (key == 'C') {
      resetInput();
    } 
    else if (key == 'D') {
      // Toggle door directly via 'D' button
      if (isLocked) {
        unlockDoor();
      } else {
        lockDoor();
      }
    } 
    else {
      processKeyInput(key);
    }
  }
}