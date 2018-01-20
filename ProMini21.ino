#include <DHT.h>        // You have to download DHT  library
#define DHTPIN 5                        // PIN №5 подключения датчика DTH22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
//------------

#include <OneWire.h>                    // Библиотека протокола 1-Wire
#include <DallasTemperature.h>          // Библиотека для работы с датчиками DS*
#define ONE_WIRE_BUS 9                 // Шина данных на 9 пине для датчиков DS18B20 (ардуино ProMini)
#define TEMPERATURE_PRECISION 9         // Точность измерений в битах (по умолчанию 12)
OneWire oneWire(ONE_WIRE_BUS);            // Создаем экземпляр объекта протокола 1-WIRE - OneWire
DallasTemperature sensorsDS(&oneWire);    // На базе ссылки OneWire создаем экземпляр объекта, работающего с датчиками DS*
// адрес датчика DS18B20 на выходном патрубке котла:
DeviceAddress sensorDsBoiler = { 0x28, 0xFF, 0x72, 0x7C, 0x01, 0x17, 0x05, 0x0A };        
// адрес датчика DS18B20 на стенке бака внизу:
DeviceAddress sensorDsTankLow = { 0x28, 0xFF, 0x3D, 0x1C, 0xB3, 0x16, 0x04, 0x75 };       
// адрес датчика DS18B20 на стенке бака посередине:
DeviceAddress sensorDsTankMiddle = { 0x28, 0xFF, 0xCF, 0x04, 0xB3, 0x16, 0x04, 0x74 };    
// адрес датчика DS18B20 на стенке бака вверху:
DeviceAddress sensorDsTankHigh = { 0x28, 0xFF, 0xCF, 0x09, 0xB3, 0x16, 0x03, 0x6A };      
// адрес датчика DS18B20 внутри бака:
DeviceAddress sensorDsTankInside = { 0x28, 0xFF, 0x51, 0xBD, 0xA1, 0x16, 0x05, 0x38 };    
// адрес датчика DS18B20 на трубе в бак:
DeviceAddress sensorDsTankIn = { 0x28, 0xFF, 0x9F, 0x65, 0x01, 0x17, 0x04, 0xC9 };    
// адрес датчика DS18B20 на трубе из бака:
DeviceAddress sensorDsTankOut = { 0x28, 0xFF, 0xAE, 0x35, 0xC1, 0x16, 0x04, 0xCF }; 

int sensorPhotoBoiler;         // фоторезисторы у кнопок-сигнализатров тенов котла
int sensorDsTempBoiler;        // температура от датчика DS18B20 на выходном патрубке котла
int sensorDhtTempBoiler;       // температура в бойлерной
int sensorDhtHumBoiler;        // влажность в бойлерной
int sensorDsTempTankLow;       // температура от датчика DS18B20 на стенке бака внизу
int sensorDsTempTankMiddle;    // температура от датчика DS18B20 на стенке бака посередине
int sensorDsTempTankHigh;      // температура от датчика DS18B20 на стенке бака вверху
int sensorDsTempTankInside;    // температура от датчика DS18B20 внутри бака
int sensorDsTempTankIn;    // температура от датчика DS18B20 на трубе в бак
int sensorDsTempTankOut;   // температура от датчика DS18B20 на трубе из бака
int sensorPressTankFrom;    // давление от датчика давления в трубе от бака
//-----------------
int action = 0;    //  0/1 ("get"/"set") - запрос выдачи данных / команда на исполнение
int levelPin = 0;   //  0/1 - установка уровня на Pin (LOW/HIGH)
//-----------------
int photoPin = A0;          //  фоторезистор подключен 0-му аналоговому входу
int pressPin = A1;          //  датчик давления в трубе от бака подключен 1-му аналоговому входу
//-----------------
int targetPin = 10;         //  Pin=10 управления контактером
//-------------------------------------------------
#include <SoftEasyTransfer.h>
#include <SoftwareSerial.h>
#define DIR 13            // переключатель прием\передача на Pin13
int ID = 21;              // номер этой ардуины
SoftwareSerial mySerial(2, 3);      // Serial для RS485
SoftEasyTransfer ETin, ETout;  //create two objects

struct SEND_DATA_STRUCTURE {       // структура, которую будем передавать
  int ID;
  int sensorPhotoBoiler;      // фоторезисторы у кнопок-сигнализатров тенов котла
  int sensorDsTankLow;        // температура от датчика DS18B20 на стенке бака внизу
  int sensorDsTankMiddle;     // температура от датчика DS18B20 на стенке бака посередине
  int sensorDsTankHigh;       // температура от датчика DS18B20 на стенке бака вверху
  int sensorDsTankInside;     // температура от датчика DS18B20 внутри бака
  int sensorDsBoiler;         // температура от датчика DS18B20 на выходном патрубке котла
  int sensorDhtTBoiler;       // температура в бойлерной
  int sensorDhtHBoiler;       // влажность в бойлерной
  int sensorDsTankIn;         // температура от датчика DS18B20 на трубе в бак
  int sensorDsTankOut;        // температура от датчика DS18B20 на трубе из бака
  int sensorPressTankFrom;    // температура от датчика давления в трубе из бака
};

struct RECEIVE_DATA_STRUCTURE {         // структура, которую будем принимать
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int ID;
  int action;
  int targetPin;
  int levelPin;
};

//give a name to the group of data
RECEIVE_DATA_STRUCTURE rxdata;
SEND_DATA_STRUCTURE txdata;

