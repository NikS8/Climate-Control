#include <iarduino_RTC.h>
iarduino_RTC time(RTC_DS1302, 5, 6, 7); // подключаем RTC модуль на базе чипа DS1302, выводы Arduino к выводам модуля RST, CLK, DAT
long lastWriteTime = 0;
long lastReadTime = 0;
long lastDataTime = 0;
//-------------
#include <DHT.h>        // You have to download DHT11  library
#define DHT1PIN 8                        // PIN №8 подключения датчика DTH11 на кухне
#define DHT1TYPE DHT11
DHT dht1(DHT1PIN, DHT1TYPE);
int tempDhtInside;
int humDhtInside;

#define DHT2PIN 9                        // PIN №9 подключения датчика DTH22 на улице
#define DHT2TYPE DHT22
DHT dht2(DHT2PIN, DHT2TYPE);
int tempDhtOutside;
int humDhtOutside;
//------------
#include <EasyTransfer.h>
#define DIR 2            // переключатель прием\передача на Pin2
int ID;              // номер ардуины
EasyTransfer ETinOfBoiler, ETinOfCollector, ETinOfKitchen, ETout;  //create  objects

struct SEND_DATA_STRUCTURE {                  // структура, которую будем передавать
  int ID;
  char action;
  int targetPin;
  int levelPin;
};
char action = "get";    //  "get"/"set" - запрос выдачи данных / команда на исполнение
int levelPin = 0;   //  установка уровня на Pin (LOW/HIGH)
int targetPin = 10;         //  Pin=10 управления контактером

struct RECEIVE_DATA_2_OF_COLLECTOR {         // структура, которую будем принимать
  int ID;
  int sensorDsHallIn;    // адрес датчика DS18B20 на трубе №1 из летней кухни
  int sensorDsHallOut;    // адрес датчика DS18B20 на трубе №2 из летней кухни
  int sensorDsCollectorIn;   // адрес датчика DS18B20 на входе в коллектор
  int sensorDsCollectorOut;  // адрес датчика DS18B20 на выходе из коллектора
};

struct RECEIVE_DATA_21_OF_BOILER {         // структура, которую будем принимать
  int ID;
  int sensorPhotoBoiler;      // фоторезисторы у кнопок-сигнализатров тенов котла
  int sensorDsTankLow;       // температура от датчика DS18B20 на стенке бака внизу
  int sensorDsTankMiddle;    // температура от датчика DS18B20 на стенке бака посередине
  int sensorDsTankHigh;      // температура от датчика DS18B20 на стенке бака вверху
  int sensorDsTankInside;    // температура от датчика DS18B20 внутри бака
  int sensorDsBoiler;        // температура от датчика DS18B20 на выходном патрубке котла
  int sensorDhtTBoiler;      // температура в бойлерной
  int sensorDhtHBoiler;      // влажность в бойлерной
  int sensorDsTankIn;    // температура от датчика DS18B20 на трубе в бак
  int sensorDsTankOut;   // температура от датчика DS18B20 на трубе из бака
  int sensorPressTankFrom;    // температура от датчика давления в трубе от бака

};

struct RECEIVE_DATA_61_OF_KITCHEN {         // структура, которую будем принимать
  int ID;
  int sensorDhtTKitchen;      // температура в летней кухне
  int sensorDhtHKitchen;      // влажность в летней кухне
  int sensorAlarmKitchen;     //сигнал открытия двери и окон в летней кухне
};

//give a name to the group of data
RECEIVE_DATA_2_OF_COLLECTOR rxOf2;
RECEIVE_DATA_21_OF_BOILER rxOf21;
RECEIVE_DATA_61_OF_KITCHEN rxOf61;
SEND_DATA_STRUCTURE txdata;
//------------------------------------

int tempDsHallIn;   // температура датчика DS18B20 на трубе №1 из летней кухни
int tempDsHallOut;   // температура датчика DS18B20 на трубе №2 из летней кухни
int tempDsCollectorIn;   // температура датчика DS18B20 на входе в коллектор
int tempDsCollectorOut;  // температура датчика DS18B20 на выходе из коллектора

