String message = "HELLO WORLD THIS IS A REALLY LONG TEST ";

void setup() {
  setup_display();
  setup_wifi();
  delay(1000);
}

void loop() {
  int col_offset = (millis()/50) % ((7 * message.length()) - 97);
  draw_display(col_offset);
}
