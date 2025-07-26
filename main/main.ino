// Nome do sistemas - BaaMori (Baa-san + Mamori = “proteger”)
// Inspirado no anime jiisan baasan wakagaeru (Grandpa and Grandma Turn Young Again)

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

// CONFIGURÇÃO DO WIFI
const char* ssid = "Camis";
const char* password = "281Camis";

// DIFINIÇÃO DO BOTÃO PARA COLETA DE DADOS
#define button 2

bool coletaAtiva = false;
bool ultimoEstadoBotao = LOW;
int contadorTestes = 0;

// CONFIGURAÇÃO DO SENSOR
const int MPU = 0x68;
WebServer server(80);

// ISFALL  == false - Não houve queda
// ISFALL == true - houve queda

bool isFall = false;
long timeButton = 0;
long prevTimeButton = 0;
long intervalButton = 10000;
// DADOS
float AccX, AccY, AccZ, GyrX, GyrY, GyrZ;
float nowAccX, nowAccY, nowAccZ, nowGyrX, nowGyrY, nowGyrZ;

struct data {
  float AccX, AccY, AccZ;
  float GyrX, GyrY, GyrZ;
  float nowAccX, nowAccY, nowAccZ;
  float nowGyrX, nowGyrY, nowGyrZ;
};


const int lenDataset = 5;
data dataset[lenDataset];
int posData = 0;
int qtdData = 0;

unsigned long prevTime = 0;
long interval = 100;
unsigned long nowTime = 0;


// FUNÇÃO PARA COLETA DE DADOS
String currentData = "";
String csvContent = "Time(ms),Teste,AccX(g),ΔAccX,AccY(g),ΔAccY,AccZ(g),ΔAccZ,GyrX(d/s),ΔGyrX,GyrY(d/s),ΔGyrY,GyrZ(d/s),ΔGyrZ\n";

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
  currentData = String(nowTime) + "\t" + String(contadorTestes) + "\t";
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

// Printa os dados no monitor serial
void printDataSerial() {
  Serial.print(nowTime);
  Serial.print("\t");

  Serial.print(AccX / 2048, 2);
  Serial.print("\t");
  Serial.print(nowAccX - AccX, 0);
  Serial.print("\t");

  Serial.print(AccY / 2048, 2);
  Serial.print("\t");
  Serial.print(nowAccY - AccY, 0);
  Serial.print("\t");

  Serial.print(AccZ / 2048, 2);
  Serial.print("\t");
  Serial.print(nowAccZ - AccZ, 0);
  Serial.print("\t");

  Serial.print(GyrX / 16.4, 2);
  Serial.print("\t");
  Serial.print(nowGyrX - GyrX, 0);
  Serial.print("\t");

  Serial.print(GyrY / 16.4, 2);
  Serial.print("\t");
  Serial.print(nowGyrY - GyrY, 0);
  Serial.print("\t");

  Serial.print(GyrZ / 16.4, 2);
  Serial.print("\t");
  Serial.println(nowGyrZ - GyrZ, 0);
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

  ultimoEstadoBotao = estadoBotaoAtual;

  // COLETA DE DADOS
  datacollection(&AccX, &AccY, &AccZ, &GyrX, &GyrY, &GyrZ);
  datacollection(&nowAccX, &nowAccY, &nowAccZ, &nowGyrX, &nowGyrY, &nowGyrZ);
  updateCurrentData();

  // ADICIONAR DADOS NO DATASET (BUFFER)
  dataset[posData].AccX = AccX;
  dataset[posData].AccY = AccY;
  dataset[posData].AccZ = AccZ;
  dataset[posData].GyrX = GyrX;
  dataset[posData].GyrY = GyrY;
  dataset[posData].GyrZ = GyrZ;
  dataset[posData].nowAccX = nowAccX;
  dataset[posData].nowAccY = nowAccY;
  dataset[posData].nowAccZ = nowAccZ;
  dataset[posData].nowGyrX = nowGyrX;
  dataset[posData].nowGyrY = nowGyrY;
  dataset[posData].nowGyrZ = nowGyrZ;

  /*
  ARVORE DE DECISÃO
  |--- GyrX(d/s)2 <= 3908.78
  |   |--- GyrX(d/s)3 <= 3900.97
  |   |   |--- AccZ(g)1 <= 15.73
  |   |   |   |--- AccZ(g)2 <= 0.04
  |   |   |   |   |--- class: 0
  |   |   |   |--- AccZ(g)2 >  0.04
  |   |   |   |   |--- class: 1
  |   |   |--- AccZ(g)1 >  15.73
  ...
  |   |   |   |   |   |--- class: 0
  |--- GyrX(d/s)2 >  3908.78
  |   |--- class: 1
  */

  if (qtdData >= 5) {
    if (dataset[2].GyrX <= 3908.78) {
      if (dataset[3].GyrX <= 3900.97) {
        if (dataset[1].AccZ <= 15.73) {
          if (dataset[2].AccZ <= 0.04) {
            isFall = false;
          } else {
            isFall = true;
          }
        } else {
          isFall = false;
        }
      }
    } 
    // else {
    //   isFall = true;
    // }
  }

  if (isFall == true) {
    digitalWrite(button, HIGH);
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
    Serial.println("🆘QUEDAAAA!!!!🆘");
  }

  // prevTimeButton = millis();

  // while (prevTimeButton - timeButton <= intervalButton) {
  //   Serial.println("🆘QUEDAAAA!!!!🆘");
  //   digitalWrite(button, HIGH);
  //   prevTimeButton = millis();
  // }

  posData++;
  posData = posData % lenDataset;
  qtdData++;

  server.handleClient();
}