int sensorPhotoBoiler;         // фоторезисторы у кнопок-сигнализатров тенов котла
int tempDsTankLow;       // температура от датчика DS18B20 на стенке бака внизу
int tempDsTankMiddle;    // температура от датчика DS18B20 на стенке бака посередине
int tempDsTankHigh;      // температура от датчика DS18B20 на стенке бака вверху
int tempDsTankInside;    // температура от датчика DS18B20 внутри бака
int tempDsBoiler;        // температура от датчика DS18B20 на выходном патрубке котла
int tempDhtBoiler;       // температура в бойлерной
int humDhtBoiler;        // влажность в бойлерной

int tempDsTankIn;    // температура от датчика DS18B20 на трубе в бак
int tempDsTankOut;   // температура от датчика DS18B20 на трубе из бака
float sensorPressTankFrom;    // давление от датчика давления в трубе от бака

int tempDhtKitchen;      // температура в летней кухне
int humDhtKitchen;      // влажность в летней кухне
int sensorAlarmKitchen;     //сигнал открытия двери и окон в летней кухне

//------------------------------------
#include <LiquidCrystal_1602_RUS.h>
LiquidCrystal_1602_RUS disp(32, 30, 28, 26, 24, 22);  // объект дисплей (MEGA 2560)
int switchX = 0;
//--------------
#include <SPI.h>
#include <Ethernet.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xEF, 0xED};
IPAddress ip(192, 168, 1, 156);                          //Check your router IP
EthernetClient client;
EthernetServer server(3003);

//-----------------
#include <SD.h>
#define SWITCH_TO_W5100 digitalWrite(4,HIGH); digitalWrite(53,LOW)    //  Переключение на интернет
#define SWITCH_TO_SD digitalWrite(53,HIGH); digitalWrite(4,LOW)       //  Переключение на SD-карту
#define ALL_OFF digitalWrite(53,HIGH); digitalWrite(4,HIGH)           //  Отключение SD-карты и интернета
//---------------
File myFile;                                          // Создаем класс для работы с SD
const int chipSelect = 4;
char filename[] = "00000000.csv";
const byte NUM_FIELDS = 1;                            //You can add extra fields but then you must list them on the next line. Read the tutorial at sparkfun! It will help.
const String fieldNames[NUM_FIELDS] = {"temp"};       //This has to be the same as the field you set up when you made the data.sparkfun.com account i.e. replace "temp" with what you used.
String fieldData[NUM_FIELDS];
//-----------------
#include <ArduinoJson.h>

//------------
#include <TimeLib.h>
#include <EthernetUdp.h>

//IPAddress timeServerUA(62, 149, 0, 30); // ntp.time.in.ua
IPAddress timeServerUA(62, 149, 2, 7); // ntp3.time.in.ua
const int timeZone = 2;     // зона Киевское время
EthernetUDP Udp;
unsigned int localPort = 8888;  // локальный порт для прослушивания UDP-пакетов
//-------------------

