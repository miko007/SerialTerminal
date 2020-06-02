# SerialTerminal
– a simple Arduino library to incorporate a serial terminal to your project

## Usage
Store a global pointer to an instance of `maschinendeck::SerialTerminal`, initialize it with your desired baudrate and commands in the `setup()` function and call its `maschinendeck::SerialTerminal::loop()` function in the general `loop()` function, like in the example below.

```c++
#include <SerialTerminal.hpp>

maschinendeck::SerialTerminal* term;

void addInt(String opts) {
	maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
	Serial.print(operands.first());
	Serial.print(" + ");
	Serial.print(operands.second());
	Serial.print(" = ");
	Serial.print(operands.first().toInt() + operands.second().toInt());
	Serial.print('\n');
}

void setup() {
	term = new maschinendeck::SerialTerminal(38400);
	term->add("add", &addInt, "adds two integers");
}

void loop() {
	term->loop();
}
```
### Flags
You can change the behavior of the library, by setting some flags:

* `ST_FLAG_NOHELP` - removes the help screen normally printed on startup
* `ST_FLAG_NOBUILTIN` - removes all default commands, if you do not need them
* `ST_FLAG_NOPROMPT` - do not print a prompt

You simply define them before the include of `SerialTerminal.h`:

```c++
#define ST_FLAG_NOHELP
#define ST_FLAG_NOBUILTIN

#include <SerialTerminal.hpp>

maschinendeck::SerialTerminal* term;

...

```
