#include <user_interface.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
#define OLED_Address 0x3C
Adafruit_SSD1306 oled(1);

ADC_MODE(ADC_TOUT);

#define num_samples 512           // Global number of samples.
uint16_t adc_addr[num_samples];   // ADC point of address.
uint16_t adc_num = num_samples;   // Sampling number [1, 65535].
uint8_t adc_clk_div = 8;          // Recommended value issued by API reference.

int i = 0;
unsigned long start = 0;
unsigned long total = 0;
unsigned long tim = 0;

void setup() {
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_Address);
  oled.clearDisplay();
  Serial.begin(115200);
}

void loop() {
  wifi_set_opmode(NULL_MODE);
  system_soft_wdt_stop();
  ets_intr_lock( ); //close interrupt
  noInterrupts();

  start = micros();
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
      oled.drawPixel(j, (int)(31-((float)analogRead(A0)/(float)1024)*31), WHITE);
      oled.display();
  }

  if (i == 100) {
    oled.clearDisplay();
      // Serial.print("Sampling rate: ");
      // Serial.println(total / 100);
      // Serial.print("It lasted: ");
      // Serial.println(tim / 100);

      i = 0;
      tim = 0;
      total = 0;
  }
}
