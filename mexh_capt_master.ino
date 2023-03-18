
#include "painlessMesh.h"
#include "DHTesp.h"
#include "heltec.h"

DHTesp dht;

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

float currentTemp;
float currentHumidity;


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


String msg1="";
String msg2="";

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = "Hi from node1";
  msg += mesh.getNodeId();
  String msgtot=msg1+msg2;
  mesh.sendBroadcast( msgtot );


taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}



// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
   Heltec.begin(true );//DisplayEnable Enable/, false /LoRa Disable/, true /Serial Enable/);
   Heltec.display->clear();
   Heltec.display->drawString(0, 0,String(msg)) ;
   Heltec.display->display();
   }

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}




void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}



void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}




void displayReadingsOnOled() {String msg = "Hi from node1";
  msg += mesh.getNodeId();
  
   float currentTemp = dht.getTemperature();
   float currentHumidity = dht.getHumidity();
   
  msg1 ="\nTemperature: " + (String)currentTemp +  "°C \n";
  msg2 = "Humidity: " + (String)currentHumidity + "%";
 
  // Clear the OLED screen
  Heltec.display->clear();
  Heltec.display->drawString(0, 0,String(msg)) ;
  Heltec.display->drawString(0,12,String(msg1)) ;
  Heltec.display->drawString(0, 24,String(msg2)) ; 
  Heltec.display->display();
}

void setup() {
  
  dht.setup(17, DHTesp::DHT22);
  currentTemp = dht.getTemperature();
  currentHumidity = dht.getHumidity();
 
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
 
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, false /*Serial Enable*/);
  displayReadingsOnOled();
  Serial.begin(115200);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop(){
    displayReadingsOnOled();
    // it will run the user scheduler as well
    mesh.update();
  }
