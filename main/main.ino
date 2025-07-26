// Nome do sistemas - BaaMori (Baa-san + Mamori = “proteger”)
// Inspirado no anime jiisan baasan wakagaeru (Grandpa and Grandma Turn Young Again)

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

// CONFIGURÇÃO DO WIFI
const char* ssid = "Camis";
const char* password = "281Camis";

// DIFINIÇÃO DO BOTÃO PARA COLETA DE DADOS
#define button 4
#define alert 2

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

// Valores media, max e min dos valores coletados
struct statistics {
  // Acelerômetro - Eixo X
  float mean_AccX, min_AccX, max_AccX;
  float mean_dAccX, min_dAccX, max_dAccX;

  // Acelerômetro - Eixo Y
  float mean_AccY, min_AccY, max_AccY;
  float mean_dAccY, min_dAccY, max_dAccY;

  // Acelerômetro - Eixo Z
  float mean_AccZ, min_AccZ, max_AccZ;
  float mean_dAccZ, min_dAccZ, max_dAccZ;

  // Giroscópio - Eixo X
  float mean_GyrX, min_GyrX, max_GyrX;
  float mean_dGyrX, min_dGyrX, max_dGyrX;

  // Giroscópio - Eixo Y
  float mean_GyrY, min_GyrY, max_GyrY;
  float mean_dGyrY, min_dGyrY, max_dGyrY;

  // Giroscópio - Eixo Z
  float mean_GyrZ, min_GyrZ, max_GyrZ;
  float mean_dGyrZ, min_dGyrZ, max_dGyrZ;
};

statistics stats = { 0 };

const int lenDataset = 5;
data dataset[lenDataset];
int posData = 0;
int qtdData = 0;

unsigned long prevTime = 0;
long interval = 100;
unsigned long nowTime = 0;
data dados;

