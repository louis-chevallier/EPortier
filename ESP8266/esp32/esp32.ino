#include <vector>
#include <algorithm>
#include "util.h"
//#include "Arduino.h"
#include <driver/dac.h>
#include <driver/adc.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#define DAC_CH1 25
#define DAC_CH2 26
const int SAMPLES=10000;
uint32_t val[SAMPLES];

#include <driver/i2s.h>

// I2S
#define I2S_SAMPLE_RATE (32000)
#define ADC_INPUT (ADC1_CHANNEL_4) //pin 32
#define I2S_DMA_BUF_LEN (64)

// The 4 high bits are the channel, and the data is inverted
size_t bytes_read;
uint16_t buffer[I2S_DMA_BUF_LEN] = {0};

unsigned long lastTimePrinted;
unsigned long loopTime = 0;

void i2sInit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate =  I2S_SAMPLE_RATE,              // The format of the signal using ADC_BUILT_IN
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = I2S_DMA_BUF_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_adc_mode(ADC_UNIT_1, ADC_INPUT);
  i2s_adc_enable(I2S_NUM_0);
  adc1_config_channel_atten(ADC_INPUT, ADC_ATTEN_DB_11);
}








void setup() {
  Serial.begin(115200); //Begin Serial at 115200 Baud
  delay(1000);
  esp_wifi_deinit();
  esp_bt_controller_deinit();
  Serial.print("starting\n");
  EKO();
  dac_output_enable(DAC_CHANNEL_1);
  auto begin(micros());

  EKO();
  i2sInit();
  EKO();
  /*
  unsigned long startMicros = ESP.getCycleCount();

  i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 0);

  unsigned long stopMicros = ESP.getCycleCount();

  loopTime = stopMicros - startMicros;
  EKOX(bytes_read);
  
  for(int i=0; i<SAMPLES; i++)
  {
    //int v;
    auto v1 = adc1_get_raw(ADC1_CHANNEL_1);
    auto v2 = adc1_get_raw(ADC1_CHANNEL_2);
    auto v3 = adc1_get_raw(ADC1_CHANNEL_3);
    auto v4 = adc1_get_raw(ADC1_CHANNEL_4);
    
    //auto v = analogRead(25);
    val[i] = v1;    
    //dacWrite(DAC_CH1, v*2);
    dac_output_voltage((dac_channel_t)DAC_CHANNEL_1, (uint8_t)(v1+v2+v3+v4));
  }
  EKOX(1000000. / ((micros() - begin) / SAMPLES));
  */
  delay(500);

}

long br = 0;

void loop() {
  delay(1); 
  unsigned long startMicros = ESP.getCycleCount();

  i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 0);
  //EKOX(bytes_read);
  unsigned long stopMicros = ESP.getCycleCount();
  br += (bytes_read/2);
  loopTime = stopMicros - startMicros;

  if (millis() - lastTimePrinted >= 1000) {
    EKOX(buffer[0] & 0x0FFF);
    EKOX(loopTime);
    lastTimePrinted = millis();
    EKOX(br);
    br = 0;
  }

}
