#include "animations/animation.h"
#include "animations/loading.h"
#include "animations/sun.h"
#include "configs.h"

#include <Arduino.h>
#include <NTPClient.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <WiFiUdp.h>
#include <calendar.h>
#include <timer.h>
#include <wifi_utils.h>

#define WIDTH 128
#define HEIGHT 64

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org",
                     TIMEZONE_UTF_OFFSET_HOURS * 3600, 60 * 60 * 1000);
U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI display(U8G2_R0, 20, 21, -1);

XbmAnimation<72> sun_animation(72, 28, 24, sun_frames);
XbmAnimation<512> loading_animation(512, 15, 64, loading_frames);

void setup(void) {
  Serial.begin(115200);
  if (!display.begin()) {
    Serial.println("Display init failed");
    while (1)
      ;
  }

  display.clearDisplay();

  loading_animation.render(WIDTH / 2 - loading_animation.buf_size(), 10,
                           &display, true);

  // scan_networks();
  setup_wifi();
  timeClient.begin();
  timeClient.forceUpdate();

  fillSchedule(&timeClient);
}

char *formatNumber(int num) {
  static char buffer[50]; // Use a sufficiently large buffer for your use case
  // Formats the number into the buffer as a string
  sprintf(buffer, "%02d", num);

  // Return the C-string
  return buffer;
}

Event *next_event;
String till_event_text;
String title_marquee_text;

int marquee_pos = 0;
Timer marquee_timer = Timer(1000, false);
Timer sync_event_timer = Timer(1000 * 60); // 1 minute

void loop(void) {
  timeClient.update();
  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();

  if (!next_event || sync_event_timer.tick()) {
    Event *event = get_next_event(&timeClient);

    if (event != next_event) {
      next_event = event;
      till_event_text = formatTimeTillEvent(next_event, &timeClient);

      title_marquee_text = event->title;
      marquee_pos = 0;
    }
  }

  display.clearBuffer();                    // clear the internal memory
  display.setFont(u8g2_font_logisoso42_tn); // choose a suitable font

  display.setFontPosTop();
  display.drawStr(15, 0, formatNumber(hours));
  display.drawStr(65, 0, ":");
  display.drawStr(75, 0, formatNumber(minutes));

  display.setFont(u8g2_font_crox2h_tf);
  display.drawUTF8(0, 17, "-2°");
  display.drawUTF8(0, 3, "-7°");

  if (till_event_text.length() > 0) {
    if (marquee_timer.tick()) {
      title_marquee_text = next_event->title.substring(marquee_pos);

      if (marquee_pos > next_event->title.length()) {
        marquee_pos = 0;
      } else {
        marquee_pos++;
      }
    }

    // display.setFont(u8g2_font_moosenooks_tr);
    display.setFont(u8g2_font_crox3t_tf);
    String raw_text = (till_event_text + ": " + title_marquee_text);
    // 20 is a max length of the text fitting string with the moosenooks font
    raw_text.remove(18);
    display.drawStr(0, 48, raw_text.c_str());
    raw_text.clear();
  }

  display.sendBuffer();
}