//-----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  time.begin();
  //--------------
  disp.begin(16, 2);  // инициализируем дисплей 2 x 16 символов
  disp.clear();  // очистка экрана
  disp.setCursor(0, 0);
  disp.print(L" Климат контроль");
  disp.setCursor(0, 1);
  disp.print(time.gettime("d.m.Y H:i")); // выводим время
  //------------------
  SWITCH_TO_SD;       //  Переключение на SD-карту
  pinMode(4, OUTPUT);
  pinMode(53, OUTPUT);
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    //     return;
  }

  Serial.println("initialization done.");
  Serial.println(time.gettime("d-m-Y, H:i:s, D")); // выводим время
  pinMode(4, OUTPUT);
  pinMode(10, OUTPUT);
  SWITCH_TO_W5100;       //  Переключение на интернет
  Serial.print("Initializing Ethernet...");
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Serial.println("Initializing Ethernet direct...");
    Ethernet.begin(mac, ip);
  } else {
    Serial.println("initialization done.");
  }

  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("My subnetMask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("My gatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("My dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());
  // give the Ethernet shield a second to initialize:

  Serial.print("FREE RAM: ");
  Serial.println(freeRam());

  delay(100);

  ALL_OFF;           //  Отключение SD-карты и интернета

  //  запyск RS-485
  Serial1.begin(9600); // start serial port
  ETinOfCollector.begin(details(rxOf2), &Serial1); //  //start the library, pass in the data details and the name of the serial port
  ETinOfBoiler.begin(details(rxOf21), &Serial1); //  //start the library, pass in the data details and the name of the serial port
  ETinOfKitchen.begin(details(rxOf61), &Serial1); //  //start the library, pass in the data details and the name of the serial port

  ETout.begin(details(txdata), &Serial1);

  pinMode(DIR, OUTPUT);
  delay(100);
  digitalWrite(DIR, LOW);                          // включаем прием

  delay(100);
  //----------
  dht1.begin();
  dht2.begin();
  //------------
  SWITCH_TO_W5100;       //  Переключение на интернет

  Ethernet.begin(mac, ip);
  delay(3000);// Дадим время шилду на инициализацию
  Serial.println("connecting...");
  //------------

  //time1302.begin();
  delay(250);
  Serial.println("TimeNTP Example");  //  "Синхронизация с помощью NTP"

  Serial.println(Ethernet.localIP());
  Udp.begin(localPort);
  Serial.println("waiting for sync");  //  "ждем синхронизации"

  setSyncProvider(getNtpTime);
  time.settime(second(), minute(), hour(), day(), month(), year()); // 0  сек, 51 мин, 21 час, 27, октября, 2015 года, вторник
  // time1302.settime(second(),minute(),hour(),day(),month(),year());  // 0  сек, 51 мин, 21 час, 27, октября, 2015 года, вторник

  //------------
}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

void loop() {

  // полyчение данных из локальных датчиков
  humDhtInside = dht1.readHumidity();
  tempDhtInside = dht1.readTemperature();
  humDhtOutside = dht2.readHumidity();
  tempDhtOutside = dht2.readTemperature();


  Serial.print("  switch txdata.ID = ");
  Serial.print(txdata.ID);                 // и отправляем в Serial

  Serial.println("  ");

  //    digitalWrite(DIR, LOW);                       // включаем прием
  //////
  switch (txdata.ID)  {

    case 2 : // Вывод на экран значений температуры и влажности на улице
      ETinOfCollector.receiveData();                           // если пришли данные
      receiveDataETinOfCollector();
      break;

    case 21 :
      ETinOfBoiler.receiveData();                           // если пришли данные
      receiveDataETinOfBoiler();
      break;

    case 61 :
      ETinOfKitchen.receiveData();
      receiveDataETinOfKitchen();
      break;

    default :
      // код выполняется если  не совпало ни одно предыдущее значение
      break;
  }

  //-----------
  if (millis() - lastDataTime > 5000) {     // Каждые 5 секунд меняются данные на дисплее
    // и отправляется запрос к ардуинам за данными.
    MyDispPrint();

    switchX++;
    if (switchX > 9) {
      switchX = 1;
    }
    lastDataTime = millis();
    delay(50);
    digitalWrite(DIR, HIGH);                      // включаем передачу
    delay(50);
    ETout.sendData();                              // отправляем в RS_485

    Serial.print(" lastDataTime = ");
    Serial.print(lastDataTime);                 // и отправляем в Serial port
    Serial.print("   txdata.ID = ");
    Serial.print(txdata.ID);                 // и отправляем в Serial port

    Serial.println("  ");
    delay(30);
    digitalWrite(DIR, LOW);                          // включаем прием
    delay(33);
    //----------

  }
  SWITCH_TO_W5100;       //  Переключение на интернет

  //----------------  Блок JSON

  EthernetClient client = server.available();
  if (client) {
    bool success = readRequest(client);
    if (success) {
      // Use http://arduinojson.org/assistant/ to
      // compute the right size for the buffer
      StaticJsonBuffer<400> jsonBuffer;
      JsonObject& json = prepareResponse(jsonBuffer);
      writeResponse(client, json);

      //-------------------------------

    }              //  Завершение 5 секундного периода


  }

  //----------------------------------

  client.stop();
  ALL_OFF;           //  Отключение SD-карты и интернета




  //------------------------
  //------------------------

  if (millis() - lastWriteTime > 59000) {   // every 60 secs.  Must be >15000.

    //// Когда температура измерена её можно вывести в файл на SD

    ALL_OFF;
    getFilename(filename);
    fileDateWriteSD();
    fileWriteSD();

    ALL_OFF;           //  Отключение SD-карты и интернета
    //--------------------------------------

    lastWriteTime = millis();             // store last write time

    //  Завершение 59 секундного периода
  }
  //---------------------------------------

  Serial.println("...");
  Serial.println("Конец цикла");
  Serial.println(" .......");
  //---------------------------------------

}
/////////////////////////////////////////////////////////////////////////////////////////////////
//			Дополнительные
//				функции
//////////////////////////////////////////////////////////////////////////////////////////////////////

//	Количество свободной памяти
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

/////////    Прием данных  от ардуины №2 от коллектора

void receiveDataETinOfCollector() {

  //   byte id = rxdata.ID; // читаем байт, в нем для кого этот пакет
  //   if (id == 01){ // и если пакет пришел нам от ардуины №1

  tempDsHallIn = rxOf2.sensorDsHallOut;    // температура датчика DS18B20 на трубе №1 из летней кухни
  tempDsHallOut = rxOf2.sensorDsHallIn;    // температура датчика DS18B20 на трубе №2 из летней кухни
  tempDsCollectorIn = rxOf2.sensorDsCollectorIn;   // температура датчика DS18B20 на входе в коллектор
  tempDsCollectorOut = rxOf2.sensorDsCollectorOut;  // температура датчика DS18B20 на выходе из коллектора

  ////////////////

  Serial.print(" rxdata.ID = ");
  Serial.print(rxOf2.ID);                 // и отправляем в Serial
  Serial.print("  ");

  Serial.print(" tempDsCollectorIn = ");
  Serial.print(tempDsCollectorIn);
  Serial.print(" tempDsCollectorOut = ");
  Serial.print(tempDsCollectorOut);                 // и отправляем в Serial
  Serial.print(" tempDsHallIn = ");
  Serial.print(tempDsHallIn);                 // и отправляем в Serial
  Serial.print(" tempDsHallOut = ");
  Serial.print(tempDsHallOut);                 // и отправляем в Serial


  Serial.println();
  //   delay(100);                                     // небольшая задержка, иначе неуспевает

}


/////////    Прием данных от ардуины №21 из бойлерной

void receiveDataETinOfBoiler() {

  //   byte id = rxOf21.ID; // читаем байт, в нем для кого этот пакет
  //   if (id == 01){ // и если пакет пришел нам от ардуины №1

  sensorPhotoBoiler = rxOf21.sensorPhotoBoiler;
  tempDsTankLow = rxOf21.sensorDsTankLow;       // температура от датчика DS18B20 на стенке бака внизу
  tempDsTankMiddle = rxOf21.sensorDsTankMiddle;    // температура от датчика DS18B20 на стенке бака посередине
  tempDsTankHigh = rxOf21.sensorDsTankHigh;      // температура от датчика DS18B20 на стенке бака вверху
  tempDsTankInside = rxOf21.sensorDsTankInside;    // температура от датчика DS18B20 внутри бака
  tempDsBoiler = rxOf21.sensorDsBoiler;        // температура от датчика DS18B20 на выходном патрубке котла
  tempDhtBoiler = rxOf21.sensorDhtTBoiler;      // температура в бойлерной
  humDhtBoiler = rxOf21.sensorDhtHBoiler;        // влажность в бойлерной

  tempDsTankIn = rxOf21.sensorDsTankOut;    // температура от датчика DS18B20 на трубе в бак
  tempDsTankOut = rxOf21.sensorDsTankIn;   // температура от датчика DS18B20 на трубе из бака
  sensorPressTankFrom = rxOf21.sensorPressTankFrom;    // давление от датчика давления в трубе от бака
  sensorPressTankFrom = ((sensorPressTankFrom - 102.3) / 167); // перевод в атм [(sensorPressTankFrom - 0,1*1023) / (1,6*1023/9,8)]
  ////////////////

  Serial.print(" ID = ");
  Serial.print(rxOf21.ID);                 // и отправляем в Serial
  Serial.print("  ");

  Serial.print(" sensorPhotoBoiler = ");
  Serial.print(sensorPhotoBoiler);
  Serial.print(" sensorPressTankFrom = ");
  Serial.print(sensorPressTankFrom);

  Serial.print(" tempDsTankLow = ");
  Serial.print(tempDsTankLow);                 // и отправляем в Serial
  Serial.print(" tempDsTankMiddle = ");
  Serial.print(tempDsTankMiddle);                 // и отправляем в Serial
  Serial.print(" tempDsTankHigh = ");
  Serial.print(tempDsTankHigh);
  Serial.print(" tempDsTankInside = ");
  Serial.print(tempDsTankInside);
  Serial.print(" rxOf21.sensorDsBoiler = ");
  Serial.print(tempDsBoiler);                 // и отправляем в Serial
  Serial.print(" tempDsTankIn = ");
  Serial.print(tempDsTankIn);
  Serial.print(" rxOf21.tempDsTankOut = ");
  Serial.print(tempDsTankOut);                 // и отправляем в Serial

  Serial.print(" tempDhtBoiler = ");
  Serial.print(tempDhtBoiler);                 // и отправляем в Serial
  Serial.print(" humDhtBoiler = ");
  Serial.print(humDhtBoiler);                 // и отправляем в Serial

  Serial.println();
  //   delay(100);                                     // небольшая задержка, иначе неуспевает

}

////////////////////////////////////////////////////////////////////////////////////////////////////


/////////    Прием данных  от ардуины №61 из новой кyхни

void receiveDataETinOfKitchen() {

  //   byte id = rxOf61.ID; // читаем байт, в нем для кого этот пакет
  //   if (id == 01){ // и если пакет пришел нам от ардуины №1

  tempDhtKitchen = rxOf61.sensorDhtTKitchen;      // температура в летней кухне
  humDhtKitchen = rxOf61.sensorDhtHKitchen;      // влажность в летней кухне

  sensorAlarmKitchen = rxOf61.sensorAlarmKitchen;     //сигнал открытия двери и окон в летней кухне

  ////////////////

  Serial.print(" rxOf61.ID = ");
  Serial.print(rxOf61.ID);                 // и отправляем в Serial
  Serial.print("  ");

  Serial.print(" tempDhtKitchen = ");
  Serial.print(tempDhtKitchen);
  Serial.print(" humDhtKitchen = ");
  Serial.print(humDhtKitchen);                 // и отправляем в Serial
  Serial.print(" sensorAlarmKitchen = ");
  Serial.print(sensorAlarmKitchen);                 // и отправляем в Serial

  Serial.println();
  //   delay(100);                                     // небольшая задержка, иначе неуспевает

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Вывод информации на дисплей и влючение передачи данных по RS-485 каждой ардyине в сети по ее ID

int MyDispPrint() {
  switch (switchX)  {

    case 1 : // Вывод на экран значений температуры и влажности на улице

      disp.clear();  // очистка экрана
      disp.setCursor(3, 0);
      disp.print(L"У л и ц а");
      disp.setCursor(2, 1);
      disp.print(int(tempDhtOutside));
      disp.print("°C   ");
      disp.print(int(humDhtOutside));
      disp.print(" %");

      txdata.ID = 2;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем
      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;

    case 2 : // Вывод на экран значений температуры и влажности в комнате

      disp.clear();  // очистка экрана
      disp.setCursor(3, 0);
      disp.print(L"Прихожая");
      disp.setCursor(2, 1);
      disp.print(int(tempDhtInside));
      disp.print("°C   ");
      disp.print(int(humDhtInside));
      disp.print(" %");

      txdata.ID = 21;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем
      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;

    case 3 : // Вывод на экран значений температуры и влажности в комнате

      disp.clear();  // очистка экрана
      disp.setCursor(0, 0);
      disp.print(L" К о м н а т а");
      disp.setCursor(2, 1);
      disp.print(int(tempDhtKitchen));
      disp.print("°C   ");
      disp.print(int(humDhtKitchen));
      disp.print(" %");

      txdata.ID = 61;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем
      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;


    case 4 : // Вывод на экран значений температуры и влажности в бойлерной

      disp.clear();  // очистка экрана
      disp.setCursor(0, 0);
      disp.print(L"   Бойлерная");
      disp.setCursor(2, 1);
      disp.print(int(tempDhtBoiler));
      disp.print("°C   ");
      disp.print(int(humDhtBoiler));
      disp.print(" %");

      txdata.ID = 2;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем

      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;


    case 5 :  // Вывод на экран значений температуры воды на выходе из котла

      disp.clear();  // очистка экрана
      disp.setCursor(0, 0);
      disp.print(L"   К о т е л");
      disp.setCursor(0, 1);
      disp.print(L"температура ");
      disp.print(int(tempDsBoiler));
      disp.print("°C   ");

      txdata.ID = 21;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем

      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;

    case 6 :  // Вывод на экран значений температуры воды в баке

      disp.clear();  // очистка экрана
      disp.setCursor(0, 0);
      disp.print(L" Бак:");
      disp.print(" N1=");
      disp.print(int(tempDsTankHigh));
      disp.print("°C ");
      disp.setCursor(0, 1);
      disp.print("N2=");
      disp.print(int(tempDsTankMiddle));
      disp.print("°C  ");
      disp.print("N3=");
      disp.print(int(tempDsTankLow));
      disp.print("°C ");

      txdata.ID = 61;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем

      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;

    case 7 :  // Вывод на экран значений температуры воды в баке

      disp.clear();  // очистка экрана
      disp.setCursor(1, 0);
      disp.print(L"Датчик в баке");
      disp.setCursor(5, 1);
      disp.print(int(tempDsTankInside));
      disp.print("°C   ");

      txdata.ID = 2;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем

      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;

    case 8 :  // Вывод на экран значений температуры воды в коллекторе

      disp.clear();  // очистка экрана
      disp.setCursor(0, 0);
      disp.print(L"   Коллектор:");
      disp.setCursor(0, 1);
      disp.print("N1=");
      disp.print(int(tempDsCollectorIn));
      disp.print("°C  ");
      disp.print("N2=");
      disp.print(int(tempDsCollectorOut));
      disp.print("°C ");

      txdata.ID = 21;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем

      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;

    case 9 :  // Вывод на экран значений температуры воды в коллекторе

      disp.clear();  // очистка экрана
      disp.setCursor(3, 0);
      disp.print(L"Из кухни:");
      disp.setCursor(0, 1);
      disp.print("N1=");
      disp.print(int(tempDsHallIn));
      disp.print("°C  ");
      disp.print("N2=");
      disp.print(int(tempDsHallOut));
      disp.print("°C ");

      txdata.ID = 61;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем

      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;

    default :
      // код выполняется если  не совпало ни одно предыдущее значение
      txdata.ID = 21;
      delay(50);
      digitalWrite(DIR, HIGH);                      // включаем передачу
      delay(50);
      ETout.sendData();                              // отправляем

      Serial.print(" switchX = ");
      Serial.print(switchX);
      Serial.print(" txdata.ID = ");
      Serial.println(txdata.ID);

      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
//----  найменование файла для записи на SD

void getFilename(char *filename) {

  int year = time.year;
  int month = time.month;
  int day = time.day;

  filename[0] = '2';
  filename[1] = '0';
  filename[2] = year / 10 + '0';
  filename[3] = year % 10 + '0';
  filename[4] = month / 10 + '0';
  filename[5] = month % 10 + '0';
  filename[6] = day / 10 + '0';
  filename[7] = day % 10 + '0';
  filename[8] = '.';
  filename[9] = 'c';
  filename[10] = 's';
  filename[11] = 'v';

  return;
}

////////////////////////////////////////////////////////////////////////////////

//----------------ПИШЕМ НА SD КАРТУ ВРЕМЯ И ДАННЫЕ---------------------------------------------------------------//

void fileWriteSD() {
  Serial.println("SD work start");
  SWITCH_TO_SD;         //  Переключение на SD-карту

  if (SD.exists("filename")) {
    Serial.println("file exists.");
  }
  else {
    Serial.println("file doesn't exist.");
  }

  myFile = SD.open(filename, FILE_WRITE);
  Serial.println("file opened");			// if the file opened okay, write to it:

  if (myFile) {
    Serial.println("Writing to data file...");
    myFile.print(time.gettime("Y.m.d H:i"));        // записываем дату и время
    myFile.print(F(" ; tempDhtInside ; "));                // записываем температуру
    myFile.print(tempDhtInside);
    myFile.print(F(" ; humDhtInside ; "));               // записываем влажность
    myFile.print(humDhtInside);
    myFile.print(F(" ; sensorPhotoBoiler ; "));       // записываем значение величины фотосопротивления
    myFile.print(sensorPhotoBoiler);
    myFile.print(F(" ; tempDsBoiler ; "));                // записываем температуру
    myFile.print(int(tempDsBoiler));
    myFile.print(F(" ; tempDsTankHigh ; "));                // записываем температуру
    myFile.print(int(tempDsTankHigh));
    myFile.print(F(" ; tempDsTankMiddle ; "));                // записываем температуру
    myFile.print(int(tempDsTankMiddle));
    myFile.print(F(" ; tempDsTankLow ; "));                // записываем температуру
    myFile.print(int(tempDsTankLow));
    myFile.print(F(" ; tempDsCollectorOut ; "));                // записываем температуру
    myFile.print(int(tempDsTankInside));
    myFile.print(F(" ; tempDsCollectorIn ; "));                // записываем температуру
    myFile.print(int(tempDsCollectorIn));
    myFile.print(F(" ; tempDsCollectorOut ; "));                // записываем температуру
    myFile.print(int(tempDsCollectorOut));
    myFile.println(" ; end... ");

    // close the file:
    myFile.close();
    Serial.println("done. SD work end.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data file.");
    myFile.close();
    Serial.println("SD work end.");
  }

}
//////////////////////////////////////////////////////////////////////////////

//----------------ПИШЕМ НА SD КАРТУ ТОЛЬКО ДАННЫЕ---------------------------------------------------------------//
void fileDateWriteSD() {
  Serial.println("SD work start");
  SWITCH_TO_SD;         //  Переключение на SD-карту

  if (SD.exists("filename")) {
    Serial.println("file exists.");
  }
  else {
    Serial.println("file doesn't exist.");
  }

  myFile = SD.open(filename, FILE_WRITE);
  Serial.println("file opened");      // if the file opened okay, write to it:

  if (myFile) {
    Serial.println("Writing to data file...");
    myFile.print(time.gettime("Y.m.d H:i"));        // записываем дату и время
    myFile.print(F(" ; tempDhtInside ; "));                // записываем температуру
    myFile.print(tempDhtInside);
    myFile.print(F(" ; humDhtInside ; "));               // записываем влажность
    myFile.print(humDhtInside);
    myFile.print(F(" ; sensorPhotoBoiler ; "));       // записываем значение величины фотосопротивления
    myFile.print(sensorPhotoBoiler);
    myFile.print(F(" ; stempDsBoiler ; "));                // записываем температуру
    myFile.print(tempDsBoiler);
    myFile.print(F(" ; tempDsTankHigh ; "));                // записываем температуру
    myFile.print(tempDsTankHigh);
    myFile.print(F(" ; tempDsTankMiddle ; "));                // записываем температуру
    myFile.print(tempDsTankMiddle);
    myFile.print(F(" ; tempDsTankLow ; "));                // записываем температуру
    myFile.print(tempDsTankLow);
    myFile.print(F(" ; tempDsTankInside ; "));                // записываем температуру
    myFile.print(tempDsTankInside);
    myFile.print(F(" ; tempDsCollectorIn ; "));                // записываем температуру
    myFile.print(tempDsCollectorIn);
    myFile.print(F(" ; tempDsCollectorOut ; "));                // записываем температуру
    myFile.print(tempDsCollectorOut);
    myFile.println(" ; end... ");

    // close the file:
    myFile.close();
    Serial.println("done. SD work end.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data file.");
    myFile.close();
    Serial.println("SD work end.");
  }

}

/////////////////////////////////////////////////////////////////////////////////
//       Rest

bool readRequest(EthernetClient& client) {
  bool currentLineIsBlank = true;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n' && currentLineIsBlank) {
        return true;
      } else if (c == '\n') {
        currentLineIsBlank = true;
      } else if (c != '\r') {
        currentLineIsBlank = false;
      }
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////////
//       Rest
JsonObject& prepareResponse(JsonBuffer& jsonBuffer) {

  JsonObject& root = jsonBuffer.createObject();
  root["time"] = (time.gettime("Y-m-dTH:i:s+0200"));        // записываем дату и время

  JsonArray& analogValues = root.createNestedArray("analog");

  root["sensorPhotoBoiler"] = sensorPhotoBoiler;
  root["sensorPressTankFrom"] = sensorPressTankFrom;    // давление от датчика давления в трубе от бака

  JsonArray& digitalValues = root.createNestedArray("digital");

  root["tempDhtOutside"] = tempDhtOutside;
  root["humDhtOutside"] = humDhtOutside;
  root["tempDhtInside"] = tempDhtInside;
  root["humDhtInside"] = humDhtInside;

  root["tempDhtKitchen"] = tempDhtKitchen;      // температура в летней кухне
  root["humDhtKitchen"] = humDhtKitchen;      // влажность в летней кухне

  root["tempDhtBoiler"] = tempDhtBoiler;       // температура в бойлерной
  root["humDhtBoiler"] = humDhtBoiler;        // влажность в бойлерной

  root["tempDsTankHigh"] = tempDsTankHigh;
  root["tempDsTankMiddle"] = tempDsTankMiddle;
  root["tempDsTankLow"] = tempDsTankLow;
  root["tempDsTankInside"] = tempDsTankInside;
  root["tempDsBoiler"] = tempDsBoiler;

  root["tempDsTankIn"] = tempDsTankIn;    // температура от датчика DS18B20 на трубе в бак
  root["tempDsTankOut"] = tempDsTankOut;   // температура от датчика DS18B20 на трубе из бака

  root["tempDsCollectorIn"] = tempDsCollectorIn;
  root["tempDsCollectorOut"] = tempDsCollectorOut;
  root["tempDsHallIn"] = tempDsHallIn;
  root["tempDsHallOut"] = tempDsHallOut;

  return root;
}

/////////////////////////////////////////////////////////////////////////////////
//       Rest
void writeResponse(EthernetClient& client, JsonObject& json) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");

  client.println("Connection: close");
  client.println();

  json.prettyPrintTo(client);
}
/////////////////////////////////////////////////////////////////////////////////
/*-------- Код для NTP ----- синхронизация даты и времени  -----*/

const int NTP_PACKET_SIZE = 48; // NTP-время – в первых 48 байтах сообщения
byte packetBuffer[NTP_PACKET_SIZE]; // буфер для хранения входящих и исходящих пакетов

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // отбраковываем все пакеты, полученные ранее
  Serial.println("Transmit NTP Request");  //  "Передача NTP-запроса"

  sendNTPpacket(timeServerUA);

  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");  //  "Получение NTP-ответа"
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // считываем пакет в буфер
      unsigned long secsSince1900;
      // конвертируем 4 байта (начиная с позиции 40) в длинное целое число:
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");  //  "Нет NTP-ответа :("
  return 0; // если время получить не удалось, возвращаема «0»
}

// отправляем NTP-запрос серверу времени по указанному адресу:
void sendNTPpacket(IPAddress &address)
{
  // задаем все байты в буфере на «0»:
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // инициализируем значения для создания NTP-запроса
  packetBuffer[0] = 0b11100011;   // LI (от «leap indicator», т.е. «индикатор перехода»), версия, режим работы
  packetBuffer[1] = 0;     // слой (или тип часов)
  packetBuffer[2] = 6;     // интервал запросов
  packetBuffer[3] = 0xEC;  // точность
  // 8 байтов с нулями, обозначающие базовую задержку и базовую дисперсию:
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // После заполнения всех указанных полей
  // вы сможете отправлять пакет с запросом о временной метке:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////*******//////////////////////////////////////////
//  E N D

