#include <U8g2lib.h>

template <size_t FRAME_LEN> class XbmAnimation {
private:
  unsigned long last_millis;
  int frame = 0;

  const unsigned int size;
  const unsigned int frame_count;
  const unsigned int frame_size;
  const byte (*frames)[FRAME_LEN];

public:
  XbmAnimation(const unsigned int _size, const unsigned int _frame_count,
               const unsigned int _frame_size, const byte (*_frames)[FRAME_LEN])
      : frames(_frames), size(_size), frame_count(_frame_count),
        frame_size(_frame_size) {}

  void render(u8g2_uint_t x, u8g2_uint_t y, U8G2 *display, bool flush = false) {
    unsigned long now = millis();

    if (now - last_millis >= 42) {
      // Perform animation update with frames here.
      // Remember to use pgm_read_byte_near() to read individual bytes.
      display->drawXBM(x, y, frame_size, frame_size, frames[frame]);
      if (flush) {
        display->sendBuffer();
      }

      frame = (frame + 1) % frame_count;
      last_millis = millis();
    }
  }

  const unsigned int buf_size() { return size; }
};
