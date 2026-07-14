#include <Wire.h> 
#include <Adafruit_GFX.h> 
#include <Adafruit_SH110X.h> 
#include <Keypad.h> 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
#define I2C_ADDRESS   0x3C 
#define IR_PIN 38  // IR sensor pin 
// Use Adafruit_SH1106G for SH1106 OLED 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, 
SCREEN_HEIGHT, &Wire, OLED_RESET); 
// Define keypad 
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
// Predefined Users 
struct User { 
String id; 
String name; 
}; 
User users[] = { 
{"ABAB", "Abhay"}, 
{"ABCB", "Subhay"}, 
{"ADDA", "Jujay"} 
}; 
int totalUsers = sizeof(users) / sizeof(users[0]); 
void setup() { 
  Serial.begin(115200); 
  pinMode(IR_PIN, INPUT); 
 
  // Initialize OLED display 
  if (!display.begin(I2C_ADDRESS, true)) { 
    Serial.println("OLED initialization failed!"); 
    while (1); 
  } 
 
  Serial.println("OLED initialized successfully!"); 
  display.clearDisplay(); 
  displayFormattedMessage("No Object"); 
  Serial.println("System Ready - Waiting for Object..."); 
} 
 
void loop() { 
  int ir_value = digitalRead(IR_PIN); 
 
  if (ir_value == LOW) {   
    Serial.println("\nObject Detected!"); 
    displayFormattedMessage("Object Detected!"); 
    delay(2000); 
 
    Serial.println("Enter Your ID..."); 
    displayFormattedMessage("Enter Your ID"); 
 
    String enteredID = getUserInput(); // Get ID from the keypad 
 
    Serial.print("Entered ID: "); 
    Serial.println(enteredID); 
 
    String userName = checkUser(enteredID); // Check if ID matches 
    if (userName != "") { 
      Serial.print("Access Granted - Welcome "); 
      Serial.println(userName); 
      displayFormattedMessage("User Located!"); 
    } else { 
      Serial.println("Access Denied - No Access!"); 
      displayFormattedMessage("Invalid Id!"); 
    } 
 
    delay(3000);  // Show message for 3 seconds 
    Serial.println("\nSystem Reset - Waiting for Next Object..."); 
    displayFormattedMessage("No Object"); 
  } else {   
    Serial.println("No Object."); 
    displayFormattedMessage("No Object"); 
  } 
 
  delay(1000); 
} 
 
// Function to get 4-character ID from the keypad 
String getUserInput() { 
  String input = ""; 
  bool firstKeyPressed = false; // Track if first key is pressed 
 
  Serial.print("Entering ID: "); 
 
  while (input.length() < 4) { 
    char key = keypad.getKey(); 
    if (key) { 
      Serial.print(key); // Print the key as it's entered 
      input += key; // Append key to input string 
 
      if (!firstKeyPressed) { 
        firstKeyPressed = true; // First key detected, now update OLED 
      } 
       
      // Display either "Enter Your ID" (before first key) or entered ID (after first key) 
      displayFormattedMessage(firstKeyPressed ? input : "Enter Your ID"); 
 
      delay(300); // Debounce delay 
    } 
  } 
  Serial.println(); // Move to next line after ID entry 
  return input; 
} 
 
// Function to check if the entered ID matches a user 
String checkUser(String enteredID) { 
  for (int i = 0; i < totalUsers; i++) { 
    if (users[i].id == enteredID) { 
      return users[i].name; // Return the user's name if ID matches 
    } 
  } 
  return ""; // Return empty string if no match found 
} 
 
// Function to properly display messages on OLED (Multi-line formatting) 
void displayFormattedMessage(String message) { 
  display.clearDisplay(); 
  display.setTextSize(2); 
  display.setTextColor(SH110X_WHITE); 
 
  int firstSpace = message.indexOf(' '); // Find the first space in the message 
  if (firstSpace != -1) {  
    // If there's a space, split the message into two lines 
    String firstLine = message.substring(0, firstSpace); 
    String secondLine = message.substring(firstSpace + 1); 
 
    display.setCursor(10, 10);  
    display.print(firstLine);   
 
    display.setCursor(10, 35);  
    display.print(secondLine);  
  } else { 
    // If only one word, center it on the screen 
    display.setCursor(10, 20);  
    display.print(message); 
  } 
   
  display.display(); 
}
