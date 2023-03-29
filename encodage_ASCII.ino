#include <IRremote.hpp>


#define IR_RECEIVE_PIN 11 // Pin de réception
#define ENABLE_LED_FEEDBACK 13 // LED témoin
#define IR_SEND_PIN 3 // Pin d'émission


unsigned long previousTime; //Temps de référence pour le FAIL
String Message_recu = ""; // Chaine intiale
unsigned int taille_msg; // Taille du message
int Delta=4000; // Délai réglable avant annonce d'une erreur : FAIL
bool Flag_Envoi = true; // Flag permettant de sortie de la boucle si erreur

void setup()
{
  Serial.begin(9600); // Début de la communication
  IrSender.begin(IR_SEND_PIN); // Initialisation de l'envoi
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Initialisation du receveur
  Serial.println("Communication établie\n"); // Message de connexion réussie
}


  
void loop() {

  if (Serial.available() > 0){ // Si le moniteur série est rempli
    String a = Serial.readString(); // Lire la chaine
    for (int i = 0; i < a.length(); i += 4) { // incrémente i de 4 à chaque itération
    if (i == 0) {
        IrSender.sendNECRaw(300); //Code de préparation au message
        Serial.print("Envoi de : ");
        Serial.print(a);
        Serial.print(" ... ");
        delay(8); // Attente nécessaire pour laisser le temps de décoder
        IrSender.sendNECRaw(a.length()); // Envoi de la taille du message
        delay(8); // Attente nécessaire pour laisser le temps de décoder
    }
    uint32_t message = 0;
    for (int j = 0; j < 4 && (i + j) < a.length(); j++) { // Combinaison de 4 caractères en un uint32_t
        message |= ((uint32_t)a.charAt(i + j) << (8 * j)); // Concaténation des 4 caractères dans la trame
    }
    IrSender.sendNECRaw(message); // Envoi de 4 caractères combinés en un uint32_t
    delay(8); // Attente nécessaire pour laisser le temps de décoder
}
  IrReceiver.resume(); //Remise à zéro des paramètres de décodage
  Serial.println(" Message envoyée");

}
  if (IrReceiver.decode()){ // Si une trame a été décodé
     if(IrReceiver.decodedIRData.decodedRawData == 300){ //Si le header est détecté
          Serial.println("Récepetion ...");
          IrReceiver.resume();
          String Message="";
          previousTime = millis(); // Initialisation temps de référence
          int j = 0;
          bool Flag = true;
          unsigned int reste; // Initialisation nombre de caractères restant à décoder
          while (j != taille_msg+1 && Flag){ // Tant qu'il reste des caractères à décoder ou le temps n'est pas écoulé
              if(millis()-previousTime > Delta){ // Temps trop long 
                Serial.print("Fail -> Message décodé : ");
                Serial.println(Message);
                Flag = false;
              }
              if (IrReceiver.decode()){ // Si une autre trame est décodée
                if(j==0){
                taille_msg = IrReceiver.decodedIRData.decodedRawData; // Initialisation de le taille du message
                j++;
                reste = taille_msg;;
              }  
              else if(j != taille_msg+1 && reste >= 4){ // Si il reste encore plus de 4 caractères
                Message += uint32_to_string(IrReceiver.decodedIRData.decodedRawData, 4); // Décodage 4 par 4
                j += 4;
                reste -= 4;
              }
              else if(j != taille_msg+1 && reste < 4){ // Si il reste moins de 4 caractères
                Message += uint32_to_string(IrReceiver.decodedIRData.decodedRawData, reste); // Décodage du reste
                j += reste ;
              }
              IrReceiver.resume(); // Remise à zéro
            }
          }
          if(Flag == true){  // Si pas de problème
            /*Serial.print("Durée du message : "); // Partie utilisé pour la vitesse de décodage
            Serial.print(millis()- previousTime);
            Serial.println(" ms");*/
            Serial.print("Message valide -> ");
            Serial.println(Message);
          }
           Serial.println("\nPrêt à recevoir...\n");
     }
       IrReceiver.resume(); // Remise à zéro
  }
}
  
String uint32_to_string(uint32_t value, unsigned int b) { // Fonction décodage
    char buffer[5] = {0};
    String msg="";
    for(int i = 0; i <b ; i++){
        buffer[3 - i] = (char) (value >> (i*8));
        msg += buffer[3 - i];
    }
    return msg;
}