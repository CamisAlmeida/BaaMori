#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi config
const char* ssid = "Camis";
const char* password = "281Camis";

const int MPU = 0x68;
WebServer server(80);

float AccX, AccY, AccZ, GyrX, GyrY, GyrZ;
float nowAccX, nowAccY, nowAccZ, nowGyrX, nowGyrY, nowGyrZ;

unsigned long prevTime = 0;
unsigned long nowTime = 0;
long interval = 100;

// Variável para enviar dados atuais na rota /data
String currentData = "";

// Função para coletar dados do sensor MPU6050
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

// Atualiza a string com os dados atuais formatados tabulados
void updateCurrentData() {
  currentData = String(nowTime) + "\t";

  currentData += String(AccX / 2048.0, 2) + "\t" + String(nowAccX - AccX, 0) + "\t";
  currentData += String(AccY / 2048.0, 2) + "\t" + String(nowAccY - AccY, 0) + "\t";
  currentData += String(AccZ / 2048.0, 2) + "\t" + String(nowAccZ - AccZ, 0) + "\t";

  currentData += String(GyrX / 16.4, 2) + "\t" + String(nowGyrX - GyrX, 0) + "\t";
  currentData += String(GyrY / 16.4, 2) + "\t" + String(nowGyrY - GyrY, 0) + "\t";
  currentData += String(GyrZ / 16.4, 2) + "\t" + String(nowGyrZ - GyrZ, 0);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Rota para página principal com atualização automática via JS
  server.on("/", []() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <title>Dados MPU6050</title>
  <style>
    #dados {
      white-space: pre-wrap;
      height: 400px;
      overflow-y: scroll;
      border: 1px solid #ccc;
      padding: 10px;
      font-family: monospace;
      background-color: #f9f9f9;
    }
  </style>
</head>
<body>
  <h2>Dados MPU6050 (atualização automática a cada 500ms)</h2>
  <pre id='dados'>Carregando dados...</pre>

  <script>
  const dadosEl = document.getElementById('dados');
  const maxLinhas = 100;  // limite de linhas para não travar

  function atualiza() {
    fetch('/data')
      .then(resp => resp.text())
      .then(txt => {
        if (dadosEl.innerText === "Carregando dados...") {
          dadosEl.innerText = "";
        }
        dadosEl.innerText += txt + '\n';

        let linhas = dadosEl.innerText.split('\n');
        if (linhas.length > maxLinhas) {
          linhas = linhas.slice(linhas.length - maxLinhas);
          dadosEl.innerText = linhas.join('\n');
        }
        dadosEl.scrollTop = dadosEl.scrollHeight;
      });
  }
  setInterval(atualiza, 500);
  atualiza();
</script>

</body>
</html>
)rawliteral";
    server.send(200, "text/html", html);
  });

  // Rota que retorna os dados atuais no formato tabulado
  server.on("/data", []() {
    server.send(200, "text/plain", currentData);
  });

  server.begin();

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

  Serial.println("Setup concluído!");
  prevTime = millis();
}

void loop() {
  nowTime = millis();

  datacollection(&AccX, &AccY, &AccZ, &GyrX, &GyrY, &GyrZ);

  if (nowTime - prevTime >= interval) {
    datacollection(&nowAccX, &nowAccY, &nowAccZ, &nowGyrX, &nowGyrY, &nowGyrZ);
    updateCurrentData();  // atualiza a string com os dados mais recentes
    prevTime = millis();

    Serial.println(currentData);  // opcional: para acompanhar no monitor serial
  }

  server.handleClient();
}
