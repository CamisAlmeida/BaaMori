#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

// CONFIGURÇÃO DO WIFI
const char* ssid = "....";
const char* password = "....";

// DIFINIÇÃO DO BOTÃO PARA COLETA DE DADOS
#define button 2
bool coletaAtiva = false;
bool ultimoEstadoBotao = LOW;
int contadorTestes = 0;

// CONFIGURAÇÃO DO SENSOR
const int MPU = 0x68;
WebServer server(80);

float AccX, AccY, AccZ, GyrX, GyrY, GyrZ;
float nowAccX, nowAccY, nowAccZ, nowGyrX, nowGyrY, nowGyrZ;

unsigned long prevTime = 0;
unsigned long nowTime = 0;
long interval = 100;

String currentData = "";
String csvContent = "Time(ms),Teste,AccX(g),ΔAccX,AccY(g),ΔAccY,AccZ(g),ΔAccZ,GyrX(d/s),ΔGyrX,GyrY(d/s),ΔGyrY,GyrZ(d/s),ΔGyrZ\n";


// FUNÇÃO PARA COLETA DE DADOS
void datacollection(float* accX, float* accY, float* accZ,
                    float* gyrX, float* gyrY, float* gyrZ) {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  *accX = Wire.read() << 8 | Wire.read();
  *accY = Wire.read() << 8 | Wire.read();
  *accZ = Wire.read() << 8 | Wire.read();
  *gyrX = Wire.read() << 8 | Wire.read();
  *gyrY = Wire.read() << 8 | Wire.read();
  *gyrZ = Wire.read() << 8 | Wire.read();
}

// MONTAR CSV DOS DADOS
void updateCurrentData() {
  currentData = String(nowTime) + "\t"+ String(contadorTestes) + "\t";
  currentData += String(AccX / 2048.0, 2) + "\t" + String(nowAccX - AccX, 0) + "\t";
  currentData += String(AccY / 2048.0, 2) + "\t" + String(nowAccY - AccY, 0) + "\t";
  currentData += String(AccZ / 2048.0, 2) + "\t" + String(nowAccZ - AccZ, 0) + "\t";
  currentData += String(GyrX / 16.4, 2) + "\t" + String(nowGyrX - GyrX, 0) + "\t";
  currentData += String(GyrY / 16.4, 2) + "\t" + String(nowGyrY - GyrY, 0) + "\t";
  currentData += String(GyrZ / 16.4, 2) + "\t" + String(nowGyrZ - GyrZ, 0);

  // CSV com vírgulas
  String csvLine = String(nowTime) + "," + String(contadorTestes) + ",";
  csvLine += String(AccX / 2048.0, 2) + "," + String(nowAccX - AccX, 0) + ",";
  csvLine += String(AccY / 2048.0, 2) + "," + String(nowAccY - AccY, 0) + ",";
  csvLine += String(AccZ / 2048.0, 2) + "," + String(nowAccZ - AccZ, 0) + ",";
  csvLine += String(GyrX / 16.4, 2) + "," + String(nowGyrX - GyrX, 0) + ",";
  csvLine += String(GyrY / 16.4, 2) + "," + String(nowGyrY - GyrY, 0) + ",";
  csvLine += String(GyrZ / 16.4, 2) + "," + String(nowGyrZ - GyrZ, 0);
  csvContent += csvLine + "\n";
}



void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Conectando");

  // CONFIGURAÇÃO WIFI
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.println(WiFi.localIP());

  // CONFIGURAÇÃO DO SERVIDOR WEB PARA ENVIO DOS DADOS
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Dados MPU6050</title>";
    html += "<style>"
            "body { font-family: Arial, sans-serif; margin: 0; padding: 0; }"
            "#dados { padding: 20px; white-space: pre-wrap; }"
            ".top-right { position: absolute; top: 10px; right: 10px; }"
            "</style></head><body>";

    html += "<button class='top-right' onclick=\"window.location='/csv'\">Baixar CSV</button>";
    html += "<h2 style='padding: 20px;'>Dados MPU6050</h2>";
    html += "<pre id='dados'>Carregando dados...</pre>";

    html += "<script>"
            "const dadosEl = document.getElementById('dados');"
            "function atualiza() {"
            "fetch('/data').then(resp => resp.text()).then(txt => {"
            "if (dadosEl.innerText === 'Carregando dados...') dadosEl.innerText = '';"
            "dadosEl.innerText += txt + '\\n';"
            "dadosEl.scrollTop = dadosEl.scrollHeight;"
            "});"
            "}"
            "setInterval(atualiza, 500); atualiza();"
            "</script>";
    html += "</body></html>";

    server.send(200, "text/html", html);
  });

  server.on("/data", []() {
    server.send(200, "text/plain", currentData);
  });

  server.on("/csv", []() {
    server.sendHeader("Content-Type", "text/csv");
    server.sendHeader("Content-Disposition", "attachment; filename=\"dados.csv\"");
    server.send(200, "text/csv", csvContent);
  });

  server.begin();

  // INICIALIZAÇÃO DO SENSOR
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU);
  Wire.write(0x1B);
  Wire.write(0b00011000);
  Wire.endTransmission();

  Wire.beginTransmission(MPU);
  Wire.write(0x1C);
  Wire.write(0b00011000);
  Wire.endTransmission();

  prevTime = millis();
  Serial.println("Setup concluído!");
}

void loop() {
  nowTime = millis();

  // Leitura do botão com detecção de transição
  bool estadoBotaoAtual = digitalRead(button);
  Serial.println(estadoBotaoAtual);

  if (ultimoEstadoBotao == HIGH && estadoBotaoAtual == LOW) {
  coletaAtiva = !coletaAtiva;

  if (coletaAtiva) {
    contadorTestes++;
    Serial.println("Coleta iniciada. Teste #" + String(contadorTestes));
  } else {
    Serial.println("Coleta pausada.");
    currentData = "";  // Limpa último dado para não repetir
  }
  delay(200); // debounce
}

  ultimoEstadoBotao = estadoBotaoAtual;

  datacollection(&AccX, &AccY, &AccZ, &GyrX, &GyrY, &GyrZ);

  // Só coleta dados se estiver ativo
  if (coletaAtiva && nowTime - prevTime >= interval) {
    datacollection(&nowAccX, &nowAccY, &nowAccZ, &nowGyrX, &nowGyrY, &nowGyrZ);
    updateCurrentData();
    prevTime = millis();
  }

  server.handleClient();
}