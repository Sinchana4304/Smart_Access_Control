#include <Wire.h> 
#include <Keypad.h> 
#include <Adafruit_SH110X.h> 
// OLED Setup 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1   
#define I2C_ADDRESS 0x3C   
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 
OLED_RESET); 
// Keypad Setup 
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = { 
{'1', '2', '3', 'A'}, 
{'4', '5', '6', 'B'}, 
{'7', '8', '9', 'C'}, 
{'*', '0', '#', 'D'} 
}; 
byte rowPins[ROWS] = {9, 10, 11, 12};    
byte colPins[COLS] = {13, 14, 15, 16};   
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); 
// Password Variables 
String enteredPassword = ""; 
String correctPassword = "1234";   
String secretResetCode = "9999";   
// LED & Buzzer Pins 
const int GREEN_LED = 7;   
const int RED_LED = 8;     
const int BUZZER = 6;      
 
// Attempt & Lockout System 
const int MAX_ATTEMPTS = 3; 
int attemptCount = 0; 
int lockoutLevel = 0;   
bool permanentlyLocked = false;   
 
void setup() { 
  Serial.begin(115200); 
  Wire.begin(); 
   
  pinMode(GREEN_LED, OUTPUT); 
  pinMode(RED_LED, OUTPUT); 
  pinMode(BUZZER, OUTPUT); 
  digitalWrite(GREEN_LED, LOW); 
  digitalWrite(RED_LED, LOW); 
  digitalWrite(BUZZER, LOW); 
 
  if (!display.begin(I2C_ADDRESS)) {   
    Serial.println("  OLED not found!"); 
    while (1); 
  } 
   
  showInitialMessage();   
} 
 
void loop() { 
  if (permanentlyLocked) {  
    checkForReset();   
    return;  
  } 
 
  char key = keypad.getKey(); 
  if (key) { 
    if (key == '*') {   
      if (enteredPassword.length() > 0) { 
        enteredPassword.remove(enteredPassword.length() - 1); 
        Serial.println("      Backspace Pressed");   
      } 
    }  
    else if (key == '#') {   
      Serial.println("  Password Submitted"); 
      checkPassword(); 
    }  
    else { 
      enteredPassword += key; 
      Serial.print("      Entered: "); 
      Serial.println(key);  // Display exact key instead of '*' 
       
      if (enteredPassword.length() > 4) { 
        showMessage("Too Long!", "Try Again"); 
        Serial.println("  Too many characters, resetting input."); 
        delay(2000); 
        enteredPassword = ""; 
      } 
    } 
    updateDisplay(); 
  } 
} 
 
void checkPassword() { 
  if (enteredPassword == correctPassword) { 
    Serial.println("\n   Correct Password!"); 
    digitalWrite(GREEN_LED, HIGH); 
    showMessage("Correct!", "Access OK"); 
    attemptCount = 0; 
    lockoutLevel = 0;   
    delay(2000); 
    digitalWrite(GREEN_LED, LOW); 
  }  
  else if (enteredPassword == secretResetCode) {   
    Serial.println("\n    Reset Code Entered!"); 
    showMessage("System", "Reset!"); 
    attemptCount = 0; 
    lockoutLevel = 0; 
    permanentlyLocked = false;   
    delay(2000); 
    showInitialMessage(); 
  } 
  else { 
    Serial.println("\n  Wrong Password!"); 
    digitalWrite(RED_LED, HIGH); 
    digitalWrite(BUZZER, HIGH); 
    showMessage("Wrong!", "Try Again"); 
    attemptCount++; 
    delay(2000); 
    digitalWrite(RED_LED, LOW); 
    digitalWrite(BUZZER, LOW); 
 
    if (attemptCount >= MAX_ATTEMPTS) { 
      lockoutLevel++; 
      attemptCount = 0; 
 
      if (lockoutLevel >= 3) { 
        permanentlyLocked = true;   
        showMessage("LOCKED!", "Enter Reset Code"); 
        Serial.println("\n    SYSTEM LOCKED PERMANENTLY!"); 
      }  
      else { 
        int lockoutTime = lockoutLevel * 10; 
        showLockout(lockoutTime); 
      } 
    } 
  } 
  enteredPassword = ""; 
  showInitialMessage(); 
} 
 
// Countdown Timer Display 
void showLockout(int seconds) { 
  Serial.print("       Locked for "); 
  Serial.print(seconds); 
  Serial.println(" seconds"); 
 
  for (int i = seconds; i > 0; i--) { 
    display.clearDisplay(); 
    display.setCursor(10, 20); 
    display.setTextSize(2); 
    display.setTextColor(SH110X_WHITE); 
    display.println("LOCKED!"); 
    display.setCursor(10, 40); 
    display.print("Wait: "); 
    display.print(i); 
    display.print("s"); 
    display.display(); 
 
    Serial.print("       "); 
    Serial.print(i); 
    Serial.println(" seconds remaining..."); 
     
    delay(1000); 
  } 
  showInitialMessage(); 
} 
 
// Show "Enter Password" Initially 
void showInitialMessage() { 
  Serial.println("\n  Enter Password:"); 
  display.clearDisplay(); 
  display.setCursor(10, 10); 
  display.setTextSize(2); 
  display.setTextColor(SH110X_WHITE); 
  display.println("Enter"); 
  display.setCursor(10, 30); 
  display.println("Password:"); 
  display.display(); 
} 
 
// Show Entered Password 
void updateDisplay() { 
  display.clearDisplay(); 
  display.setCursor(10, 10); 
  display.setTextSize(2); 
  display.setTextColor(SH110X_WHITE); 
  display.println("Enter"); 
  display.setCursor(10, 30); 
  display.println("Password:"); 
  display.setCursor(10, 50); 
  for (int i = 0; i < enteredPassword.length(); i++) { 
    display.print('*'); 
  } 
  display.display(); 
} 
 
// Show Messages like "Correct" or "Wrong" 
void showMessage(String line1, String line2) { 
  display.clearDisplay(); 
  display.setCursor(10, 20); 
  display.println(line1); 
  display.setCursor(10, 40); 
  display.println(line2); 
  display.display(); 
  Serial.println(line1 + " - " + line2); 
} 
 
// Check for Reset Code 
void checkForReset() { 
  char key = keypad.getKey(); 
  if (key) { 
    if (key == '*') {   
      if (enteredPassword.length() > 0) { 
        enteredPassword.remove(enteredPassword.length() - 1); 
        Serial.println("      Backspace Pressed"); 
      } 
    }  
    else if (key == '#') {   
      if (enteredPassword == secretResetCode) { 
        Serial.println("\n    Reset Code Entered!"); 
        showMessage("System", "Reset!"); 
        attemptCount = 0; 
        lockoutLevel = 0; 
        permanentlyLocked = false;   
        delay(2000); 
        showInitialMessage(); 
      } 
      else { 
        Serial.println("\n  Wrong Reset Code!"); 
        showMessage("Wrong", "Reset Code!"); 
        delay(2000); 
      } 
      enteredPassword = "";   
    }  
    else { 
      enteredPassword += key; 
      Serial.print("      Entered (Reset Mode): "); 
      Serial.println(key); 
      updateDisplay(); 
    } 
  } 
} 
