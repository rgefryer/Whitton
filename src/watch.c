#include "watch.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_24_bold;
static GFont s_res_bitham_42_medium_numbers;
static GFont s_res_bitham_30_black;
static GFont s_res_roboto_condensed_21;
static TextLayer *s_destination;
static TextLayer *s_stopwatch;
static TextLayer *s_remaining;
static InverterLayer *s_inverterlayer_1;
static TextLayer *s_ahead;
static InverterLayer *s_inverterlayer_2;
static InverterLayer *s_inverterlayer_3;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_fullscreen(s_window, true);
  
  s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_res_bitham_42_medium_numbers = fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS);
  s_res_bitham_30_black = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  s_res_roboto_condensed_21 = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  // s_destination
  s_destination = text_layer_create(GRect(4, 66, 138, 50));
  text_layer_set_background_color(s_destination, GColorClear);
  text_layer_set_text_color(s_destination, GColorWhite);
  text_layer_set_text(s_destination, "Press select to start, 7:50 latest");
  text_layer_set_font(s_destination, s_res_gothic_24_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_destination);
  
  // s_stopwatch
  s_stopwatch = text_layer_create(GRect(1, 0, 140, 52));
  text_layer_set_background_color(s_stopwatch, GColorBlack);
  text_layer_set_text_color(s_stopwatch, GColorWhite);
  text_layer_set_text(s_stopwatch, "00:00");
  text_layer_set_text_alignment(s_stopwatch, GTextAlignmentCenter);
  text_layer_set_font(s_stopwatch, s_res_bitham_42_medium_numbers);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_stopwatch);
  
  // s_remaining
  s_remaining = text_layer_create(GRect(62, 124, 76, 38));
  text_layer_set_background_color(s_remaining, GColorBlack);
  text_layer_set_text_color(s_remaining, GColorWhite);
  text_layer_set_text(s_remaining, "(99)");
  text_layer_set_text_alignment(s_remaining, GTextAlignmentCenter);
  text_layer_set_font(s_remaining, s_res_bitham_30_black);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_remaining);
  
  // s_inverterlayer_1
  s_inverterlayer_1 = inverter_layer_create(GRect(4, 59, 137, 3));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_inverterlayer_1);
  
  // s_ahead
  s_ahead = text_layer_create(GRect(5, 130, 42, 26));
  text_layer_set_background_color(s_ahead, GColorBlack);
  text_layer_set_text_color(s_ahead, GColorWhite);
  text_layer_set_text(s_ahead, "(99)");
  text_layer_set_text_alignment(s_ahead, GTextAlignmentCenter);
  text_layer_set_font(s_ahead, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_ahead);
  
  // s_inverterlayer_2
  s_inverterlayer_2 = inverter_layer_create(GRect(2, 121, 53, 3));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_inverterlayer_2);
  
  // s_inverterlayer_3
  s_inverterlayer_3 = inverter_layer_create(GRect(52, 124, 3, 34));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_inverterlayer_3);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_destination);
  text_layer_destroy(s_stopwatch);
  text_layer_destroy(s_remaining);
  inverter_layer_destroy(s_inverterlayer_1);
  text_layer_destroy(s_ahead);
  inverter_layer_destroy(s_inverterlayer_2);
  inverter_layer_destroy(s_inverterlayer_3);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

struct Location {
  char * name;
  int hours;
  int minutes;
};
  
int mins(struct Location *l) {
  return 60 * l->hours + l->minutes;
}

/*
3:40 to braithwaite
be there by 11:30
set off by 7:50
50 mins driving
15 minutes faff
leave wigtone at 6:45 latest

Next steps
- Test that "ahead" indicator works
- Test behaviour at end of the ride
- Fix up the places array.
*/
  
struct Location places[] = {
  { "Start", 0, 13 },
  { "Ambleside", 0, 26 },
  { "Troutbeck", 0, 33 },
  { "Kirkstone Summit", 1, 7 },
  { "Bottom Matterdale end", 1, 27 },
  { "A66", 1, 51 },
  { "Keswick", 2, 19 },
  { "Seatoller", 2, 49 },
  { "Honister Summit", 3, 5 },
  { "Buttermere", 3, 28 },
  { "Newlands Summit", 3, 39 },
  { "Braithwaite", 3, 54 },
  { "Whinlatter Summit", 4, 12 },
  { "Low Lorton", 4, 24 },
  { "Top Fangs Brow", 4, 53 },
  { "Lamplugh", 4, 59 },
  { "Ennerdale Bridge", 5, 18 },
  { "Top Cold Fell", 5, 39 },
  { "Calder Bridge", 6, 1 },
  { "Gosforth", 6, 9 },
  { "Eskdale", 6, 30 },
  { "Top Hardknott", 7, 5 },
  { "Top Wrynose", 7, 22 },
  { "Onto new route", 7, 33 },
  { "Grasmere", 8, 12 }};

