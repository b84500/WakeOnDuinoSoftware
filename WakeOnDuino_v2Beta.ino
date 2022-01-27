//WakeOnDuino Version 2 Beta WorkVersion --- Ezio Cangialosi 09/07/2020 sur une base d'OpenClassroom

#include <SPI.h> //bibliothèqe pour SPI
#include <Ethernet.h> //bibliothèque pour Ethernet
#include <PubSubClient.h>

const byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // tableau pour l'adresse MAC de la carte
IPAddress ip(192, 168, 1, 69); //IP Arduino
IPAddress server(192, 168, 1, 16); //IP Broker

#define PwrBtnP   7;
#define RstBtnP   6;
#define StateLedP 8;

bool flag1=false;
bool flag2=false;
bool AddCookie=false;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

long lastReconnectAttempt = 0;

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect("WakeOnDuino")) {
    Serial.println("connected");
    // Once connected, publish an announcement...
    client.publish("WoDStatus","WakeOnDuino Connected");
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
  
  pinMode(PwrBtnP,OUTPUT);
  pinMode(RstBtnP,OUTPUT);
  pinMode(StateLedP,INPUT);

  delay(1000);

  lastReconnectAttempt = 0;

  Serial.println("\nWait for MQTT Broker connection");

}

void loop(){
  if(!client.connected()){
    long now = millis();
    if (now - lastReconnectAttempt > 5000){
      lastReconnectAttempt = now;
      //Attempt to reconnect
      if(reconnect()){
        lastReconnectAttempt = 0;
      }
      else{
        //Client connected
        client.loop();
      }
    }
  }
}

void oldloop() {
  EthernetClient client = serveur.available(); //on écoute le port
  if (client) { //si client connecté
    Serial.println("Client en ligne"); //on le dit...
    if (client.connected()&& (millis()>=time_btw+1000 || time_btw==0)) { // si le client est connecté
      if(AddCookie){
        SetCookies(client);
        flag2=false;
        AddCookie=false;
      }
      else if(!AddCookie){
        if(!flag1){
          verifMDP(client);
          if(Granteed){
            CPanel(client);
            PiedP(client);
            time_btw=millis();
          }
          else if(!Granteed){
            MsgFalse(client);
          }
        }
        else if(flag1){
          if(Granteed){
            GET(client);
            CPanel(client);
            PiedP(client);
          }
          else if(!Granteed){
            MsgFalse(client);
          }
        }
      }
    }
    else if (client.connected() && millis()<=time_btw+1000 && time_btw!=0){
      PoubelleBuffer(client);
    }
  }
  if(millis()>=time_logout+30000 && flag1){
    Granteed=false;
    time_btw=0;
    flag1=false;
    Serial.println("Verrouilage de l'interface");
  }
  while(Serial.available()){
    char c = Serial.read();
    delay(1);
    if(c=='A'){
      delay(1);
      c=Serial.read();
      if(c=='d'){
        delay(1);
        c=Serial.read();
        if(c=='d'){
          delay(1);
          c=Serial.read();
          if(c=='C'){
            AddCookie=true;
            flag2=true;
            time_cookies=millis();
            Serial.println("Mode Ajout de Cookies Activé pdt 30sec");
          }
        }
      }
    }
    else if(!(Serial.available()) && !flag2){
      AddCookie=false;
      Serial.println("Message Faux ou Incompris");
    }
  }
  if(millis()>=time_cookies+30000 && flag2){
    AddCookie=false;
    flag2=false;
    Serial.println("Mode Ajout de Cookies désactivé");
  }
}
void verifMDP(EthernetClient cl) {
  Serial.println("Recherche du Cookie de Connexion");
  bool lu = 0; //variable pour indiquer l'état de lecture
  while (cl.available()) { // tant qu'il a des infos à transmettre
    char c = cl.read(); // on lit le caractère
    delay(1); //delai de lecture
    if (c == 'C') { //si "C" repéré
      c = cl.read(); //on lit le caractère suivant qui contient la donnée
      if (c == 'o') { //si code reçu
        c = cl.read();
        if (c=='o'){
          c = cl.read();
          if(c=='k'){
            c=cl.read();//Allumer
            if(c=='i'){c = cl.read(); if(c=='e'){c = cl.read(); if(c==':'){c = cl.read(); if(c==' '){
              c = cl.read();
              Serial.println("Lecture du Cookie de Connexion en cours");
              if(c=='L'){c = cl.read(); if(c=='o'){c = cl.read(); if(c=='g'){c = cl.read(); if(c=='i'){c = cl.read(); if(c=='n'){c = cl.read(); if(c=='='){c = cl.read();
                if(c==password[0]){c=cl.read();if(c==password[1]){c=cl.read();if(c==password[2]){c=cl.read();if(c==password[3]){c=cl.read();if(c==password[4]){c=cl.read();if(c==password[5]){c=cl.read();if(c==password[6]){c=cl.read();if(c==password[7]){c=cl.read();if(c==password[8]){c=cl.read();if(c==password[9]){c=cl.read();if(c==password[10]){c=cl.read();
                if(c==password[11]){
                Granteed=true;
                flag1=true;
                time_logout=millis();
                Serial.println("Cookie de Connexion valide");
            }}}}}}}}}}}}
          }}}}}}
        }}}}
      }
     }
    }
   }
   else if(!(cl.available()) && !flag1){
    Granteed=false;
    Serial.println("Cookie de Connection Absent ou Faux");
   }
  }
}
void CPanel(EthernetClient cl){
  //réponse au client
  entetePanel(cl);
  cl.println("<h2>Computer State: ");
  /*
  PcState();
  if (!digitalRead(StateLedP)!OnState && !SleepState){
    cl.println("Off</h2>");
  }
  else if (digitalRead(StateLedP) OnState && !SleepState){
    cl.println("On</h2>");
  }
  else if (!OnState && SleepState){
    cl.println("En Veille</h2>");
  }*/
  cl.println("Information not Available</h2>");
  cl.println("<p><a href=/?PiO><input id=Menu type=button value=SwitchOn><a/></p>");
  cl.println("<p><a href=?FsD><input id=Menu type=button value=Forced-shutdown><a/> Caution ! To be used only in case of force majeure, please use the windows shutdown</p>");
  cl.println("<p><a href=?RstB><input id=Menu type=button value=ResetButton><a/> Caution ! To be used only in case of force majeure, please use the windows reset</p>");
}

