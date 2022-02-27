//WakeOnDuino Version 2.1 Beta WorkVersion --- Ezio Cangialosi 29/01/2022

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

void(* resetFunc) (void) = 0;

void callback(char* topic, byte* payload, unsigned int length) {//Est appelée automatiquement quand un nouveau message est détecté sur un topic
  String msg="";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    msg +=(char)payload[i];//On rentre le message dans une string pour le traiter en suite
  }
  master(msg);//Fonction de traitement principale
  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

long lastReconnectAttempt = 0;

boolean reconnect() {
  Serial.print("Attempting MQTT connection...");
  // On essaye de se connecter en se nommant WakeOnDuino
  if (client.connect("WakeOnDuino")) {
    Serial.println("connected");
    //Une fois connectée on publie un msg sur le topic de status
    client.publish(OUTTOPIC,"WakeOnDuino V2.1BetaWVer Connected");
    //Et on écoute sur le topic de commande
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
  client.setCallback(callback);//Ce qui permet de récupérer les messages entrants
  Ethernet.begin (mac, ip); //initialisation de la communication Ethernet

  Serial.println("WakeOnDuino Version 2.1 Beta WVersion");
  Serial.println("Caution, WorkVersion, HIGH action on pin DEACTIVATED !");
  
  pinMode(PwrBtnP,OUTPUT);
  pinMode(RstBtnP,OUTPUT);
  pinMode(StateLedP,INPUT);

  delay(1000);

  lastReconnectAttempt = 0;

  Serial.println("\nWait for MQTT Broker connection");
}

void loop(){
  if (!client.connected()) {//Si on n'est pas connecté au Broker
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {//Un essai toutes les 5secs
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
  unsigned int mode=checkString(message);//Traitement de la string et renvoie d'un code
  if(mode != 0){//Si il n'y a pas eu d'erreur dans la comprehension de la string
    selectAction(mode);
  }
  else{
    String errorStr="Error when trying to understand message or no corresponding action, message recieved : "+message;
    client.publish(OUTTOPIC,errorStr);
  }
}

unsigned int checkString(String message){
 if(message=="PiO"){
    return 1;
 }
 else if(message=="RstB"){
    return 2;
 }
 else if(message=="FsD"){
    return 3;
 }
 else if(message=="Ping"){
    return 4;
 }
 else if(message=="SelfReboot"){
    return 5;
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
    case 4:
      receivedPing();
      break;
    case 5:
      selfReboot();
      break;
    default:
      Serial.println("Error no action available for this mode (");
      Serial.print(mode);
      Serial.println(").");
  }
}

void shortPress(bool pinChoice){//PinChoice pour sélectionner le bon pin (7-8 --> 0-1)
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

void receivedPing(){
  Serial.println("Ping reçu");
  delay(500);
  client.publish(OUTTOPIC,"Ping received, Pong");
  Serial.println("Pong renvoyé");
}

void selfReboot(){
  Serial.println("Ordre de redémarrage reçu\nExecution dans 5 secondes");
  client.publish(OUTTOPIC,"Self Reboot order Received, WakeOnDuino will reboot in 5 seconds");
  delay(5000);
  Serial.println("Rebooting...");
  client.publish(OUTTOPIC,"Rebooting, See you soon world !");
  delay(100);
  resetFunc();
}
