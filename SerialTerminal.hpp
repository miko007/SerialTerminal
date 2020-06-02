#pragma once

#define ST_VERSION "1.1.1"

#include <Arduino.h>

namespace maschinendeck {

  #if not defined ST_FLAG_NOBUILTIN && defined E2END
  #include "EEPROM.h"
  void printEEPROM(String opts) {
    Serial.print("offset \t");
    for (uint8_t h = 0; h < 16; h++) {
        Serial.print(h, HEX);
        Serial.print('\t');
    }
    Serial.print("\r\n");
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
      Serial.print("\r\n");
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
      String message;

      #ifndef ST_FLAG_NOHELP
      void printCommands() {
        for (uint8_t i = 0; i < this->size_; i++) {
          Serial.println("\t" + this->commands[i]->command + "\t" + this->commands[i]->description);
        }
        #ifndef ST_FLAG_NOPROMPT
        Serial.print("st> ");
        #endif
      }
      #endif

    public:
      SerialTerminal(long baudrate = 0) : size_(0), firstRun(true), message("") {
        #if not defined ST_FLAG_NOBUILTIN && defined E2END
            this->add("eeprom", &printEEPROM, "prints the contents of EEPROM");
        #endif
        if (baudrate > 0)
            Serial.begin(baudrate);
        #ifndef ST_FLAG_NOHELP
            Serial.print("SerialTerm v");
            Serial.print(ST_VERSION);
            Serial.print("\r\n");
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
        bool commandComplete = false;
        while (Serial.available()) {
            char car = Serial.read();
            if (car == 127 && this->message.length() > 0) {
                Serial.print("\e[1D");
                Serial.print(' ');
                Serial.print("\e[1D");
                this->message.remove(this->message.length() - 1);
                continue;
            }
            if (isAscii(car))
                Serial.print(car);
            // Check if user ended the line
            if (car == '\r') {
                Serial.print("\r\n");
                commandComplete = true;
                // If there are more data on the line, drop a \n, if it is
                // there. Some terminals may send both, giving 
                // an extra lineend, if we do not drop it.
                if (Serial.available() && Serial.peek() == '\n') {
                    Serial.read();
                }
                break;
            }
            this->message += car;
        }

        if (!commandComplete)
            return;
        if (this->message == "") {
            #ifndef ST_FLAG_NOPROMPT
            Serial.print("st> ");
            #endif

            return;
        }

        Pair<String, String> command = SerialTerminal::ParseCommand(this->message);
        this->message = "";

        #ifndef ST_FLAG_NOPROMPT
        bool found = false;
        #endif
        for (uint8_t i = 0; i < this->size_; i++) {
          if (this->commands[i]->command == command.first()) {
            this->commands[i]->callback(command.second());
            #ifndef ST_FLAG_NOPROMPT
            found = true;
            #endif
          }
        }
        #ifndef ST_FLAG_NOPROMPT
        if (!found) {
            Serial.print(command.first());
            Serial.print(": command not found");
        }
        Serial.print("\r\nst> ");
        #endif
      }

      static Pair<String, String> ParseCommand(String message) {
        String keyword = "";
        for (auto& car : message) {
          if (car == ' ')
            break;
          keyword += car;
        }
        if (keyword != "")
        message.remove(0, keyword.length());
        keyword.trim();
        message.trim();

        return Pair<String, String>(keyword, message);
      }
  };

}