void PcState(){
  OnState=false;
  SleepState=false;
  if (digitalRead(StateLedP)){
    OnState=true;
    SleepState=false;
  }
  else if (!digitalRead(!StateLedP)){
    OnState=false;
    SleepState=false;
  }
}

//fonction d'affichage de l'entête HTML
void entetePanel(EthernetClient cl) {
  cl.println("HTTP/1.1 200 OK");
  cl.println("Content-Type: text/html; charset=ascii");
  cl.println("Connection: close");
  cl.println();
  cl.println("<!DOCTYPE HTML>");
  cl.println("<html>");
  cl.println("<head><meta charset=UTF=8/><title>WakeOnDuino</title><link rel=icon type=image/png href=https://cutt.ly/Ggfj85w/></head>");
  cl.println("<body><h1>WakeOnDuino Control Panel</h1><hr><br>");
}
void MsgFalse(EthernetClient cl) {
  cl.println("HTTP/1.1 401 Unauthorized");
  Serial.println("Error 401, fin de communication");
  cl.println("Content-Type: text/html; charset=ascii");
  cl.println("Connection: close");
  cl.stop(); //on déconnecte le client
}
void SetCookies(EthernetClient cl) {
  cl.println("HTTP/1.1 200 OK");
  cl.println("CContent-Type: text/html; charset=ascii");
  cl.print("Set-cookie: Login=");
  cl.print(password[0]);
  cl.print(password[1]);
  cl.print(password[2]);
  cl.print(password[3]);
  cl.print(password[4]);
  cl.print(password[5]);
  cl.print(password[6]);
  cl.print(password[7]);
  cl.print(password[8]);
  cl.print(password[9]);
  cl.print(password[10]);
  cl.print(password[11]);
  cl.println("; Expires=Fri, 31 Dec 2021 23:59:59 GMT");
  Serial.println("Cookie de Connexion crée chez le client");
  cl.println("Connection: close");
  cl.stop(); //on déconnecte le client
  Serial.println("Fin de communication avec le client\n");
}
//fonctin décodage GET
void GET(EthernetClient cl) {
  Serial.println("Lecture des infos GET");
  bool lu = 0; //variable pour indiquer l'état de lecture
  while (cl.available()) { // tant qu'il a des infos à transmettre
    char c = cl.read(); // on lit le caractère
    delay(1); //delai de lecture
    if (c == '?' && lu == 0) { //si "?" repéré
      c = cl.read(); //on lit le caractère suivant qui contient la donnée
      if (c == 'P') { //si code reçu
        c = cl.read();
        if (c=='i'){
          c = cl.read();
          if(c=='O'){ //Allumer
            digitalWrite(PwrBtnP,HIGH);
            delay(100);
            Serial.println("Allumage du PC (Pin7-HIGH)");
            digitalWrite(PwrBtnP,LOW);
            Granteed = false;
          }
        }
      }
      else if (c == 'F'){
        c = cl.read();
        if (c=='s'){
          c = cl.read();
          if(c=='D'){
            digitalWrite(PwrBtnP,HIGH);
            Serial.println("Coupure du PC (Pin7-HIGH)");
            delay(5000);
            digitalWrite(PwrBtnP,LOW);
            Granteed = false;
          }
        }
      }
      else if (c=='R'){
        c = cl.read();
        if (c=='s'){
          c = cl.read();
          if(c=='t'){
            c = cl.read();
            if (c=='B'){
              digitalWrite(RstBtnP,HIGH);
              Serial.println("Reset du PC (Pin8-HIGH)");
              delay(100);
              digitalWrite(RstBtnP,LOW);
              Granteed = false;
            }
          }
        }
      }
      delay(10);
      lu = 1; // on dit qu'on a lu l'info
    }
    Serial.println("Aucune Info GET ou Infos GET Incomprises ou Inconnues");
  }
}
void PoubelleBuffer(EthernetClient cl){
  while (cl.available()) { // tant qu'il a des infos à transmettre
    char c = cl.read(); // on lit le caractère
    delay(1); //delai de lecture
  }
  cl.println("HTTP/1.1 409 Conflict");
  Serial.println("Error 409, requête trop rapide");
  cl.println("Content-Type: text/html; charset=ascii");
  cl.println("Connection: close");
  cl.stop(); //on déconnecte le client
}

void PiedP(EthernetClient client){
  client.println("<br><hr><p>Version 1.0 Beta</p>");
  client.println("<p>Please remember to log out !</body></html>"); //ligne horizontale et fermeture des balises
  client.stop(); //on déconnecte le client
  Serial.println("Fin de communication avec le client\n");
}
