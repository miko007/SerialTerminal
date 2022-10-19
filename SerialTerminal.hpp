#pragma once

#define ST_VERSION "1.1.2"

#include <Arduino.h>

namespace maschinendeck {

    /**
     * Naive implementation of `std::move`
     *
     * @author Michael Ochmann <miko@maschinendeck.org>
     * @since 1.1.2
     */
    template <typename T>
    T&& move(T object) {
        return static_cast<T&&>(object);
    }

    /**
     * Naive implementation of `std::forward`
     *
     * @author Michael Ochmann <miko@maschinendeck.org>
     * @since 1.1.2
     */
    template <typename T>
    T&& forward(T& param) {
        return static_cast<T&&>(param);
    }

    /**
     * Prints the contents of EEPROM as a HEX table, if an EEPROM is present.
     *
     * It gets registered as al builtin command to the SerialTerminal, when the
     * flag ST_FLAG_NOBUILTIN is not set.
     *
     * @author Michael Ochmann <miko@maschinendeck.org>
     * @since 0.0.1
     */
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
        void (*callback)(String param);
        String description;

        Command(String&& command,  void(*callback)(String param), String&& description) :
                command(move(command)),
                callback(callback),
                description(move(description)) {}
    };

    /**
     * Naive implementation of `std::pair`
     *
     * this exists, because c++ does not support multiple return types and we
     * want to return a word and the rest of the string in
     * `maschinendeck::ParseCommand()`
     *
     * @author Michael Ochmann <miko@maschinendeck.org>
     * @since 0.0.1
     */
    template <typename T, typename U>
    struct Pair {
        T first_;
        U second_;

        Pair()                       : first_(), second_() {}
        Pair(T&& first, U&& second)  : first_(forward(first)), second_(forward(second)) {}
        Pair(const Pair<T, U>& pair) : first_(pair.first_), second_(pair.second_) {}
        Pair(Pair<T, U>&& pair)      : first_(move(pair.first_)), second_(move(pair.second_)) {}

        const T& first() const  {
                return this->first_;
        }

        const U& second() const {
                return this->second_;
        }

    };

    /**
     * The actual SerialTerminal class
     *
     * @author Michael Ochmann <miko@maschinendeck.org>
     * @since 0.0.1
     */
    class SerialTerminal {
        private:
            Command* commands[64];
            uint8_t size_;
            bool firstRun;
            String message;

            /**
             * Prints a list of all commands
             *
             * If the flag `ST_FLAG_NOHELP` is not set, this prints a list of
             * all registered commands with their description text.
             *
             * @author Michael Ochmann <miko@maschinendeck.org>
             * @since 0.0.1
             */
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
				#if not defined ST_FLAG_NOHELP && not defined ST_FLAG_NOBUILTIN
				this->add("help", [](){}, "shows this help screen");
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

            ~SerialTerminal() {
                for (auto& command : this->commands)
                    delete command;
            }

            void add(String&& command,  void (*callback)(String param), String&& description = "") {
                if (this->size_ >= 64)
                    return;
                this->commands[this->size_] = new Command(
                    move(command),
                    callback,
                    move(description)
                );
                this->size_++;
            }

            /**
             * Returns the actual number of registered commands
             */
            uint8_t size() {
                return this->size_ + 1;
            }

            /**
             * Parses serial input and checks for occuring commands
             *
             * Must be called in the mail `loop()` function
             *
             * @author Michael Ochmann <miko@maschinendeck.org>
             * @since 0.0.1
             */
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

                    //this enables the use of backspace to delete characters
                    if (car == 127 && this->message.length() > 0) {
                        Serial.print("\e[1D");
                        Serial.print(' ');
                        Serial.print("\e[1D");
                        this->message.remove(this->message.length() - 1);
                        continue;
                    }
                    /*
                        this makes shure, the users input gets printed back to
                        his serial console
                    */
                    if (isAscii(car))
                        Serial.print(car);
                    // Check if user ended the line
                    if (car == '\r') {
                        Serial.print("\r\n");
                        commandComplete = true;
                        /*
                            If there are more data on the line, drop a \n, if it is
                            there. Some terminals may send both, giving
                            an extra lineend, if we do not drop it.
                        */
                        if (Serial.available() && Serial.peek() == '\n')
                            Serial.read();
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
				#if not defined ST_FLAG_NOHELP && not defined ST_FLAG_NOBUILTIN
				if (command.first() == "help") {
					this->printCommands();
					return;
				}
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

            /**
             * Parses a string and returns a pair containing the first word and
             * the rest string
             *
             * @author Michael Ochmann <miko@maschinendeck.org>
             * @since 0.0.1
             */
            static Pair<String, String> ParseCommand(String& message) {
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

                return Pair<String, String>(move(keyword), move(message));
            }

            static String ParseArgument(String message) {
                String keyword = "";
                for (auto& car : message) {
                    if (car == '"')
                        break;
                    keyword += car;
                }
                if (keyword != "")
                    message.remove(0, keyword.length());
                message.trim();
                int msg_len = message.length();
                if (msg_len > 0) {
                    message.remove(0, 1);
                    message.remove(msg_len - 2);
                }

                return message;
            }
    };

}
