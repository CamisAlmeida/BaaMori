// Nome do sistemas - BaaMori (Baa-san + Mamori = “proteger”)
// Inspirado no anime jiisan baasan wakagaeru (Grandpa and Grandma Turn Young Again)

// Inclusão das Bibliotecas
#include <Wire.h>
#include <WiFi.h>

// Cofiguração wifi
const char* ssid = "....";
const char* password = "....";

unsigned long prevTime = 0;
unsigned long nowTime = 0;
long interval = 100;

// Endereco I2C do sensor MPU-6050
const int MPU = 0x68;

// Variaveis para armazenar valores do sensor
float AccX, AccY, AccZ, GyrX, GyrY, GyrZ;
float nowAccX, nowAccY, nowAccZ, nowGyrX, nowGyrY, nowGyrZ;

void setup() {
  // Inicializa Serial
  Serial.begin(115200);

  // Conecxão wifi
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Inicializa o MPU-6050
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Configura Giroscópio: +/-2000°/s
  Wire.beginTransmission(MPU);
  Wire.write(0x1B);
  Wire.write(0b00011000);
  Wire.endTransmission();

  // Configura Acelerômetro: +/-16g
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);
  Wire.write(0b00011000);
  Wire.endTransmission();

  Serial.println("Setup concluido!!!!");
  // Cabeçalho com nomes das variáveis
  Serial.println("Time(ms)\tAccX(g)\tΔAccX\tAccY(g)\tΔAccY\tAccZ(g)\tΔAccZ\tGyrX(d/s)\tΔGyrX\tGyrY(d/s)\tΔGyrY\tGyrZ(d/s)\tΔGyrZ");
  prevTime = millis();
}

void loop() {
  nowTime = millis();
  datacollection(&AccX, &AccY, &AccZ, &GyrX, &GyrY, &GyrZ);

  if (nowTime - prevTime >= interval) {
    datacollection(&nowAccX, &nowAccY, &nowAccZ, &nowGyrX, &nowGyrY, &nowGyrZ);
    printData();
    prevTime = millis();
  }
}


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

void printData() {
  Serial.print(nowTime); Serial.print("\t");

  Serial.print(AccX / 2048, 2); Serial.print("\t");
  Serial.print(nowAccX - AccX, 0); Serial.print("\t");

  Serial.print(AccY / 2048, 2); Serial.print("\t");
  Serial.print(nowAccY - AccY, 0); Serial.print("\t");

  Serial.print(AccZ / 2048, 2); Serial.print("\t");
  Serial.print(nowAccZ - AccZ, 0); Serial.print("\t");

  Serial.print(GyrX / 16.4, 2); Serial.print("\t");
  Serial.print(nowGyrX - GyrX, 0); Serial.print("\t");

  Serial.print(GyrY / 16.4, 2); Serial.print("\t");
  Serial.print(nowGyrY - GyrY, 0); Serial.print("\t");

  Serial.print(GyrZ / 16.4, 2); Serial.print("\t");
  Serial.println(nowGyrZ - GyrZ, 0);
}

