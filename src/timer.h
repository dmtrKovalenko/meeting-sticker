#include <Arduino.h>

class Timer {
private:
  unsigned long period;
  unsigned long lastTick;
  bool ticked = false;

public:
  Timer(unsigned long period, bool first_tick = false) {
    this->period = period;
    this->lastTick = millis();
    this->ticked = first_tick;
  }

  bool tick() {
    unsigned long now = millis();
    if (!this->ticked) {
      this->ticked = true;
      lastTick = now;
      return false;
    }

    if (now - lastTick >= period) {
      lastTick = now;
      return true;
    } else {
      return false;
    }
  }
};
