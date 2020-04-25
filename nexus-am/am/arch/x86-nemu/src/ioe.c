#include <am.h>
#include <x86.h>

#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT)-boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int i;
  int cp_bytes = w;
  if (_screen.width - x < w)
	cp_bytes = _screen.width - x;
  // int cp_bytes = sizeof(uint32_t) * tmp;
  for (i = 0; i < h; i++) {
    // fb[i] = i;
	memcpy(fb + (y + i) * _screen.width + x, pixels, cp_bytes);
	pixels += w;
  }
}

void _draw_sync() {
}

int _read_key() {
  int ret = _KEY_NONE;
  if (inb(I8042_STATUS_PORT)) {
    ret = inl(I8042_DATA_PORT);
  }
  return ret;
}
