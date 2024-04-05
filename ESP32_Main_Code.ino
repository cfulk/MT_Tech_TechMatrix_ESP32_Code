
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyCallbacks : public BLECharacteristicCallbacks {

    // Run when BLE characteristic is written to
    void onWrite(BLECharacteristic *pCharacteristic) {

      std::string message = pCharacteristic->getValue();  // Encoded message received

      // REMOVE
      Serial0.println("*********");
      Serial0.println("Message received: ");
      for (size_t i = 0; i < message.length(); i++){
        Serial0.println(int(message[i]));
      }
      Serial0.println();
      Serial0.println("*********");

      // Expected Format: <Signature> <GS> <Message Type> <GS> <Message | Data> <EOT>

      // Get signature (read until next GS character)
      std::string signature = "";
      size_t curPosition = 0;
      while (message[curPosition] != 29 && curPosition < message.length()) {
        signature += message[curPosition];
        curPosition++;
      }
      curPosition++;

      // If signature is valid
      if (signature == "TMATRIX") {

        // Get message type (read until next GS character)
        std::string messageType = "";
        while (message[curPosition] != 29 && curPosition < message.length()) {
          messageType += message[curPosition];
          curPosition++;
        }
        curPosition++;

        // If message is a message to scroll
        if (messageType == "M") {

          // Get message to scroll (read until EOT character)
          std::string messageToScroll = "";
          while (message[curPosition] != 4 && curPosition < message.length()) {
            messageToScroll += message[curPosition];
            curPosition++;
          }
          curPosition++;

          // Print received message in serial window
          Serial0.println("*********");
          Serial0.print("Message received: ");
          for (size_t i = 0; i < messageToScroll.length(); i++)
            Serial0.print(messageToScroll[i]);
          Serial0.println();
          Serial0.println("*********");

          // Relay message to connected TechMatrix board...

        }

        // If message is a graphic
        if (messageType == "G") {

          // Note: Cannot use EOT character as stopping point, as data values can evaluate to control characters

          // Get frame count
          int frameCount = message[curPosition];
          if (frameCount < 2) frameCount = 2;
          curPosition++;

          // Get frame rate
          int fps = message[curPosition];
          if (fps < 1) fps = 1;
          curPosition++;

          // Get pixel values
          std::vector<int> pixelColumns;
          for (size_t col = 0; col < (frameCount * 5); col++) {
            pixelColumns.push_back(message[curPosition]);
            curPosition++;
          }

          // Print received graphic data in serial window
          Serial0.println("*********");
          Serial0.print("Graphic received: ");
          Serial0.print("Frame Count: ");
          Serial0.print(frameCount);
          Serial0.println();
          Serial0.print("Frame Rate (FPS): ");
          Serial0.print(fps);
          Serial0.println();
          for (size_t col = 0; col < pixelColumns.size(); col++) {
            Serial0.print("Column ");
            Serial0.print(col);
            Serial0.print(": ");
            Serial0.print(pixelColumns[col]);
            Serial0.println();
          }
          Serial0.println("*********");

          // Relay message to connected TechMatrix board...

        }
      }
    }
};


// Node class for menu system's tree structure
class Menu {
  private:
    std::string label;
    std::vector<Menu*> children;
    Menu* parent;

  public:
    std::function<void()> action;

    // Constructor
    // label:     Description of menu/submenu
    // action:    Function to execute when menu is loaded
    // children:  Vector of submenus accessible from this menu
    Menu(const std::string label, std::function<void()> action, std::vector<Menu*> children = {})
        : label(label), action(action), children(children), parent(nullptr) {}

    // Access this menu's label
    std::string getLabel(){
      return this->label;
    }

    // Get vector of child submenus
    std::vector<Menu*> getChildren(){
      return this->children;
    }

    // Get this menu's parent menu
    Menu* getParent(){
      return this->parent;
    }

    // Adds a submenu to this menu
    void addChild(Menu* submenu){
      this->children.push_back(submenu);
      submenu->setParent(this);
    }

    // Assigns this menu a parent menu for backwards navigation
    bool setParent(Menu* parentMenu){
      return this->parent = parentMenu;
    }

    // Print all submenu labels
    void printChildren(){
      Serial.println();

      for(size_t i = 0; i < this->children.size(); i++){
        Serial.print(i + 1);
        Serial.print(":\t");
        Serial.println(this->children[i]->getLabel().c_str());
      }

      // Print parent menu label for backwards navigation
      Serial.print(this->children.size() + 1);
      Serial.println(":\tPrevious Menu");
    }
};



