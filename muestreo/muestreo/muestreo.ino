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
unsigned long end = 0;
unsigned long total = 0;
unsigned long tim = 0;
unsigned long tot = 0;

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

  system_adc_read_fast(adc_addr, adc_num, adc_clk_div);

  interrupts();
  ets_intr_unlock(); // Open interrupt.
  system_soft_wdt_restart();

  // tim += tot;
  // total += num_samples * 1000000.0 / tot;
  // i++;
  // int osc = analogRead(A0);
  // Serial.println(osc);
  // delayMicroseconds(1);
  // analog_buffer[127] = (int)(63-((float)osc/(float)1024)*63);
  // (int)(63-((float)adc_addr[j-1]/(float)1024)*63)
  // draw_gui();
  // Serial.println(adc_addr[sizeof(adc_addr)-1]);-
  // int max_voltage_read = 0;
  // int min_voltage_read = 1024;
  int max_voltage_read = 0;
  int min_voltage_read = 0;
  // int peak_delta = 0;
  // int valley_delta = 0;
  int mean_total = 0;
  start = 0;
  end = 0;
  bool read_value = false;
  // tot = 0;
  total = 0;
  Serial.println("Period: "+(String)mean_total);
  // DEPRECATED: This kind of approach requires a lot of optimization and is not suitable for abrupt changes in waveform such as seen in square waves.
  /* int zero_count = 0;
  for (int j=0; j<adc_num;  j++) {
     if (adc_addr[j-1] >= 487 && adc_addr[j-1] <= 537 ) {
      // Serial.println(adc_addr[j-1]);
      switch (zero_count) {
        case 0:
          zero_count = 1;
          start = micros();
          // Serial.println("First Zero!");
        break;
        case 3:
          tot = micros() - start;
          Serial.println("reached");
          break;
        default:
          zero_count += 1;
          break;
      }
    }
  } */
  int cnt = 0;
  for (int j = 4; j < 126; j++) {
    // min_voltage_read = adc_addr[j-1];
    /*if ( adc_addr[j-1] > max_voltage_read ) {
      max_voltage_read = adc_addr[j-1];
    }
    if ( adc_addr[j-1] < min_voltage_read ) {
      min_voltage_read = adc_addr[j-1];
    }*/
    // adc_addr[j-1] points to current position.
    // if (((adc_addr[j-2] <= adc_addr[j-1]) && (adc_addr[j-1] >= adc_addr[j])) && (read_value == false)) Serial.println("OK");
    if (((adc_addr[j-2] <= adc_addr[j-1]) && (adc_addr[j-1] >= adc_addr[j])) && (read_value == false) && (adc_addr[j-1] > 512)) {
      max_voltage_read = adc_addr[j-1];
      // Add dotted line drawer.
      // start = micros();
      start = micros();
      // Serial.println("Period start: "+(String)start);
      read_value = true;
      oled.drawLine(j-1, 0, j-1, 63, WHITE);
    }
    if (((adc_addr[j-2] >= adc_addr[j-1]) && (adc_addr[j-1] <= adc_addr[j])) && (read_value == true) && (adc_addr[j-1] < 512)) {
    //if ((adc_addr[j-2] >= min_voltage_read <= adc_addr[j]) && read_value) {
      min_voltage_read = adc_addr[j-1];
      // end = start - micros();
      end = micros() - start;
      // Serial.println("Period end: "+(String)end);
      // Add dotted line drawer.
      total = total + 2*end;
      // Just in case it causes some sort of misreading.
      start = 0;
      end = 0;
      read_value = false;
      oled.drawLine(j-1, 20, j-1, 63, WHITE);
    }
    adc_addr[j-1] = adc_addr[j];
    oled.drawPixel(j-1, (int)(60-((float)adc_addr[j-1]/(float)1024)*46), WHITE);
    // delayMicroseconds(500);
    cnt++;
  }
  // Serial.println("Counter: "+(String)cnt);
  float voltage = parse_voltage(max_voltage_read, min_voltage_read);
  // Serial.println("Period: "+(String)total);
  // mean_total = total/cnt;
  mean_total = total;
  Serial.println("Period: "+(String)mean_total);
  // Serial.println("Period: "+(String)mean_total);
  float frequency = parse_frequency((float)mean_total);
  // Serial.println("Start: "+(String)start);
  // Serial.println("Tot: "+(String)tot);
  draw_gui(voltage, frequency);
  oled.display();
  // delayMicroseconds(1000);
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
  // int frequency = 0;
  // int voltage = 0;
  oled.setCursor(2,1);
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  char prefix = ' ';
  // frequency = 1/frequency;
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
  if (period > 0) frequency = 1.0/(float)period;
  frequency = frequency * 1000000;
  // Serial.println("Freq: "+(String)frequency);
  return frequency;
}
