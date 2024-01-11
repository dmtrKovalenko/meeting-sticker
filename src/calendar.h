#include <Arduino.h>
#include <NTPClient.h>

struct Event {
  int start_hour;
  int start_minute;
  int end_hour;
  int end_minute;
  String title;
};

void fillSchedule(NTPClient *time);
Event *get_next_event(NTPClient *time);
String formatTimeTillEvent(Event *event, NTPClient *time);