void setup() {
  Serial0.begin(9600);      // BLE serial port for BLE app to ESP32 communication
  Serial.begin(9600);       // UART serial port for computer to ESP32 communication
  Serial.print("\f\r\n");   // Begin writing UART output on new page

  BLEDevice::init("MyESP32");                                   // Name device
  BLEServer *pServer = BLEDevice::createServer();               // Set up server
  BLEService *pService = pServer->createService(SERVICE_UUID);  // Set up service

  // Set up readable/writable characteristic for receiving commands
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  //pCharacteristic->setValue("Hello");
  pService->start();

  // Begin advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}


// Declare all menus
Menu* rootMenu = new Menu("Root Menu", [](){ rootMenuAction(); });
Menu* printMessageMenu = new Menu("Print a Message", [](){ printMessageMenuAction(); });
Menu* aboutMenu = new Menu("About TechMatrix", [](){ aboutMenuAction(); });

// Do this when the root menu loads
void rootMenuAction(){
  Serial.println("\n**************************************");
  Serial.println("\tWelcome to TechMatrix!");
  Serial.println("**************************************\n");
  Serial.println("What would you like to do?");
  rootMenu->printChildren();
}

// Do this when the print message menu loads
void printMessageMenuAction(){
  std::string message = "";       // Message to output to TechMatrix board
  bool messageReceived = false;   // True when user presses Enter

  Serial.println("\n*****\n");
  Serial.println("Type the message you would like to display on your TechMatrix device:\n");

  // Get input until user presses Enter
  while (!messageReceived){

    // Wait for any keypress
    while (Serial.peek() == -1){
      delay(1);
    }

    // Check that keypress is valid, otherwise flush RX buffer and wait for more input
    int input = Serial.peek();

    // If Enter key was pressed, terminate loop
    if (input == 13){
      messageReceived = true;
    }

    // If key was ASCII 32 - 126 or backspace, append to message
    if ((input >= 32 && input <= 126) || input == 8){

      // Append string input to scroll message
      message += char(input);
      
      // Display character input
      Serial.print(char(input));

    }
    
    Serial.read(); // Flush input buffer
    
  }

  // Send message to TechMatrix board here...

  Serial.println("\n\n\rThe message should now be displayed!");
  printMessageMenu->printChildren();
}

// Do this when the about menu loads
void aboutMenuAction(){
  Serial.println("\n*****\n");
  Serial.println("TechMatrix was designed and built at Montana Technological University\n\rby electrical engineering students Ethan Messner, Myles McClernan,\n\rDustin Marquardt, and John Johns as well as computer science student\n\rClay Fulk as a promotional product to advertize Montana Tech's EE and\n\rCS programs.");
  aboutMenu->printChildren();
}


void loop() {

  rootMenu->addChild(printMessageMenu);
  rootMenu->addChild(aboutMenu);

  int byteReceived = -1;  // Byte received in RX buffer
  Menu* currentMenu = rootMenu;

  // Wait until any keypress is received
  while(Serial.read() == -1){
    delay(1);
  }

  // Allow user to navigate menu system indefinitely
  while(1){

    // Display current menu's contents
    currentMenu->action();

    // Wait until any valid keypress is received
    bool validByteReceived = false;
    while (!validByteReceived){

      // Wait for any keypress
      while (Serial.peek() == -1){
        delay(1);
      }

      // Check that keypress is valid, otherwise flush RX buffer and wait for more input
      if (Serial.peek() >= 49 && Serial.peek() <= (49 + currentMenu->getChildren().size())){
        validByteReceived = true;
      }
      else{
        Serial.read();
      }
    }
    
    byteReceived = Serial.read(); // Read ASCII character
    int choice = byteReceived - 48; // For example, 49 ASCII maps to integer 1

    // If chosen submenu is parent menu
    if (choice == (currentMenu->getChildren().size() + 1) && currentMenu->getParent() != nullptr){
      currentMenu = currentMenu->getParent();
    }
    else if (choice == (currentMenu->getChildren().size() + 1) && currentMenu->getParent() == nullptr){
      continue;
    }
    else{
      currentMenu = currentMenu->getChildren()[choice - 1];
    }
  }
}