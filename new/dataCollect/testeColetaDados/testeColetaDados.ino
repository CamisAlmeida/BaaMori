#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

#define MPU_ADDR 0x69
#define BTN_PIN 4   // botão usado apenas no modo queda

enum Sessao { 
  SESSAO_NORMAL, 
  SESSAO_QUEDA 
};

// Sessao sessaoAtual = SESSAO_NORMAL;
Sessao sessaoAtual = SESSAO_QUEDA;

bool quedaGravando = false;
bool lastBtn = HIGH;

static int fall_id = 0;   // <<< ID DA QUEDA >>>

int16_t ax, ay, az, gx, gy, gz;

void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = Wire.read()<<8 | Wire.read();
  ay = Wire.read()<<8 | Wire.read();
  az = Wire.read()<<8 | Wire.read();
  gx = Wire.read()<<8 | Wire.read();
  gy = Wire.read()<<8 | Wire.read();
  gz = Wire.read()<<8 | Wire.read();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(9, 10);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // acorda MPU
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  WiFi.softAP("ESP32_DATA", "12345678");
  webSocket.begin();

  Serial.println("\n========= SISTEMA INICIADO =========");
  Serial.println("Modo selecionado: QUEDA");
  Serial.println("=====================================");
}

void loop() {
  webSocket.loop();
  readMPU();

  if (sessaoAtual == SESSAO_NORMAL) {

    String msg =
      String(ax) + "," + String(ay) + "," + String(az) + "," +
      String(gx) + "," + String(gy) + "," + String(gz) + ",0," + 
      String(fall_id);

    webSocket.broadcastTXT(msg);
    delay(50);
    return;
  }

  //
  // ===========================
  //      MODO QUEDA
  // ===========================
  //

  bool btn = digitalRead(BTN_PIN);

  if (lastBtn == HIGH && btn == LOW) {
    quedaGravando = !quedaGravando;

    if (quedaGravando) {
      Serial.println("INICIO DA QUEDA");
    } else {
      Serial.println("FIM DA QUEDA");
      fall_id++;  // incrementa o id da queda
    }

    delay(250);
  }

  lastBtn = btn;

  if (quedaGravando) {
    String msg =
      String(ax) + "," + String(ay) + "," + String(az) + "," +
      String(gx) + "," + String(gy) + "," + String(gz) + ",1," +
      String(fall_id);  // envia id da queda

    webSocket.broadcastTXT(msg);
  }

  delay(10);
}
