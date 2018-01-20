#include <OneWire.h>                    // Библиотека протокола 1-Wire
#include <DallasTemperature.h>          // Библиотека для работы с датчиками DS*
#define ONE_WIRE_BUS 9                 // Шина данных на 9 пине для датчиков DS18B20 (ардуино UNO)
#define TEMPERATURE_PRECISION 9         // Точность измерений в битах (по умолчанию 12)
OneWire oneWire(ONE_WIRE_BUS);            // Создаем экземпляр объекта протокола 1-WIRE - OneWire
DallasTemperature sensorsDS(&oneWire);    // На базе ссылки OneWire создаем экземпляр объекта, работающего с датчиками DS*
// адрес датчика DS18B20 на входе в коллектор:
DeviceAddress sensorDsCollectorIn = { 0x28, 0xFF, 0x74, 0xF0, 0xB2, 0x16, 0x03, 0x1C };  
// адрес датчика DS18B20 на выходе из коллектора:
DeviceAddress sensorDsCollectorOut = { 0x28, 0xFF, 0x57, 0x1E, 0xB3, 0x16, 0x04, 0xDE };  
// адрес датчика DS18B20 на трубе в прихожую:
DeviceAddress sensorDsKitchenOut1 = { 0x28, 0xFF, 0x63, 0x89, 0x01, 0x17, 0x05, 0x4B };   
// адрес датчика DS18B20 на трубе из прихожей:
DeviceAddress sensorDsKitchenOut2 = { 0x28, 0xFF, 0x14, 0x17, 0x01, 0x17, 0x03, 0x5E };  

int sensorDsTempCollectorIn;   // температура датчика DS18B20 на входе в коллектор
int sensorDsTempCollectorOut;  // температура датчика DS18B20 на выходе из коллектора
int sensorDsTempKitchenOut1;    // температура датчика DS18B20 на трубе  в прихожую
int sensorDsTempKitchenOut2;    // температура датчика DS18B20 на трубе из прихожей

//-------------------------------------------------
#include <SoftEasyTransfer.h>
#include <SoftwareSerial.h>
#define DIR 13            // переключатель прием\передача на Pin13

SoftwareSerial mySerial(2, 3);      // Serial для RS485
SoftEasyTransfer ETin, ETout;  //create two objects

int ID = 2;              // номер этой ардуины
struct SEND_DATA_STRUCTURE {                  // структура, которую будем передавать
  int ID;
  int sensorDsKitchenOut1;    // адрес датчика DS18B20 на трубе в прихожую
  int sensorDsKitchenOut2;    // адрес датчика DS18B20 на трубе из прихожей
  int sensorDsCollectorIn;   // адрес датчика DS18B20 на входе в коллектор
  int sensorDsCollectorOut;  // адрес датчика DS18B20 на выходе из коллектора
};

struct RECEIVE_DATA_STRUCTURE {         // структура, которую будем принимать
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int ID;
  int action;
  int targetPin;
  int levelPin;
};

int action = 0;    //  0/1 ("get"/"set") - запрос выдачи данных / команда на исполнение
int levelPin = 0;   //  0/1 - установка уровня на Pin (LOW/HIGH)
int targetPin;         //  Pin управления

//give a name to the group of data
RECEIVE_DATA_STRUCTURE rxdata;
SEND_DATA_STRUCTURE txdata;
//-----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  //----------
  mySerial.begin(9600); // start serial port
  //  //start the library, pass in the data details and the name of the serial port
  ETin.begin(details(rxdata), &mySerial); 
  ETout.begin(details(txdata), &mySerial);

  pinMode(DIR, OUTPUT);
  delay(50);
  digitalWrite(DIR, LOW);     // включаем прием
  //----------
  sensorsDS.begin(); // Запускаем поиск всех датчиков DS18B20 на шине

  //------------

}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

void loop() {
  //----------------------------------
  digitalWrite(DIR, LOW);                       // включаем прием

  sensorsDS.requestTemperatures(); // Запускаем процесс измерение температуры на всех датчиках DS18B20
  delay(30);
  // температура датчика DS18B20 на трубе в прихожую:
  sensorDsTempKitchenOut1 = sensorsDS.getTempC(sensorDsKitchenOut1);      
  // температура датчика DS18B20 на трубе из прихожей: 
  sensorDsTempKitchenOut2 = sensorsDS.getTempC(sensorDsKitchenOut2);     
  // температура датчика DS18B20 на входе в коллектор:
  sensorDsTempCollectorIn = sensorsDS.getTempC(sensorDsCollectorIn);     
  // температура датчика DS18B20 на выходе из коллектора:
  sensorDsTempCollectorOut = sensorsDS.getTempC(sensorDsCollectorOut);  
  //-------------
  if (ETin.receiveData()) {                         // если пришли данные
    byte id = rxdata.ID; // читаем байт, в нем для кого этот пакет

    if (id == ID) { // и если пакет пришел нам
 
       if (rxdata.action == 1) {   //  и если команда управления
        targetPin = rxdata.targetPin;
        levelPin = rxdata.levelPin;
        pinMode(targetPin, OUTPUT);
           
        if (rxdata.levelPin == 1) {   //  то управляем реле

        delay(50);
 //       analogWrite(targetPin, levelPin);
          digitalWrite(targetPin, LOW);   // включаем реле 
           }
        else
        {
            digitalWrite(targetPin, HIGH);    // выключаем реле
        }

      }
      else //  иначе включаем передачу данных
      {
        //      ID = 2;  // номер этой ардуины
        txdata.ID = ID;
        txdata.sensorDsKitchenOut1 = sensorDsTempKitchenOut1;    // температура датчика DS18B20 на трубе в прихожую
        txdata.sensorDsKitchenOut2 = sensorDsTempKitchenOut2;    // температура датчика DS18B20 на трубе из прихожей
        txdata.sensorDsCollectorIn = sensorDsTempCollectorIn;   // температура датчика DS18B20 на входе в коллектор
        txdata.sensorDsCollectorOut = sensorDsTempCollectorOut;  // температура датчика DS18B20 на выходе из коллектора

        digitalWrite(DIR, HIGH);                      // включаем передачу
        delay(50);

        ETout.sendData();                              // отправляем
        delay(50);
        // небольшая задержка, иначе неуспевает
        /////////////////
        Serial.print(" ID = ");
        Serial.print(txdata.ID);                 // и отправляем в Serial
        Serial.print("  ");

        Serial.print(" txdata.sensorDsCollectorIn = ");
        Serial.print(txdata.sensorDsCollectorIn);
        Serial.print(" txdata.sensorDsCollectorOut = ");
        Serial.print(txdata.sensorDsCollectorOut);                 // и отправляем в Serial
        Serial.print(" txdata.sensorDsKitchenOut1 = ");
        Serial.print(txdata.sensorDsKitchenOut1);                 // и отправляем в Serial
        Serial.print(" txdata.sensorDsKitchenOut2 = ");
        Serial.print(txdata.sensorDsKitchenOut2);

        Serial.println();
        delay(10);                                     // небольшая задержка, иначе неуспевает
      }
    }
  }
  //----------
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//              Дополнительные
//                функции
//////////////////////////////////////////////////////////////////////////////////////////////////////