//-----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  //----------
  mySerial.begin(9600); // start serial port
  ETin.begin(details(rxdata), &mySerial); //start the library, pass in the data details and the name of the serial port
  ETout.begin(details(txdata), &mySerial);

  pinMode(DIR, OUTPUT);
  delay(50);
  digitalWrite(DIR, LOW);     // включаем прием
  //----------
  sensorsDS.begin(); // Запускаем поиск и запуск всех датчиков DS18B20 на шине
  dht.begin();
  //------------
pinMode(targetPin, OUTPUT);
}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

void loop() {

  digitalWrite(DIR, LOW);                       // включаем прием

  //----------------------------------

  sensorsDS.requestTemperatures(); // Запускаем процесс измерение температуры на всех датчиках DS18B20
  delay(50);  // небольшая задержка, иначе неуспевают

  sensorDhtTempBoiler = dht.readTemperature();
  sensorDhtHumBoiler = dht.readHumidity();

  sensorPhotoBoiler = analogRead(photoPin);
  sensorDsTempBoiler = sensorsDS.getTempC(sensorDsBoiler);
  sensorDsTempTankIn = sensorsDS.getTempC(sensorDsTankIn);
  sensorDsTempTankOut = sensorsDS.getTempC(sensorDsTankOut);
  sensorPressTankFrom = analogRead(pressPin);

  sensorDsTempTankHigh = sensorsDS.getTempC(sensorDsTankHigh);
  sensorDsTempTankMiddle = sensorsDS.getTempC(sensorDsTankMiddle);
  sensorDsTempTankLow = sensorsDS.getTempC(sensorDsTankLow);
  sensorDsTempTankInside = sensorsDS.getTempC(sensorDsTankInside);

  //-------------
  if (ETin.receiveData()) {                         // если пришли данные
    
    byte id = rxdata.ID; // читаем байт, в нем для кого этот пакет

    if (id == ID) { // и если пакет пришел нам

      if (rxdata.action == 1) {
        
           if (rxdata.levelPin == 1) {
        targetPin = rxdata.targetPin;
        levelPin = rxdata.levelPin;
       
        delay(50);
 //       analogWrite(targetPin, levelPin);
              digitalWrite(targetPin, LOW);   // включаем реле (подаем питание на контактер включения тенов)
           }
        else
        {
           digitalWrite(targetPin, HIGH);    // выключаем реле
        }
      }
      else
      {
        //      ID = 21;
        txdata.ID = ID;
        txdata.sensorPhotoBoiler = sensorPhotoBoiler;
        txdata.sensorDsTankLow = sensorDsTempTankLow;       // температура от датчика DS18B20 на стенке бака внизу
        txdata.sensorDsTankMiddle = sensorDsTempTankMiddle;    // температура от датчика DS18B20 на стенке бака посередине
        txdata.sensorDsTankHigh = sensorDsTempTankHigh;      // температура от датчика DS18B20 на стенке бака вверху
        txdata.sensorDsTankInside = sensorDsTempTankInside;    // температура от датчика DS18B20 внутри бака
        txdata.sensorDsBoiler = sensorDsTempBoiler;        // температура от датчика DS18B20 на выходном патрубке котла
        txdata.sensorDhtTBoiler = sensorDhtTempBoiler;      // температура в бойлерной
        txdata.sensorDhtHBoiler = sensorDhtHumBoiler;        // влажность в бойлерной

        txdata.sensorDsTankIn = sensorDsTempTankIn;    // температура от датчика DS18B20 на трубе в бак
        txdata.sensorDsTankOut = sensorDsTempTankOut;   // температура от датчика DS18B20 на трубе из бака
        txdata.sensorPressTankFrom = sensorPressTankFrom;    // давление от датчика давления в трубе от бака

        Serial.print(" ID = ");
        Serial.print(txdata.ID);                 // и отправляем в Serial
        Serial.print("  ");

        Serial.print(" sensorPhotoBoiler = ");
        Serial.print(sensorPhotoBoiler);
        Serial.print(" sensorPressTankFrom = ");
        Serial.print(sensorPressTankFrom);

        Serial.print(" sensorDsTempTankLow = ");
        Serial.print(sensorDsTempTankLow);                 // и отправляем в Serial
        Serial.print(" sensorDsTempTankMiddle = ");
        Serial.print(sensorDsTempTankMiddle);                 // и отправляем в Serial
        Serial.print(" sensorDsTempTankHigh = ");
        Serial.print(sensorDsTempTankHigh);
        Serial.print(" sensorDsTempTankInside = ");
        Serial.print(sensorDsTempTankInside);
        Serial.print(" sensorDsTempBoiler = ");
        Serial.print(sensorDsTempBoiler);                 // и отправляем в Serial

        Serial.print(" sensorDsTempTankIn = ");
        Serial.print(sensorDsTempTankIn);
        Serial.print(" sensorDsTempTankOut = ");
        Serial.print(sensorDsTempTankOut);

        Serial.print(" sensorDhtTempBoiler = ");
        Serial.print(sensorDhtTempBoiler);                 // и отправляем в Serial
        Serial.print(" sensorDhtHumBoiler = ");
        Serial.print(sensorDhtHumBoiler);                 // и отправляем в Serial

        Serial.println();
        delay(30);                                     // небольшая задержка, иначе неуспевает

        digitalWrite(DIR, HIGH);                      // включаем передачу
        delay(50);

        ETout.sendData();                              // отправляем
      }
    }
  }
  //----------
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//			Дополнительные
//				функции
//////////////////////////////////////////////////////////////////////////////////////////////////////

