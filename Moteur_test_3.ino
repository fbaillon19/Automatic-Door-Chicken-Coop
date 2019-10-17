/***************************************************************************
 * Porte_poulailler
 * 
 * Le programme permet le contrôle d'un moteur qui actionne une porte
 * d'ouverture et de fermeture d'un poulailler.
 * L'ouverture se produit à une heure donnée et la fermeture lorsque la
 * nuit est tombée.
 * 
 * Author : Frédéric BAILLON
 * Version : 1.0
 * Date : 7 octobre 2019
 * 
 **************************************************************************/

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"

/** Registres */
#define IN1  6      // Commande de rotation pour le moteur sur la broche D6
#define IN2  7      // Commande de rotation pour le moteur sur la broche D7

#define STOP_HAUT   4   // Capteur de fin de course haut sur la broche D4
#define STOP_BAS    3   // Capteur de fin de course bas sur la broche D3

/** Constantes */
#define HEURE_LEVER          7  // Heure pour l'ouverture de la porte
#define MINUTE_LEVER        45  // Minute pour l'ouverture de la porte
#define HEURE_COUCHER       19  // Heure pour la fermeture de la porte
#define MINUTE_COUCHER      50  // Minute pour l'ouverture de la porte
#define HEURE_MIN_COUCHER   19  // Heure minimum pour la fermeture de la porte
#define NB_LECTURE_MAX      10  // Nombre de lectures pour valider que la nuit
                                // est tombée
#define SEUIL_PENOMBRE  1000    // Seuil pour valider la baisse de luminosité

/** Déclaration des variables */
RTC_DS3231 rtc;             // Module pour la gestion du temps

int nbLecture;              // Compte le nombre de lectures pour la détection
                            // de la nuit, pour éviter un déclenchement intempestif
int seuilPenombre;

/*
 * Fonction de préparation de l'arduino
 */
void setup() {
  
  // Démarrage de la liaison série avec l'ordinateur
  Serial.begin(9600);

  // Programmation des broches utilisées de l'arduino
  pinMode(IN1, OUTPUT);         // En écriture
  pinMode(IN2, OUTPUT);         // En écriture
  
  pinMode(STOP_HAUT, INPUT_PULLUP);    // En lecture
  pinMode(STOP_BAS, INPUT_PULLUP);     // En lecture

  // Démarre le module RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  
  Serial.println(digitalRead(STOP_HAUT));
  Serial.println(digitalRead(STOP_BAS));
}

/*
 * Fonction 
 */
void loop() {

DateTime maintenant = rtc.now();

int seuilDetecte;

  seuilPenombre = analogRead(A1);
  Serial.print("Seuil programmé = ");
  Serial.println(seuilPenombre);
  seuilDetecte = analogRead(A0);
  Serial.print("Seuil détecté = ");
  Serial.println(seuilDetecte);

  if(analogRead(A0) >= seuilPenombre){
    Serial.println("Nuit");
  }
  else{
    Serial.println("Jour");
  }
    
  affichage(maintenant);

  // Se déclenche à un temps donné
  if(coqChante(maintenant)){
    ouverturePorte();
  }
  
  // En tout état de cause, la porte ne peut pas se fermer avant une heure donnée.
  // Cela permet que la porte ne se ferme pas en journée lors d'une eclipse mais
  // surtout lorsque le jour se lève tard en hiver
  if(maintenant.hour() >= HEURE_MIN_COUCHER){
    if(soleilCouche(maintenant)){     // Est-ce que la luminosité a suffisamment baissée ?
      fermeturePorte();
    }
  }
  
  delay(5000);             // attente de 5 secondes avant de relancer la vérification 
}

/*
 * 
 */
bool coqChante(DateTime now){

  if(now.hour() == HEURE_LEVER && now.minute() == MINUTE_LEVER)
    return 1;
  
  return 0;
}

/*
 * Détecte que la luminosité a baissé, ce qui peut indiquer la nuit
 */
bool soleilCouche(DateTime now){

/*  if(analogRead(A0) >= seuilPenombre){
    Serial.print("Nuit (");
    Serial.print(nbLecture);
    Serial.println(")");
    if(nbLecture++ >= NB_LECTURE_MAX){
      nbLecture = 0;   
      return 1;
    }
  }
  else
    nbLecture = 0;
*/
  if(now.hour() == HEURE_COUCHER && now.minute() == MINUTE_COUCHER)
    return 1;
    
   return 0;
}


/*
 * Fonction pour fermer la porte. Le moteur s'arrête lorsque le capteur
 * détecte la position basse de la porte
 */
void fermeturePorte() {

  Serial.println("Fermeture porte");

  // Programmation des broches pour la rotation droite du moteur
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // Tourne dans la boucle tant que la porte n'est pas complètement fermée
  while(digitalRead(STOP_BAS) == HIGH){};

  arretMoteur();      // Arrête le moteur
}

/*
 * Fonction pour ouvrir la porte. Le moteur s'arrête lorsque le capteur
 * détecte la position haute de la porte
 */
void ouverturePorte() {

  Serial.println("Ouverture porte");
    
  // Programmation des broches pour la rotation gauche du moteur
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  
  // Tourne dans la boucle tant que la porte n'est pas complètement ouverte
  while(digitalRead(STOP_HAUT) == HIGH){};

  arretMoteur();      // Arrête le moteur
}

/*
 * Fonction pour arrêter le moteur
 */
void arretMoteur() {
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void affichage(DateTime now){
  
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}
