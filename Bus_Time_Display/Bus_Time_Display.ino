String message = "              ";
String bus_names[] = {"","","","","","","",""};
int bus_times[] = {0,0,0,0,0,0,0,0};

// 0 == good
// 1 == connection failed
// 2 == json failed to parse
int bus_error = 1;

// Update the message string given the bus_names, bus_times, and bus_error
void set_message() {
  message = "";

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
    if(bus_times[i] == -1) break;
    if(i > 0) message.concat("  ");
    message.concat(bus_names[i] + "-" + String(bus_times[i]) + " min");
  }

  // Pad the message with spaces in the case
  if(message.length() < 14) { 
    for(int i = message.length(); i < 14; i++) {
      message.concat(' ');
    }
  }
}

void setup() {
  setup_display();
  setup_wifi();
  delay(1000);
}

int hold_time = 3000; // ms to hold at start and end of message
int scroll_speed = 40; // ms per column
int refresh_time = 30000; // ms to display between refreshes

void loop() {
  // Get the bus predictions over wifi and update bus_names, bus_times, and bus_error
  get_predictions();
  // Write the result to the message string to be displayed
  set_message();

  uint32_t hold_init = millis();
  uint32_t reset_time = millis();
  uint32_t hold_stop = millis();
  // Loop for refresh_time milliseconds but don't cut off the scroll animation
  while(millis() - hold_init < refresh_time || !(millis() < hold_stop && millis() - reset_time > hold_time)) {
    // Number of columns to offset the message 
    int col_offset = 0;
    if(message.length() > 14 || (message.length() == 14 && message[13] != ' ')) {
      // Scroll because message is too long
      if(millis() < hold_stop) {
        col_offset = (7 * message.length()) - 95;
      } else if(millis() - reset_time > hold_time) {
        col_offset = (millis() - reset_time - hold_time) / scroll_speed;
        if(col_offset >= (7 * message.length()) - 95) {
          hold_stop = millis() + hold_time;
          reset_time = millis() + hold_time;
        }
      }
    } else if(millis() - hold_init >= refresh_time) {
      break;
    }
    // Draw the message to the display with the calculated col_offset
    draw_display(col_offset);
  }
}
