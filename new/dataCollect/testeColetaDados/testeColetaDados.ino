#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <TFT_eSPI.h>

// -------- CONFIGURAÇÕES --------
WebSocketsServer webSocket = WebSocketsServer(81);
const int MPU_ADDR = 0x68;
const int INT_PIN = 5;

// Display TFT integrado
TFT_eSPI tft = TFT_eSPI();

// Variáveis MPU6050
volatile bool motionInterrupt = false;
volatile bool zeroInterrupt = false;
int16_t ax, ay, az, gx, gy, gz;

// -------- ESTADOS --------
enum Estado { NORMAL, QUEDA };
Estado estado = NORMAL;

// -------- INTERRUPÇÃO ----------
void IRAM_ATTR handleMPUInterrupt() {
  uint8_t intStatus;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3A); // INT_STATUS
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 1, true);
  intStatus = Wire.read();

  if (intStatus & 0x40) motionInterrupt = true; // Motion
  if (intStatus & 0x20) zeroInterrupt = true;   // Zero motion
}

// -------- CONFIGURAÇÃO --------
void setup() {
  Serial.begin(115200);

  Wire.begin(9, 10);  // SDA=9, SCL=10
  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), handleMPUInterrupt, FALLING);

  // Inicializa MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0); // Acorda
  Wire.endTransmission(true);

  // Configura Motion detection
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1F); // MOT_THR
  Wire.write(15);   // menos sensível
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x20); // MOT_DUR
  Wire.write(10);
  Wire.endTransmission(true);

  // Habilita interrupções Motion + Zero Motion
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x38); // INT_ENABLE
  Wire.write(0x60);
  Wire.endTransmission(true);

  // Inicializa TFT
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0,0);
  tft.println("MPU6050 Iniciado");

  // WiFi AP
  WiFi.softAP("ESP32_DATA", "12345678");
  tft.println("AP criado: ESP32_DATA");
  tft.println("IP: 192.168.4.1");

  webSocket.begin();
  Serial.println("WebSocket iniciado porta 81");
}

// -------- LEITURA MPU --------
void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}

// -------- LOOP PRINCIPAL --------
void loop() {
  webSocket.loop();
  readMPU();

  static unsigned long lastNormalTime = 0;

  switch (estado) {
    case NORMAL:
      if (motionInterrupt) {
        motionInterrupt = false;
        estado = QUEDA;
        Serial.println("🔥 Queda detectada!");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0);
        tft.println("🔥 Queda detectada!");
      }

      // Coleta normal cada 2s
      if (millis() - lastNormalTime > 2000) {
        String msg = String(ax) + "," + String(ay) + "," + String(az) + "," +
                     String(gx) + "," + String(gy) + "," + String(gz);
        webSocket.broadcastTXT(msg);
        lastNormalTime = millis();
      }
      break;

    case QUEDA:
      // Envia dados de queda continuamente
      {
        String msg = String(ax) + "," + String(ay) + "," + String(az) + "," +
                     String(gx) + "," + String(gy) + "," + String(gz);
        webSocket.broadcastTXT(msg);
      }

      // Checa zero motion para terminar a queda
      if (zeroInterrupt) {
        zeroInterrupt = false;
        estado = NORMAL;
        Serial.println("📌 Fim da queda");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0);
        tft.println("📌 Fim da queda");
      }
      break;
  }
}
