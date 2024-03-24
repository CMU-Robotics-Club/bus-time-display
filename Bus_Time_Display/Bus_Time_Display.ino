String message = "              ";
String bus_names[][8] = {{"","","","","","","",""}, {"","","","","","","",""}};
int bus_times[][8] = {{0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}};

// 0 == good
// 1 == connection failed
// 2 == json failed to parse
int bus_error = 1;

// Update the message string given the bus_names, bus_times, and bus_error
void set_message(bool display_eastbound) {
  message = "";
  if(display_eastbound) {
    message = "EAST ";
  } else {
    message = "WEST ";
  }

  // If there is an error, set the message to that so it is visible
  if(bus_error != 0) {
    if(bus_error == 1) {
      message = "Connection error";
    } else if(bus_error == 2) {
      message = "JSON parse error";
    } else {
      message = "Unknown error";
    }
    return;
  }

  // If there was no error, display the bus names and times
  for(int i = 0; i < 8; i++) {
    if(bus_times[display_eastbound][i] == -1) break;
    if(i > 0) message.concat("  ");
    message.concat(bus_names[display_eastbound][i] + "-" + String(bus_times[display_eastbound][i]) + "min");
  }

  // Pad the message with spaces in the case
  if(message.length() < 14) { 
    for(int i = message.length(); i < 14; i++) {
      message.concat(' ');
    }
  }
}

enum {
  REFRESH,
  IDLE_START,
  SCROLLING,
  IDLE_STOP,
} system_state;
bool displaying_eastbound = true;

void setup() {
  setup_display();
  setup_wifi();
  system_state = REFRESH;
  delay(1000);
}

int hold_time = 2000; // ms to hold at start and end of message
int scroll_speed = 30; // ms per column
int display_number = 1; // Number of times to display each message before refreshing
int min_refresh_time = 30000;

uint32_t last_refresh_time = 0;
uint32_t t_hold = 0;
int display_count = 0;

void loop() {
  uint32_t t = millis();
  int col_offset = 0;
  switch(system_state) {
    case REFRESH:
      // Get the bus predictions over wifi and update bus_names, bus_times, and bus_error
      get_predictions(true);
      get_predictions(false);
      get_time();
      
      // Write the result to the message string to be displayed
      displaying_eastbound = true;
      set_message(displaying_eastbound);
      system_state = IDLE_START;
      t_hold = millis();
      break;
    case IDLE_START:
      if(t >= t_hold + hold_time) {
        t_hold = t;
        system_state = SCROLLING;
      } else {
        draw_display(0);
      }
      break;
    case SCROLLING:
      col_offset = (t - t_hold) / scroll_speed;
      if(col_offset >= (7 * message.length()) - 95) {
        t_hold = t;
        system_state = IDLE_STOP;
      } else {
        draw_display(col_offset);
      }
      break;
    case IDLE_STOP:
      if(t >= t_hold + hold_time) {
        t_hold = t;
        if(displaying_eastbound) {
          system_state = IDLE_START;
          displaying_eastbound = false;
          set_message(displaying_eastbound);
        } else {
          display_count++;
          if(display_count >= display_number && t >= last_refresh_time + min_refresh_time) {
            system_state = REFRESH;
            display_count = 0;
          } else {
            system_state = IDLE_START;
            displaying_eastbound = true;
            set_message(displaying_eastbound);
          }
        }
      } else {
        draw_display((7 * message.length()) - 95);
      }
      break;
  };
}
