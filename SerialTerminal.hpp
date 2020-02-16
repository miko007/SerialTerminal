#pragma once

#define ST_VERSION "1.0.2"

namespace maschinendeck {

  #if not defined ST_FLAG_NOBUILTIN && defined E2END
  #include "EEPROM.h"
  void printEEPROM(String opts) {
    Serial.print("offset\t");
    for (uint8_t h = 0; h < 16; h++) {
        Serial.print(h, HEX);
        Serial.print('\t');
    }
    Serial.print('\n');
    for (uint8_t i = 0; i < (E2END / 16); i++) {
      String line = "";
      Serial.print(i * 16, HEX);
      Serial.print('\t');
      for (uint8_t n = 0; n <= 15; n++) {
        size_t value = EEPROM.read(i * 16 + n);
        Serial.print(value, HEX);
        Serial.print('\t');
        line += static_cast<char>(value);
      }
      Serial.print(line);
      Serial.print('\n');
    }
  }
  #endif

  struct Command {
    String command;
    void(*callback)(String param);
    String description;

    Command(String command,  void(*callback)(String param), String description) : command(command), callback(callback), description(description) {}
  };

  template <typename T, typename U>
  class Pair {
    private:
      T first_;
      U second_;
    public:
      Pair(T first, U second) : first_(first), second_(second) {}
      T first() {
        return this->first_;  
      }

      U second() {
        return this->second_;  
      }
  };

  class SerialTerminal {
    private:
      Command* commands[64];
      uint8_t size_;
      bool firstRun;

      #ifndef ST_FLAG_NOHELP
      void printCommands() {
        for (uint8_t i = 0; i <= this->size_; i++) {
          Serial.println("\t" + this->commands[i]->command + "\t" + this->commands[i]->description);  
        }  
      }
      #endif
      
    public:
      SerialTerminal(long baudrate) : size_(0), firstRun(true) {
        #if not defined ST_FLAG_NOBUILTIN && defined E2END
          this->add("eeprom", &printEEPROM, "prints the contents of EEPROM");
        #endif     
        Serial.begin(baudrate);
        #ifndef ST_FLAG_NOHELP
          Serial.print("SerialTerm v");
          Serial.print(ST_VERSION);
          Serial.print("\n");
          Serial.println("(C) 2019, MikO");
          Serial.println("  available commands:");
        #endif
      }
      
      void add(String command,  void(*callback)(String param), String description = "") {
        if (this->size_ >= 64)
          return;
        this->commands[this->size_] = new Command(command, callback, description);
        this->size_++;
      }

      uint8_t size() {
        return this->size_ + 1;  
      }

      void loop() {
        #ifndef ST_FLAG_NOHELP
        if (this->firstRun) {
          this->firstRun = false;
            this->printCommands();
        }
        #endif
        if (!Serial.available())
          return;
        String message = Serial.readString();
        if (message == "")
          return;
        Pair<String, String> command = SerialTerminal::ParseCommand(message);
        
        for (uint8_t i = 0; i <= this->size_; i++) {
          if (this->commands[i]->command == command.first())
            this->commands[i]->callback(command.second()); 
        }
      }

      static Pair<String, String> ParseCommand(String message) {
        String keyword = "";
        for (auto& car : message) {
          if (car == ' ')
            break;
          keyword += car;  
        }
        message.remove(0, keyword.length());
        keyword.trim();
        message.trim();

        return Pair<String, String>(keyword, message);
      }
  };
  
}
