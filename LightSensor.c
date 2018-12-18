#include <SPI.h>
#include <RF24.h>
#include <String.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
RF24 radio(7, 8); // CE, CSN
const uint64_t pipe = 0xE8E8F0F0E1LL;
char rdTxt[15]; //variável na qual serão guardadas as diversas mensagens recebidas
char ansTxt[33]; //variável que conterá a resposta a ser enviada

int calibrated[] = { -2, 47, 69, 86, 128, 154, 250, 336, 474, 596, 692, 770, 930, 1268, 1392, 1534, 1869 };
int sensor[] = { 1004, 330, 250, 187, 147, 142, 97, 78, 63, 50, 47, 43, 40, 36, 32, 30, 23 };

void radioRxMode()
{
  radio.begin();
  radio.openReadingPipe(0, pipe);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void radioTxMode()
{
  radio.begin();
  radio.openWritingPipe(pipe);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}
void setup() {
  radioRxMode();
  Serial.begin(9600);
}

int CalibratedValue(int value)
{
  if (value <= 23)
  {
    return 1869;
  }
  else if (value >= 1004)
  {
    return -2;
  }
  else
  {
    int Sindex = getIndeSensor(value);
    float sensorDist = percentageDistance(sensor[Sindex + 1], sensor[Sindex], value);
    int trueValue = lerp(calibrated[Sindex], calibrated[Sindex + 1], 1 - sensorDist);
    return trueValue;
  }
}

int getIndeCal(int value)
{
  int idx = 0;
  for (int i = 0; i < 17; i++)
  {
    if (calibrated[i] > value)
    {
      return idx;
    }
    idx++;
  }
  return idx;
}

int getIndeSensor(int value)
{
  int idx = 0;
  for (int i = 0; i < 17; i++)
  {
    if (sensor[i] < value)
    {
      return idx - 1;
    }
    idx++;
  }
  return idx - 1;
}

float percentageDistance(int min, int max, int point)
{
  max = max - min;
  point -= min;
  float v = ((float)point) / max;
  return v;
}

int lerp(int start, int end, float percent)
{
  return (int)roundf(start + percent * (end - start));
}

void loop() {
  if (radio.available()) {
    radio.read(&rdTxt, sizeof(rdTxt)); //ler uma mensagem do “Master”
    Serial.println(rdTxt);

    if (strcmp(rdTxt, "Query 05") == 0) { //verificar se a mensagem recebida é a esperada
      int sensorValue = CalibratedValue(analogRead(A0)); // ir buscar o valor do sensor
      //int aState = getState(sensorValue);

      sprintf (ansTxt, "Light (g05): %d", sensorValue); //construir a resposta
      Serial.println(ansTxt);
      radioTxMode(); // colocar em modo de escrita
      radio.write(ansTxt, sizeof(ansTxt)); // enviar a mensagem já construída
      radioRxMode(); // retornar ao modo de leitura
    }

  }
}