// Função para calcular estatisticas
// ds == dataset
statistics calcStatistics(data ds[], int lenDataset, int posData, int qtdData) {
  statistics stats = { 0 };  // zera todos os campos

  // Inicializa mínimos e máximos
  stats.min_AccX = stats.min_AccY = stats.min_AccZ = 99999;
  stats.min_dAccX = stats.min_dAccY = stats.min_dAccZ = 99999;
  stats.min_GyrX = stats.min_GyrY = stats.min_GyrZ = 99999;
  stats.min_dGyrX = stats.min_dGyrY = stats.min_dGyrZ = 99999;

  stats.max_AccX = stats.max_AccY = stats.max_AccZ = -99999;
  stats.max_dAccX = stats.max_dAccY = stats.max_dAccZ = -99999;
  stats.max_GyrX = stats.max_GyrY = stats.max_GyrZ = -99999;
  stats.max_dGyrX = stats.max_dGyrY = stats.max_dGyrZ = -99999;

  for (int i = 0; i < qtdData; i++) {
    int index = (posData + i) % lenDataset;

    float dAccX = ds[index].nowAccX - ds[index].AccX;
    float dAccY = ds[index].nowAccY - ds[index].AccY;
    float dAccZ = ds[index].nowAccZ - ds[index].AccZ;
    float dGyrX = ds[index].nowGyrX - ds[index].GyrX;
    float dGyrY = ds[index].nowGyrY - ds[index].GyrY;
    float dGyrZ = ds[index].nowGyrZ - ds[index].GyrZ;

    // Soma para média
    stats.mean_AccX += ds[index].AccX;
    stats.mean_AccY += ds[index].AccY;
    stats.mean_AccZ += ds[index].AccZ;
    stats.mean_dAccX += dAccX;
    stats.mean_dAccY += dAccY;
    stats.mean_dAccZ += dAccZ;

    stats.mean_GyrX += ds[index].GyrX;
    stats.mean_GyrY += ds[index].GyrY;
    stats.mean_GyrZ += ds[index].GyrZ;
    stats.mean_dGyrX += dGyrX;
    stats.mean_dGyrY += dGyrY;
    stats.mean_dGyrZ += dGyrZ;

    // Atualiza mínimos
    stats.min_AccX = min(stats.min_AccX, ds[index].AccX);
    stats.min_AccY = min(stats.min_AccY, ds[index].AccY);
    stats.min_AccZ = min(stats.min_AccZ, ds[index].AccZ);
    stats.min_dAccX = min(stats.min_dAccX, dAccX);
    stats.min_dAccY = min(stats.min_dAccY, dAccY);
    stats.min_dAccZ = min(stats.min_dAccZ, dAccZ);

    stats.min_GyrX = min(stats.min_GyrX, ds[index].GyrX);
    stats.min_GyrY = min(stats.min_GyrY, ds[index].GyrY);
    stats.min_GyrZ = min(stats.min_GyrZ, ds[index].GyrZ);
    stats.min_dGyrX = min(stats.min_dGyrX, dGyrX);
    stats.min_dGyrY = min(stats.min_dGyrY, dGyrY);
    stats.min_dGyrZ = min(stats.min_dGyrZ, dGyrZ);

    // Atualiza máximos
    stats.max_AccX = max(stats.max_AccX, ds[index].AccX);
    stats.max_AccY = max(stats.max_AccY, ds[index].AccY);
    stats.max_AccZ = max(stats.max_AccZ, ds[index].AccZ);
    stats.max_dAccX = max(stats.max_dAccX, dAccX);
    stats.max_dAccY = max(stats.max_dAccY, dAccY);
    stats.max_dAccZ = max(stats.max_dAccZ, dAccZ);

    stats.max_GyrX = max(stats.max_GyrX, ds[index].GyrX);
    stats.max_GyrY = max(stats.max_GyrY, ds[index].GyrY);
    stats.max_GyrZ = max(stats.max_GyrZ, ds[index].GyrZ);
    stats.max_dGyrX = max(stats.max_dGyrX, dGyrX);
    stats.max_dGyrY = max(stats.max_dGyrY, dGyrY);
    stats.max_dGyrZ = max(stats.max_dGyrZ, dGyrZ);
  }

  // Finaliza médias
  stats.mean_AccX /= qtdData;
  stats.mean_AccY /= qtdData;
  stats.mean_AccZ /= qtdData;
  stats.mean_dAccX /= qtdData;
  stats.mean_dAccY /= qtdData;
  stats.mean_dAccZ /= qtdData;

  stats.mean_GyrX /= qtdData;
  stats.mean_GyrY /= qtdData;
  stats.mean_GyrZ /= qtdData;
  stats.mean_dGyrX /= qtdData;
  stats.mean_dGyrY /= qtdData;
  stats.mean_dGyrZ /= qtdData;

  return stats;
}


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

  pinMode(alert, OUTPUT);
  pinMode(button, INPUT);

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
  // Atualizar posição do buffer:
  posData++;
  posData = posData % lenDataset;
  if (qtdData < lenDataset) { qtdData++; }

  if (qtdData >= lenDataset) {
    stats = calcStatistics(dataset, lenDataset, posData, qtdData);
  }

  /*
  ARVORE DE DECISÃO
|--- min_ΔAccX <= -61.50
|   |--- max_GyrY(d/s) <= 3978.05
|   |   |--- class: 0
|   |--- max_GyrY(d/s) >  3978.05
|   |   |--- min_ΔAccY <= -45.50
|   |   |   |--- class: 1
|   |   |--- min_ΔAccY >  -45.50
|   |   |   |--- class: 0
|--- min_ΔAccX >  -61.50
|   |--- min_ΔAccY <= -137.00
|   |   |--- mean_ΔGyrY <= 4139.88
|   |   |   |--- class: 1
|   |   |--- mean_ΔGyrY >  4139.88
|   |   |   |--- class: 0
|   |--- min_ΔAccY >  -137.00
|   |   |--- class: 0
  */

  if (stats.min_dAccX <= -61.50) {
    if (stats.max_GyrY <= 3978.05) {
      isFall = 0;
    } else {
      if (stats.min_dAccY <= -45.50) {
        isFall = 1;
      } else {
        isFall = 0;
      }
    }
  } else {
    if (stats.min_dAccY <= -137.00) {
      if (stats.mean_dGyrY <= 4139.88) {
        isFall = 1;
      } else {
        isFall = 0;
      }
    } else {
      isFall = 0;
    }
  }

  Serial.print(isFall);
  if (isFall == true) {
    digitalWrite(alert, HIGH);
    Serial.println("\t🆘QUEDAAAA!!!!🆘");
  } else{
    Serial.println("\t🆗Sem quedas!");
  }

  if(digitalRead(button) == HIGH && digitalRead(alert) == HIGH){
    digitalWrite(alert, LOW);
  }

  delay(500);

  server.handleClient();
}
