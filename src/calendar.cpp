#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NtpClient.h>
#include <WiFi.h>
#include <calendar.h>
#include <configs.h>

#define REQ_DATA                                                               \
  "{\"client_id\":\"" CRONIFY_CLIENT_ID "\",\"client_secret\":\"" CRONIFY_CLIENT_SECRET        \
  "\",\"grant_type\":\"refresh_token\",\"refresh_token\":\"" CRONIFY_REFRESH_TOKEN     \
  "\"}"

char *copy_string(const char *str) {
  char *cloned_str = (char *)malloc(strlen(str) + 1);
  strcpy(cloned_str, str);
  return cloned_str;
}

char *refresh_token() {
  HTTPClient http;

  http.begin("https://api-de.cronofy.com/oauth/token");
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(REQ_DATA);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Refresh token response: ");
    Serial.println(httpResponseCode);
    Serial.println(response);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response.c_str());

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      doc.clear();
    } else {
      char *cloned_token = copy_string(doc["access_token"]);
      // const char *new_token = doc["access_token"];
      // char *cloned_token = (char *)malloc(strlen(new_token) + 1);
      // strcpy(cloned_token, new_token);

      http.end();
      doc.clear();
      return cloned_token;
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
    Serial.println(http.getString());
  }

  http.end();
  return NULL;
}

String format_date(NTPClient *time, int day_offset = 0) {
  unsigned long epoch = time->getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epoch);

  return String(1900 + ptm->tm_year) + "-" + String(1 + ptm->tm_mon) + "-" +
         String(ptm->tm_mday + day_offset);
}

JsonDocument get_events(NTPClient *time, const char *access_token) {
  HTTPClient http;
  JsonDocument doc;

  // update your URL with the date.
  String query_url = "https://api-de.cronofy.com/v1/events?tzid=Etc/UTC&from=" +
                     format_date(time) + "&to=" + format_date(time, 1);

  Serial.print("Querying events: ");
  Serial.println(query_url);

  http.begin(query_url.c_str());
  http.addHeader("Authorization", "Bearer " + String(access_token));

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(access_token));

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Get events response: ");
    Serial.println(httpResponseCode);
    Serial.println(response);

    DeserializationError error = deserializeJson(doc, response.c_str());

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);

    Serial.println(http.getString());
  }

  http.end();
  return doc;
}

#define MAX_EVENTS 12
Event events[MAX_EVENTS];
unsigned long last_synced_millis = -1;

void fillSchedule(NTPClient *time) {
  char *access_token = refresh_token();
  JsonDocument doc = get_events(time, access_token);

  JsonArray jsonArray = doc["events"];
  int current_hour = time->getHours();
  int current_minute = time->getMinutes();

  int i = 0;
  for (JsonVariant v : jsonArray) {
    struct Event event;
    const char *start = v["start"]; // "2024-01-10T18:00:00Z"
    const char *end = v["end"];     // "2024-01-10T18:00:00Z"
    const char *title_ptr = v["summary"];

    int start_hour = atoi(&start[11]) + TIMEZONE_UTF_OFFSET_HOURS;
    int start_minute = atoi(&start[14]);

    String title = String(title_ptr);
    bool is_past_event =
        start_hour < current_hour ||
        (start_hour == current_hour && start_minute < current_minute);

    if (String(start).indexOf("T") == -1 || title.indexOf("OOO") != -1 ||
        is_past_event) {
      continue;
    }

    event.title = title;
    event.start_hour = start_hour;
    event.start_minute = start_minute;
    event.end_hour = atoi(&end[11]);
    event.end_minute = atoi(&end[14]);

    Serial.println("Event:");
    Serial.println(event.title);
    events[i] = event;

    if (i == MAX_EVENTS - 1) {
      break;
    }
    i++;
  }

  Serial.println("Events:");
  Serial.println(sizeof(events) / sizeof(Event));

  last_synced_millis = millis();

  doc.clear();
  free(access_token);
}

int next_event_idx = 0;
int current_event_idx = -1;

Event *get_next_event(NTPClient *time) {
  if (millis() - last_synced_millis > 1000 * 60 * 60) {
    fillSchedule(time);
  }

  Event *next_event = &events[next_event_idx];
  if (next_event->start_hour >= time->getHours() &&
      next_event->start_minute >= time->getMinutes()) {
    current_event_idx = next_event_idx;
    next_event_idx++;
  }

  return next_event;
}

String formatTimeTillEvent(Event *event, NTPClient *time) {
  int currentHour = time->getHours();
  int currentMinute = time->getMinutes();

  // Calculate the time difference till start of the event
  int hourDiff = event->start_hour - currentHour;
  int minuteDiff = event->start_minute - currentMinute;

  // If minute difference is negative, subtract one hour
  if (minuteDiff < 0) {
    hourDiff--;
    minuteDiff += 60;
  }

  // If the event is less than an hour away
  if (hourDiff == 0) {
    return "in " + String(minuteDiff) + "m";
  }
  // If the event is more than an hour away
  else {
    return "in " + String(hourDiff) + "h";
  }
}
