#include <pebble.h>
#include <watch.h>
  

void handle_init(void) {
  show_watch();
}

void handle_deinit(void) {
  hide_watch();
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
