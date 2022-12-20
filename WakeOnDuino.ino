//WakeOnDuino Version 3.0 Alpha--- Ezio Cangialosi 11/10/2022

#define WIFISSID    "YOUR SSID"
#define PASSWORD    "YOUR WIFI PASSWORD"
#define MQTT_SERVER "YOUR BROKER"
#define PwrBtnP     7
#define RstBtnP     6
#define OUTTOPIC    "WoDStatus"
#define INTOPIC     "WoDCmd"
#define ERRORSTR    "Error when trying to understand message or no corresponding action."
#define AUTORSTTIME 86400000

#define VER_STR   "V3.0 Alpha" //Str for communicate version

/*-------------------------------------------------------------------
          TO ENABLE WORKVERSION, CAUTION DO NOT USE IN PROD
-------------------------------------------------------------------*/
//#define WORK_VERSION

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

long lastReconnectAttempt = 0;

void(* resetFunc) (void) = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

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

boolean reconnect() {
  Serial.print("Attempting MQTT connection...");
  // On essaye de se connecter en se nommant WakeOnDuino
  if (client.connect("WakeOnDuino")) {
    Serial.println("connected");
    //Une fois connectée on publie un msg sur le topic de status
    client.publish(OUTTOPIC,"WakeOnDuino Connected");
    client.publish(OUTTOPIC,VER_STR);
    #ifdef WORK_VERSION
    client.publish(OUTTOPIC,"Caution WorkVersion");
    #endif
    //Et on écoute sur le topic de commande
    client.subscribe(INTOPIC);
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

  Serial.println("--- WakeOnDuino ---");
  Serial.println(VER_STR);
  #ifdef WORK_VERSION
  Serial.println("Caution, WorkVersion, HIGH action on pin DEACTIVATED !");
  #endif
  
  pinMode(PwrBtnP,OUTPUT);
  pinMode(RstBtnP,OUTPUT);

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
  if(millis() > AUTORSTTIME){
    Serial.println("AutoReboot incomming in 5sec...");
    client.publish(OUTTOPIC,"AutoReboot incomming in 5sec... ");
    delay(5000);
    Serial.println("Rebooting...");
    client.publish(OUTTOPIC,"Rebooting...");
    delay(100);
    client.disconnect();
    delay(100);
    resetFunc();
  }
}

void master(String message){
  unsigned int mode=checkString(message);//Traitement de la string et renvoie d'un code
  if(mode != 0){//Si il n'y a pas eu d'erreur dans la comprehension de la string
    selectAction(mode);
  }
  else{
    client.publish(OUTTOPIC,ERRORSTR);
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
    #ifdef WORK_VERSION
    digitalWrite(RstBtnP,HIGH);
    #endif
    Serial.println("Reset du PC (Pin8-HIGH)");
    delay(100);
    digitalWrite(RstBtnP,LOW);
    client.publish(OUTTOPIC,"Reset order Received, Executed.");
  }
  else{
    #ifdef WORK_VERSION
    digitalWrite(PwrBtnP,HIGH);
    #endif
    delay(100);
    Serial.println("Allumage du PC (Pin7-HIGH)");
    digitalWrite(PwrBtnP,LOW);
    client.publish(OUTTOPIC,"SwitchOn order Received, Executed.");
  }
}

void forcedShutdown(){
  #ifdef WORK_VERSION
  digitalWrite(PwrBtnP,HIGH);
  #endif
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
  client.disconnect();
  delay(100);
  resetFunc();
}
