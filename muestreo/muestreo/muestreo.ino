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

// #define num_samples 512           // Global number of samples.
#define num_samples 128           // Global number of samples.
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
  oled.setCursor(0,0);
  oled.setTextColor(WHITE);
  oled.println("OSC. v1.0");
  oled.display();
  Serial.begin(115200);
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
  for (int j=0; j<adc_num;  j++) {
    Serial.println(adc_addr[j]);
  }
  // int osc = analogRead(A0);
  // Serial.println(osc);
  // delayMicroseconds(1);
  // analog_buffer[127] = (int)(63-((float)osc/(float)1024)*63);
  // (int)(63-((float)adc_addr[j-1]/(float)1024)*63)
  for (int j = 1; j < 128; j++) {
    // analog_buffer[j-1] = analog_buffer[j];
    // oled.drawPixel(j-1, analog_buffer[j-1], WHITE);
    adc_addr[j-1] = adc_addr[j];
    oled.drawPixel(j-1, (int)(63-((float)adc_addr[j-1]/(float)1024)*63), WHITE);
  }
  oled.display();
  oled.clearDisplay();
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
