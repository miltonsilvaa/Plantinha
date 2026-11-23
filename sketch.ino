#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "DHTesp.h"

const int PINO_DHT = 15;
const int PINO_LED = 2;

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* CHAVE_ESCRITA = "D4L0YH7CKRYR9ABV";
const char* CANAL_ID = "3580188";

const char* BOT_TOKEN = "9084193401:AAFzNjqc5ronT0EuqQWl3C7Ps3a3gafpkg0";
const char* CHAT_ID   = "6634516637";

const float TEMP_MIN = 18.0;
const float TEMP_MAX = 30.0;
const float UMID_MIN = 30.0;
const float UMID_MAX = 70.0;

const unsigned long INTERVALO_REENVIO = 2UL * 60UL * 1000UL;
bool avisouTemp = false;
bool avisouUmid = false;
unsigned long ultimoAlerta = 0;

DHTesp dhtSensor;

String lerCampo(int campo) {
  HTTPClient http;
  String url = String("http://api.thingspeak.com/channels/") + CANAL_ID +
               "/fields/" + campo + "/last.txt";
  http.begin(url);
  int codigo = http.GET();
  String resposta = "";
  if (codigo == 200) resposta = http.getString();
  http.end();
  resposta.trim();
  return resposta;
}

String escaparJson(const String& texto) {
  String saida = "";
  saida.reserve(texto.length() + 16);
  for (unsigned int i = 0; i < texto.length(); i++) {
    char c = texto[i];
    switch (c) {
      case '\\': saida += "\\\\"; break;
      case '"':  saida += "\\\""; break;
      case '\n': saida += "\\n";  break;
      case '\r': saida += "\\r";  break;
      case '\t': saida += "\\t";  break;
      default:
        if ((unsigned char)c < 0x20) {
          char buf[7];
          snprintf(buf, sizeof(buf), "\\u%04x", (unsigned int)(unsigned char)c);
          saida += buf;
        } else {
          saida += c;
        }
    }
  }
  return saida;
}

bool enviarTelegram(const String& texto) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String("https://api.telegram.org/bot") + BOT_TOKEN + "/sendMessage";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  String corpo = String("{\"chat_id\":\"") + CHAT_ID + "\",\"text\":\"" + escaparJson(texto) + "\"}";
  int codigo = http.POST(corpo);
  Serial.printf("Telegram: codigo %d\n", codigo);
  if (codigo != 200) Serial.println(http.getString());
  http.end();
  return codigo == 200;
}

bool foraDaFaixa(float valor, float minimo, float maximo) {
  return valor < minimo || valor > maximo;
}

String descreverMedida(const char* nome, const char* unidade, float valor, float minimo, float maximo) {
  if (valor < minimo) {
    return String(nome) + " BAIXA: " + String(valor, 1) + unidade +
           " (mínimo " + String(minimo, 1) + unidade + ")";
  }
  return String(nome) + " ALTA: " + String(valor, 1) + unidade +
         " (máximo " + String(maximo, 1) + unidade + ")";
}

String montarAlerta(float temp, float umid, bool tempFora, bool umidFora) {
  String msg = "⚠️ ALERTA: fora da faixa ideal";
  if (tempFora) msg += String("\n🌡️ ") + descreverMedida("Temperatura", " °C", temp, TEMP_MIN, TEMP_MAX);
  if (umidFora) msg += String("\n💧 ") + descreverMedida("Umidade", " %", umid, UMID_MIN, UMID_MAX);
  return msg;
}

String montarNormalizacao(float temp, float umid) {
  return String("✅ Normalizado: valores de volta à faixa ideal") +
         "\n🌡️ Temperatura: " + String(temp, 1) + " °C" +
         "\n💧 Umidade: " + String(umid, 1) + " %";
}

void tratarAlertas(float temp, float umid) {
  bool tempFora = foraDaFaixa(temp, TEMP_MIN, TEMP_MAX);
  bool umidFora = foraDaFaixa(umid, UMID_MIN, UMID_MAX);

  if (!tempFora && !umidFora) {
    if (avisouTemp || avisouUmid) {
      String msg = montarNormalizacao(temp, umid);
      Serial.println(msg);
      if (enviarTelegram(msg)) {
        avisouTemp = false;
        avisouUmid = false;
        ultimoAlerta = 0;
      }
    }
    return;
  }

  if (!tempFora) avisouTemp = false;
  if (!umidFora) avisouUmid = false;

  bool problemaNovo = (tempFora && !avisouTemp) || (umidFora && !avisouUmid);
  bool passouIntervalo = (millis() - ultimoAlerta) >= INTERVALO_REENVIO;
  if (!problemaNovo && !passouIntervalo) return;

  String msg = montarAlerta(temp, umid, tempFora, umidFora);
  Serial.println(msg);
  if (enviarTelegram(msg)) {
    avisouTemp = tempFora;
    avisouUmid = umidFora;
    ultimoAlerta = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);
  dhtSensor.setup(PINO_DHT, DHTesp::DHT22);

  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
}

void loop() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("Falha ao ler o DHT22!");
  } else {
    Serial.println("=========================================");
    Serial.printf("Temperatura: %.1f °C\n", data.temperature);
    Serial.printf("Umidade:     %.1f %%\n", data.humidity);

    tratarAlertas(data.temperature, data.humidity);

    String url = String("http://api.thingspeak.com/update?api_key=") + CHAVE_ESCRITA +
                 "&field1=" + String(data.temperature, 1) +
                 "&field3=" + String(data.humidity, 1);
    HTTPClient http;
    http.begin(url);
    int codigo = http.GET();
    Serial.printf("Envio: codigo %d\n", codigo);
    http.end();

    String comando = lerCampo(2);
    digitalWrite(PINO_LED, comando == "1" ? HIGH : LOW);
    Serial.printf("Comando LED: %s\n", comando.c_str());
  }

  delay(15000);
}
