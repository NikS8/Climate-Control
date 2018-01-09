#include <DHT.h>        // You have to download DHT11  library
#define DHTPIN 5                        // PIN №8 подключения датчика DTH11
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
int sensorDhtTempKitchen;
int sensorDhtHumKitchen;

//------------

//-----------------
int alarmKitchenPin = A0;          //сигнал открытия двери и окон в летней кухне
int sensorAlarmKitchen;
//-------------------------------------------------
#include <SoftEasyTransfer.h>
#include <SoftwareSerial.h>
#define DIR 13            // переключатель прием\передача на Pin13

SoftwareSerial mySerial(2, 3);      // Serial для RS485
SoftEasyTransfer ETin, ETout;  //create two objects

int ID = 61;              // номер этой ардуины в летней кухне
struct SEND_DATA_STRUCTURE {                  // структура, которую будем передавать
  int ID;
  int sensorDhtTKitchen;      // температура в летней кухне
  int sensorDhtHKitchen;      // влажность в летней кухне
  int sensorAlarmKitchen;     //сигнал открытия двери и окон в летней кухне
};

struct RECEIVE_DATA_STRUCTURE {         // структура, которую будем принимать
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int ID;
  char action;
  int targetPin;
  int levelPin;
};
char action = "get";    //  "get"/"set" - запрос выдачи данных / команда на исполнение
int levelPin = 0;       //  установка уровня на Pin (LOW/HIGH)
int targetPin;          //  Pin управления

//give a name to the group of data
RECEIVE_DATA_STRUCTURE rxdata;
SEND_DATA_STRUCTURE txdata;
//-----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  //----------
  mySerial.begin(9600); // start serial port
  ETin.begin(details(rxdata), &mySerial); //  //start the library, pass in the data details and the name of the serial port
  ETout.begin(details(txdata), &mySerial);

  pinMode(DIR, OUTPUT);
  delay(50);
  digitalWrite(DIR, LOW);     // включаем прием
  //----------
  dht.begin();
  //------------

}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

void loop() {
  //----------------------------------

  sensorDhtTempKitchen = dht.readTemperature();
  sensorDhtHumKitchen = dht.readHumidity();

  sensorAlarmKitchen = analogRead(alarmKitchenPin);

  //-------------
  if (ETin.receiveData()) {                         // если пришли данные
    byte id = rxdata.ID; // читаем байт, в нем для кого этот пакет

    if (id == ID) { // и если пакет пришел нам

      if (rxdata.action == "set") {
        targetPin = rxdata.targetPin;
        levelPin = rxdata.levelPin;

        pinMode(targetPin, OUTPUT);
        delay(50);
        analogWrite(targetPin, levelPin);

      }
      else
      {

        //      ID = 61;
        txdata.ID = ID;
        txdata.sensorDhtTKitchen = sensorDhtTempKitchen;      // температура в летней кухне
        txdata.sensorDhtHKitchen = sensorDhtHumKitchen;        // влажность в летней кухне
        txdata.sensorAlarmKitchen = sensorAlarmKitchen;       //сигнал открытия двери и окон в летней кухне

        Serial.print(" ID = ");
        Serial.print(txdata.ID);                 // и отправляем в Serial
        Serial.print("  ");

        Serial.print(" txdata.sensorDhtTKitchen = ");
        Serial.print(txdata.sensorDhtTKitchen);
        Serial.print(" txdata.sensorDhtHKitchen = ");
        Serial.print(txdata.sensorDhtHKitchen);                 // и отправляем в Serial
        Serial.print(" txdata.sensorAlarmKitchen = ");
        Serial.print(txdata.sensorAlarmKitchen);                 // и отправляем в Serial

        Serial.println();
        delay(10);                                     // небольшая задержка, иначе неуспевает

        digitalWrite(DIR, HIGH);                      // включаем передачу
        delay(50);

        ETout.sendData();                              // отправляем
        delay(50);
        // небольшая задержка, иначе неуспевает
        digitalWrite(DIR, LOW);                       // включаем прием
      }
    }
  }
  //----------
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//			Дополнительные
//				функции
//////////////////////////////////////////////////////////////////////////////////////////////////////



