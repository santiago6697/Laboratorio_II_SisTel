#include <user_interface.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_Address 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Adafruit_SSD1306 oled(1);

ADC_MODE(ADC_TOUT);

#define num_samples 2048          // Global number of samples.
uint16_t adc_addr[num_samples];   // ADC point to the address of continuous sampling.
uint16_t adc_num = num_samples;   // Sampling number [1, 65535].
uint8_t adc_clk_div = 8;          // Recommended value issued by API reference.

int i = 0;
unsigned long start = 0;
unsigned long total = 0;
unsigned long tim = 0;

int analog_buffer[127];

void setup() {
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_Address);
  oled.clearDisplay();
  Serial.begin(115200);
  // attachInterrupt(digitalPinToInterrupt(0), blink, CHANGE);
}

void loop() {
  wifi_set_opmode(NULL_MODE);
  system_soft_wdt_stop();
  ets_intr_lock( ); //close interrupt
  noInterrupts();

  start = micros();

  // Serial.println(system_adc_read());
  system_adc_read_fast(adc_addr, adc_num, adc_clk_div);

  unsigned int tot = micros() - start;

  interrupts();
  ets_intr_unlock(); //open interrupt
  system_soft_wdt_restart();

  tim += tot;
  total += num_samples * 1000000.0 / tot;
  i++;
  // for (int j=0; j<adc_num;  j++) {
  //   Serial.println(adc_addr[j]);
  // }
  // int osc = analogRead(A0);
  // Serial.println(osc);
  // delayMicroseconds(1);
  // analog_buffer[127] = (int)(63-((float)osc/(float)1024)*63);
  // (int)(63-((float)adc_addr[j-1]/(float)1024)*63)
  // draw_gui();
  // Serial.println(adc_addr[sizeof(adc_addr)-1]);-
  int max_voltage_read = 0;
  int min_voltage_read = 1024;
  int max_read_position = 0;
  int min_read_position = 0;
  for (int j = 4; j < 126; j++) {
    // min_voltage_read = adc_addr[j-1];
    if ( adc_addr[j-1] > max_voltage_read ) {
      max_voltage_read = adc_addr[j-1];
      // max_read_position = j-1;
      // Serial.println("Max: "+(String)max_voltage_read);
      // Serial.println("Max: "+(String)max_read_position);
    }
    if ( adc_addr[j-1] < min_voltage_read ) {
      min_voltage_read = adc_addr[j-1];
      // min_read_position = j-1;
      // Serial.println("Min: "+(String)min_voltage_read);
      // Serial.println("Min: "+(String)min_read_position);
    }
    adc_addr[j-1] = adc_addr[j];
    oled.drawPixel(j-1, (int)(60-((float)adc_addr[j-1]/(float)1024)*46), WHITE);
    delayMicroseconds(500);
  }
  float voltage = normalize_voltage(max_voltage_read, min_voltage_read);
  draw_gui(voltage);
  oled.display();
  // delayMicroseconds(1000);
  oled.clearDisplay();
  // delayMicroseconds(1000);
  // draw_gui();
  if (i == 100) {
       /* Serial.print("Sampling rate: ");
       Serial.println(total / 100);
       Serial.print("It lasted: ");
       Serial.println(tim / 100); */
      i = 0;
      tim = 0;
      total = 0;
  }
}

void draw_gui(float voltage) {
  int frequency = 0;
  // int voltage = 0;
  oled.setCursor(2,1);
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  oled.println("F: "+(String)frequency+"Hz");
  oled.setCursor(64,1);
  oled.println("V: "+(String)voltage+"vpp");
  oled.drawRect(2, 10, 124, 52, WHITE);
  for (int i = 2; i < 123; i++) {
    if (i%4 == 0){
       oled.drawLine(i, 36, i+2, 36, WHITE);
    }
  }
  oled.display();
}

float normalize_voltage (int max_voltage_read, int min_voltage_read) {
  float max_voltage = 3.3;
  int peak_to_peak_voltage = max_voltage_read - min_voltage_read;
  return max_voltage*((float)peak_to_peak_voltage/(float)1024);
}
