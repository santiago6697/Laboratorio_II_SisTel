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

#define num_samples 4096          // Global number of samples.
uint16_t adc_addr[num_samples];   // ADC point to the address of continuous sampling.
uint16_t adc_num = num_samples;   // Sampling number [1, 65535].
uint8_t adc_clk_div = 8;          // Recommended value issued by API reference.

int i = 0;
unsigned long start = 0;
unsigned long end = 0;
float sampling_rate = 0;
unsigned long tim = 0;
unsigned long tot = 0;
int scale = 2;

void setup() {
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_Address);
  oled.clearDisplay();
  Serial.begin(115200);
}

void loop() {
  wifi_set_opmode(NULL_MODE);
  system_soft_wdt_stop();
  ets_intr_lock( ); // Close interrupt.
  // Measuring sampling rate.
  noInterrupts();
  start = micros();

  system_adc_read_fast(adc_addr, adc_num, adc_clk_div);
  
  end = micros() - start;
  // Yet, just the first heuristic approach.
  sampling_rate = 4.545454545;
  interrupts();
  ets_intr_unlock(); // Open interrupt.
  system_soft_wdt_restart();
  int max_voltage_read = 0;
  int min_voltage_read = 0;
  float mean_period = 0;
  bool read_value = false;
  int cnt = 0;
  int peak_position = 0;
  int valley_position = 0;
  float period = 0.0;
  for (int j = 0; j < num_samples; j++) {
    if (((adc_addr[j-2] <= adc_addr[j-1]) && (adc_addr[j-1] >= adc_addr[j])) && (read_value == false) && (adc_addr[j-1] > 512)) {
      max_voltage_read = adc_addr[j-1];
      peak_position = j-1;
      read_value = true;
    }
    if (((adc_addr[j-2] >= adc_addr[j-1]) && (adc_addr[j-1] <= adc_addr[j])) && (read_value == true) && (adc_addr[j-1] < 512)) {
      min_voltage_read = adc_addr[j-1];
      valley_position = j-1;
      period = 2 * (valley_position - peak_position);
      if (period >= 550){
        scale = 16;
      } else if ((period >= 220) && (period < 550)) {
        scale = 8;
      } else if (period < 220) {
        scale = 2;
      }
      read_value = false;
    }
    // adc_addr[j-1] = adc_addr[j];
    // Heuristic approach.
    if ((j-1 > 4) && (j-1 < 126)) oled.drawPixel(j-1, fix_value(adc_addr[int(j-1)*scale]), WHITE);
  }
  float voltage = parse_voltage(max_voltage_read, min_voltage_read);
  period = period * sampling_rate;
  float frequency = parse_frequency(period);
  draw_gui(voltage, frequency);
  oled.display();
  oled.clearDisplay();
  // delayMicroseconds(1000);
  // draw_gui();
//  if (i == 100) {
//       /* Serial.print("Sampling rate: ");
//       Serial.println(total / 100);
//       Serial.print("It lasted: ");
//       Serial.println(tim / 100); */
//      i = 0;
//      tim = 0;
//      total = 0;
//  }
}

void draw_gui(float voltage, float frequency) {
  oled.setCursor(2,1);
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  char prefix = ' ';
  if (frequency >= 1000) {
    frequency = frequency/1000;
    prefix = 'k';
    oled.println("F:"+(String)frequency+prefix+"Hz");
  } else {
    oled.println("F:"+(String)frequency+"Hz");
  }
  oled.setCursor(64,1);
  oled.println("V:"+(String)voltage+"vpp");
  oled.drawRect(2, 10, 124, 52, WHITE);
  for (int i = 2; i < 123; i++) {
    if (i%4 == 0){
       oled.drawLine(i, 36, i+2, 36, WHITE);
    }
  }
  oled.display();
}

float parse_voltage (int max_voltage_read, int min_voltage_read) {
  float max_voltage = 3.3;
  int peak_to_peak_voltage = max_voltage_read - min_voltage_read;
  return max_voltage*((float)peak_to_peak_voltage/(float)1024);
}

float parse_frequency (float period) {
  float frequency = 0;
  if (period > 0) frequency = 1.0/period;
  frequency = frequency * 1000000;
  return frequency;
}

int fix_value (int value) {
  return (int)(60-((float)value/(float)1024)*46);
}
