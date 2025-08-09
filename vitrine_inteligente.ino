/*
  Vitrine Inteligente - ESP32
  - Sensor PIR: detectar presença -> aciona iluminação por TEMPO_ON segundos
  - Sensor DHT22: leitura de temperatura e umidade, exibida no OLED
  - Display OLED I2C (SSD1306 / U8g2)
  - Relé para controlar a lâmpada interna da vitrine
  - Logs básicos em EEPROM (ciclo circular) a cada LOG_INTERVAL_MS

  Autor: Matheus Pereira Martins (projeto acadêmico)
  Data: 2025
*/

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <EEPROM.h>

// ===== CONFIGURAÇÃO =====
#define PIN_PIR        14    // GPIO do sensor PIR
#define PIN_DHT        15    // GPIO do DHT22
#define PIN_RELAY      26    // GPIO para sinal do relé (LOW/HIGH depende do módulo)
#define DHT_TYPE       DHT22

// I2C (OLED)
#define I2C_SDA        21
#define I2C_SCL        22

// Tempos (ms)
const unsigned long TEMPO_ON_MS = 40UL * 1000UL;    // 40 segundos de iluminação
const unsigned long LOG_INTERVAL_MS = 5UL * 60UL * 1000UL; // 5 minutos por log

// EEPROM (simples)
const int EEPROM_SIZE = 512;    // ajuste conforme necessidade
const int MAX_LOGS = 20;        // número de registros que cabem (simplificado)
const int LOG_START_ADDR = 0;   // endereço inicial

// ===== OBJETOS =====
DHT dht(PIN_DHT, DHT_TYPE);
// U8g2 para display 128x64 I2C, change constructor if needed
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ===== ESTADO =====
volatile bool pirTriggered = false;
unsigned long lastMotionMillis = 0;
unsigned long lightOnUntil = 0;
unsigned long lastLogMillis = 0;

// EEPROM circular index
uint8_t eeprom_next_index = 0;

// ===== FUNÇÕES AUX =====
void IRAM_ATTR pirISR() {
  pirTriggered = true;
}

void saveLogToEEPROM(float temp, float hum) {
  // Estrutura simples: [index][temp*100 (int16)][hum*100 (int16)][timestampMinutes (uint32 low precision)]
  int addr = LOG_START_ADDR + (eeprom_next_index * 10); // 10 bytes por registro (ajuste se necessário)
  if (addr + 10 > EEPROM_SIZE) {
    // wrap around
    eeprom_next_index = 0;
    addr = LOG_START_ADDR;
  }

  int16_t t100 = (int16_t)round(temp * 100.0f);
  int16_t h100 = (int16_t)round(hum * 100.0f);
  uint32_t tmins = (uint32_t)(millis() / 60000UL); // minutos desde boot (não é tempo real)

  EEPROM.write(addr + 0, eeprom_next_index); // índice
  EEPROM.write(addr + 1, (uint8_t)(t100 >> 8));
  EEPROM.write(addr + 2, (uint8_t)(t100 & 0xFF));
  EEPROM.write(addr + 3, (uint8_t)(h100 >> 8));
  EEPROM.write(addr + 4, (uint8_t)(h100 & 0xFF));
  EEPROM.write(addr + 5, (uint8_t)((tmins >> 24) & 0xFF));
  EEPROM.write(addr + 6, (uint8_t)((tmins >> 16) & 0xFF));
  EEPROM.write(addr + 7, (uint8_t)((tmins >> 8) & 0xFF));
  EEPROM.write(addr + 8, (uint8_t)(tmins & 0xFF));
  EEPROM.write(addr + 9, 0xAA); // marker

  EEPROM.commit();
  eeprom_next_index++;
  if (LOG_START_ADDR + (eeprom_next_index * 10) >= EEPROM_SIZE) {
    eeprom_next_index = 0;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Inicializa I2C (opcional; U8g2 faz isso mas é bom deixar claro)
  Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(PIN_PIR, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PIR), pirISR, RISING);

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW); // assumindo LOW = desligado (verificar módulo relé)

  dht.begin();
  u8g2.begin();

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);

  lastLogMillis = millis();
  Serial.println("Vitrine Inteligente inicializado");
}

void loop() {
  unsigned long now = millis();

  // === Detecta movimento ===
  if (pirTriggered) {
    // debounce simples: ler novamente
    delay(50);
    if (digitalRead(PIN_PIR) == HIGH) {
      lastMotionMillis = now;
      lightOnUntil = now + TEMPO_ON_MS;
      // liga relé
      digitalWrite(PIN_RELAY, HIGH);
      Serial.println("Movimento detectado -> LUZ LIGADA");
    }
    pirTriggered = false;
  }

  // === Desliga se tempo esgotado ===
  if (lightOnUntil != 0 && now > lightOnUntil) {
    digitalWrite(PIN_RELAY, LOW);
    lightOnUntil = 0;
    Serial.println("Tempo esgotado -> LUZ DESLIGADA");
  }

  // === Leitura sensor DHT e display atualização periódica ===
  static unsigned long lastDisplayMillis = 0;
  const unsigned long DISPLAY_REFRESH_MS = 2000;
  if (now - lastDisplayMillis >= DISPLAY_REFRESH_MS) {
    lastDisplayMillis = now;

    float t = NAN, h = NAN;
    // Ler DHT com tentativas simples
    t = dht.readTemperature();
    h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("Falha leitura DHT");
    } else {
      Serial.printf("Temp: %.2f C  Hum: %.2f %%\n", t, h);
    }

    // Atualiza OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 12, "Vitrine Inteligente");
    char buf[64];
    if (!isnan(t) && !isnan(h)) {
      sprintf(buf, "Temp: %.1f C  Umid: %.0f%%", t, h);
    } else {
      sprintf(buf, "Temp: ---   Umid: ---");
    }
    u8g2.drawStr(0, 28, buf);

    // Estado luz
    if (digitalRead(PIN_RELAY) == HIGH) {
      u8g2.drawStr(0, 44, "Luz: ON");
    } else {
      u8g2.drawStr(0, 44, "Luz: OFF");
    }

    // Tempo restante
    if (lightOnUntil != 0) {
      long secRem = (lightOnUntil - now) / 1000;
      sprintf(buf, "Rem: %lds", secRem);
      u8g2.drawStr(70, 44, buf);
    }

    u8g2.sendBuffer();
  }

  // === Registro em EEPROM a cada LOG_INTERVAL_MS ===
  if (now - lastLogMillis >= LOG_INTERVAL_MS) {
    lastLogMillis = now;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      saveLogToEEPROM(t, h);
      Serial.printf("Log salvo -> T: %.2f H: %.2f\n", t, h);
    } else {
      Serial.println("Log ignorado (falha DHT)");
    }
  }

  // Pequena espera para liberar CPU
  delay(10);
}