int num_places = sizeof places / sizeof places[0];

static time_t target_time;  // Time to arrive at next destination
static time_t start_time; // Time that we set off
static int dest_index = 0; // Index of next destination
static int ahead_by; // Number of seconds we are ahead of target
static int start_offset = 0;  // Number of minutes offset from the start

// Write a 1/2 digit integer into a string
void itoa(char *buf, int val, bool leading_zero) {
  if (val > 99) {
    strcpy(buf, "++");
    return;
  }
  if (val < -99) {
    strcpy(buf, "--");
    return;
  }
  bool negative = false;
  if (val < 0) {
    negative = true;
    val = 0 - val;
    *(buf++) = '(';
  }
  
  if (leading_zero && (val < 10)) {
    *(buf++) = '0';
  }
  else if (val >= 10) {
    *buf = '0';
    while (val >= 10) {
      val -= 10;
      (*buf)++;
    }
    buf++;
  }

  *(buf++) = '0' + val;
  
  if (negative) {
    *(buf++) = ')';
  } 
  
  *buf = 0;  // Null-terminate the string!
}

// Format a time in minutes as hh:mm
void mintext(char *buf_pos, int mins, bool sign) {
  
  if (sign) {
    if (mins < 0) {
      *(buf_pos++) = '-';
      mins = 0 - mins;
    }
    else {
      *(buf_pos++) = '+';      
    }
  }

  int hrs = 0;
  while (mins >= 60) {
    mins -= 60;
    hrs++;
  }
  
  itoa(buf_pos, hrs, true);
  buf_pos += 2;
  *(buf_pos++) = ':';
  itoa(buf_pos, mins, true);
}

static time_t next_eat;
static void update_time() {

  // Do nothing if we've finished
  if (dest_index >= num_places) {
    return;
  }
  
  // Display how mamy minutes we've been going for
  time_t now = time(NULL);
  
  if (now > next_eat) {
    next_eat += 15 * 60;
    vibes_long_pulse();
  }
  
  int minutes_active = (now - start_time) / 60;
  static char buff1[10];
  mintext(buff1, minutes_active, false);
  text_layer_set_text(s_stopwatch, buff1);

  // Display the current destination
  text_layer_set_text(s_destination, places[dest_index].name);

  // Display how many minutes to the current destination
  int mins_to_target = (59 + target_time - now) / 60;
  static char buff2[10];
  itoa(buff2, mins_to_target, false);
  text_layer_set_text(s_remaining, buff2);

  // Display how many minutes to the current destination
  int mins_ahead = (ahead_by) / 60;
  if (target_time < now) {
    mins_ahead = (ahead_by - (now - target_time)) / 60;
  }
  static char buff3[10];
  itoa(buff3, mins_ahead, false);
  text_layer_set_text(s_ahead, buff3);
}  

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static time_t last_click;
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  
  time_t now = time(NULL);
  
  // If this is the first click, start the timer (event every minute)
  if (dest_index == 0) {
    start_time = now;
    last_click = now;
    next_eat = now + 15 * 60;;
    start_offset = mins(&(places[0]));
    
    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);  
  }

  // If the last click was less than 10 secs ago, ignore.
  if ((dest_index != 0) && ((now - last_click) < 10)) {
    return;
  }

  // Store the time of this click
  last_click = now;

  // If this is the first click, update the ahead/behind count.
  if (dest_index == 0) {
    ahead_by = 0;
    target_time = now;
  }
  
  // Advance the current destination, reset the current countdown.
  if (dest_index == (num_places - 1)) {
    ahead_by += (target_time - now);
    target_time = now;
    vibes_short_pulse();  
    update_time();                 // Final update at the end of the ride
  }  
  
  dest_index++;
  if (dest_index >= num_places) {  // Do nothing if we're off the list of places
    return;  
  }
  
  // Haptic notification that the click was registered
  vibes_short_pulse();  
   
  ahead_by += (target_time - now);
  target_time = now + 60 * (mins(&(places[dest_index])) - mins(&(places[dest_index-1])));

  // Make sure the time is displayed from the start
  update_time();  
}

void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
}

void show_watch(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_set_click_config_provider(s_window, (ClickConfigProvider) config_provider);

  window_stack_push(s_window, true);
}

void hide_watch(void) {
  window_stack_remove(s_window, true);
}
