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
  term->add("add", &addInt, "adds to integers");
}

void loop() {
  term->loop();
}
