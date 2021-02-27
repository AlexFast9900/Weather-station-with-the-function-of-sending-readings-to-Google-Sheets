// https://github.com/AlexFast9900/Weather-station-with-the-function-of-sending-readings-to-Google-Sheets

#include <iarduino_DHT.h>   // подключаем библиотеки
#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h>

iarduino_DHT sensor(16);     // объявляем  переменную для работы с датчиком DHT, указывая номер цифрового вывода к которому подключён датчик

// Здесь вводятся данные для Wi-Fi сети //
const char* ssid = "FedoraLinux"; // SSID
const char* password = "router_blin02"; // Пароль
//////////////////////////////////////////

const char *GScriptId = "AKfycbzFNeX3H3KJk7qTl5Q9fPxPcHfcnkdnJmlpGHiHsM738LbRpoq76556"; // Ключ из Google Script
const int dataPostDelay = 900000; // Задержка отправки данных: 15 минут = 15 * 60 * 1000
const char* host = "script.google.com"; // Хост для отправки
const char* googleRedirHost = "script.googleusercontent.com"; // Хост переадресации
const int httpsPort = 443; // Порт
String url = String("/macros/s/") + GScriptId + "/exec?"; // Собираем URL с данными (без проверки данных) 
 
const char* fingerprint = "F0 5C 74 77 3F 6B 25 D7 3B 66 4D 43 2F 7E BC 5B E9 28 86 AD"; // Сертефикат

HTTPSRedirect* client = nullptr;

void setup(){
    Serial.begin(115200); // устанавливаем скорость порта
    Serial.print("Подключение к Wi-Fi: "); 
    Serial.print(ssid); 
    Serial.flush();
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) { // Пока Wi-Fi не подключен - ничего не делаем (на самом деле выводим точки)
            delay(500); 
            Serial.print("."); 
    } 
    Serial.println(" Подключено.");
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
    Serial.print("Подключение к "); 
    Serial.println(host);
    bool flag = false; 
    for (int i=0; i<15; i++){ // 15 попыток подключения
            int retval = client->connect(host, httpsPort); 
            if (retval == 1) { 
                        flag = true; 
                        break; 
            } 
            else 
                    Serial.println("Не получилось подключиться. Пробую снова…"); 
    }
    Serial.println("Код подключения: " + String(client->connected())); // 1 - подключено, 0 - не подключено
    Serial.flush(); 
    if (!flag){ 
            Serial.print("Не удалось подключиться к серверу: "); 
            Serial.println(host); 
            Serial.println("Выход…"); 
            Serial.flush(); 
            return; 
    }
 
    // Данные будут отправляться, пока срок сертификата не закончится
    if (client->verify(fingerprint, host)) { 
            Serial.println("Сертификат не просрочен"); 
    } else { 
            Serial.println("Сертификат просрочен"); 
    }
    
}

void postData(int humi, int temp){ // Функция отправки данных в таблицы
    if (!client->connected()){ 
            Serial.println("Повторное подключение к клиенту…"); 
            client->connect(host, httpsPort);
    } 
    String urlFinal = url + "humi=" + String(humi) + "&temp=" + String(temp);
    client->GET(urlFinal, host);
}

void loop(){
  switch(sensor.read()){    // читаем показания датчика
    case DHT_OK:               Serial.println((String) "Датчик успешно подключен. Показания: " + sensor.hum + "% - " + sensor.tem + "*C");  break;
    case DHT_ERROR_NO_REPLY:   Serial.println(         "Ошибка подключения датчика");                          break;
    default:                   Serial.println(         "SENSOR ERROR");                               break;
  }
  postData(sensor.hum, sensor.tem); // Вызов функции отправки данных
  Serial.print("Следующая запись через "); Serial.print(dataPostDelay/60000); Serial.println(" минут");
  delay (dataPostDelay); // Пауза до следующей отправки
}
