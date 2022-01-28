//WakeOnDuino Version 2 Beta WorkVersion --- Ezio Cangialosi 09/07/2020 sur une base d'OpenClassroom

#include <SPI.h> //bibliothèqe pour SPI
#include <Ethernet.h> //bibliothèque pour Ethernet
#include <PubSubClient.h>

const byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // tableau pour l'adresse MAC de la carte
IPAddress ip(192, 168, 1, 69); //IP Arduino
IPAddress server(192, 168, 1, 16); //IP Broker

#define PwrBtnP   7
#define RstBtnP   6
#define StateLedP 8
#define OUTTOPIC "WoDStatus"

void callback(char* topic, byte* payload, unsigned int length) {
  String msg="";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    msg +=(char)payload[i];
  }
  master(msg);
  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

long lastReconnectAttempt = 0;

boolean reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect("WakeOnDuino")) {
    Serial.println("connected");
    // Once connected, publish an announcement...
    client.publish(OUTTOPIC,"WakeOnDuino V2BetaWVer Connected");
    // ... and resubscribe
    client.subscribe("WoDCmd");
    return client.connected();
  }
  else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
  }
}

void setup() {
  Serial.begin (9600); //initialisation de communication série
  client.setServer(server, 1883);//Démarrage serveur mqtt sur port 1883
  client.setCallback(callback);//"Répéteur" de message dans console
  Ethernet.begin (mac, ip); //initialisation de la communication Ethernet

  Serial.println("WakeOnDuino Version 2.0 Beta WVersion");
  Serial.println("Caution, WorkVersion, HIGH action on pin DEACTIVATED !");
  
  pinMode(PwrBtnP,OUTPUT);
  pinMode(RstBtnP,OUTPUT);
  pinMode(StateLedP,INPUT);

  delay(1000);

  lastReconnectAttempt = 0;

  Serial.println("\nWait for MQTT Broker connection");
}

void loop(){
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  }
  else {
    // Client connected
    client.loop();
  }
}

void master(String message){
  unsigned int mode=checkString(message);
  if(mode != 0){
    selectAction(mode);
  }
}

unsigned int checkString(String message){
 Serial.println("msg :");
 Serial.print(message);
 if(message=="PiO"){
    return 1;
 }
 else if(message=="RstB"){
    return 2;
 }
 else if(message=="FsD"){
    return 3;
 }
 else{
    return 0;
 }
}

void selectAction(unsigned int mode){
  switch(mode){
    case 1:
      shortPress(0);
      break;
    case 2:
      shortPress(1);
      break;
    case 3:
      forcedShutdown();
      break;
    default:
      Serial.println("Error no action available for this mode (");
      Serial.print(mode);
      Serial.println(").");
  }
}

void shortPress(bool pinChoice){
  if(pinChoice){
    //digitalWrite(RstBtnP,HIGH);
    Serial.println("Reset du PC (Pin8-HIGH)");
    delay(100);
    digitalWrite(RstBtnP,LOW);
    client.publish(OUTTOPIC,"Reset order Received, Executed.");
  }
  else{
    //digitalWrite(PwrBtnP,HIGH);
    delay(100);
    Serial.println("Allumage du PC (Pin7-HIGH)");
    digitalWrite(PwrBtnP,LOW);
    client.publish(OUTTOPIC,"SwitchOn order Received, Executed.");
  }
}

void forcedShutdown(){
  //digitalWrite(PwrBtnP,HIGH);
  Serial.println("Coupure du PC (Pin7-HIGH)");
  delay(5000);
  digitalWrite(PwrBtnP,LOW);
  Serial.println(PwrBtnP);
  client.publish(OUTTOPIC,"Fshutdown order Received, Executed.");
}
