#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  char key_state = (key & 0x8000) ? 'd' : 'u';
  if (key_state=='d') { key ^= 0x8000; }
  if (key!=_KEY_NONE)
  { snprintf(buf, len, "k%c %s\n", key_state, keyname[key]); }
  else { snprintf(buf, len, "t %d\n", _uptime()); }

  return strlen(buf);
  // return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  strncpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  // int ly = (offset + len) / sizeof(uint32_t) / _screen.width;
  offset /= sizeof(uint32_t);
  int wx = offset % _screen.width;
  int wy = offset / _screen.width;

  _draw_rect(buf, wx, wy, len/sizeof(uint32_t), 1);
  /*
  if (ly==wy)
  { _draw_rect(buf, wx, wy, len / sizeof(uint32_t), 1); }
  else {
    int tmp = _screen.width - wx;
	if (ly-wy==1) {
	  _draw_rect(buf, wx, wy, tmp, 1);
	  _draw_rect(buf+tmp*sizeof(uint32_t), 0, ly, len/sizeof(uint32_t)-tmp, 1);
	}
	else {
	  _draw_rect(buf, wx, wy, tmp, 1);
	  int ny = ly-wy-1;
	  _draw_rect(buf+tmp*sizeof(uint32_t), 0, wy+1, _screen.width, ny);
	  _draw_rect(buf+tmp*sizeof(uint32_t)+ny*_screen.width*sizeof(uint32_t), 0, ly, len/sizeof(uint32_t)-tmp-ny*_screen.width, 1);
	}
  }
  */
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH: %d\nHEIGHT: %d\n", _screen.width, _screen.height);
  Log("dispinfo: %s", dispinfo);
}
