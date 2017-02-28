#include <Servo.h> 


//Control per llum 
const int ambient = 30; // Nivell llum ambient
const int umbral = 200;  // Diferencia entre llum fotocel.lules
const int umbralFars = 800; //Nivell minim ambient per encendre leds fars
int lightLeft = 0;
int lightRight = 0;
const int lightLeftPin = A1; //fotocel.lula esquerra
const int lightRightPin = A0;//fotocel.lula dreta

//Control distancia fixa
const int echoPin = 4; // taronja ,
const int trigPin = 5; // gris
long duration, distance;

//Control per paraxocs

const int bumpRight = 2; // Right bumper pin 2, groc
const int bumpLeft = 3;  // Left bumper pin 3, blau
int pbLeft = 0;          // Var for left bump
int pbRight = 0;         // Var for left bump

//Deteccio desnivells
const int ifrCentre = A2; // Infraroig centre, led:groc, fototrans:verd, A2:blanc
const int ifrLeft = A3;   // Gris
const int ifrRight = A4;  // Negre
const int ifrRear = A5;   // Violeta
int lecturaInicialCentre, lectInLeft, lectInRight, lectInRear = 0; 
int lectFiCentre, lectFiLeft, lectFiRight, lectFiRear = 0;
int difCentre, difLeft, difRight, difRear = 0;
int difLectures = 200;
int difLectRear = 200;

//Control distancia Torreta
const int echoPinTorr = 6; // blanc                           
const int trigPinTorr = 7; // groc
long durationTorr, distanceTorr =100;
int i = 90;  //variable per comptar moviment torreta

//Control temps programa
int intervalLectIfr = 3000; //llegir valor Ifr cada 3 segons
unsigned long time;
unsigned long previousTime;
unsigned long difTime;

//Definicio servos
Servo servoLeft;         // Define left servo
Servo servoRight;        // Define right servo
Servo servoTorreta;      // Define servo torreta
const int servoLeftPin = 9;  //blau
const int servoRightPin = 10;//groc
const int servoTorrPin = 8;  //verd

//Variables LEDS
const int ledPinFront = 13;  // LEDS frontals, cable blau.
const int ledPinFars = 12;  //  LEDS fars paraxocs, cable lila
const int ledPinRear = 1;    // LEDS posteriors vermells. Funcionen al anar enrrera
unsigned long timeLedFront, difTimeFront, prevTimeFront;
unsigned long timeLedFar, difTimeFar, prevTimeFar;
unsigned long timeLedFrontF, difTimeFrontF, prevTimeFrontF;


//Variables buzzer
void nota(int a, int b);
int spk=11;  // color marro
unsigned long difTimeSo, timeSo, previousTimeSo;
//==========================================================================================================
void setup() {
  //Serial.begin(9600); //Anul.lat per utilitzar el pin nº1
  servoLeft.attach(servoLeftPin);   // Left servo pin D9, blau
  servoRight.attach(servoRightPin); // Right servo pin D10, groc
  servoTorreta.attach(servoTorrPin); //servo torreta pin D8, 
  servoTorreta.write(90);
  
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  pinMode(trigPinTorr,OUTPUT);
  pinMode(echoPinTorr,INPUT);
  pinMode(bumpLeft, INPUT);  
  pinMode(bumpRight, INPUT);  
  pinMode(ledPinFront, OUTPUT);
  pinMode(ledPinFars, OUTPUT);
  pinMode(ledPinRear, OUTPUT);
  soInici();
} 

//===========================================================================================================
void loop() {
  
  lightLeft=analogRead(lightLeftPin); //blau
  lightRight=analogRead(lightRightPin);//groc
  reAttach();
  /*Serial.print("llumLeft: ");
  Serial.println(lightLeft);
  Serial.print("llumRight: ");
  Serial.println(lightRight);*/
 
  if (lightRight<ambient || lightLeft<ambient) {
    llanterna();
    turnRight();
    delay (2000);
    stopRobot();
    delay(1000);
  }
  else {
    forward();    // Endevant
    paraxocs();   // Proteccio cops
    llanterna();  // Seguiment de llum
    llumsFars();
  }
}    
//============================================================================================================

// Rutines de moviment
void forward() {   //Endevant
  reAttach();
  digitalWrite(ledPinRear,LOW);
  servoLeft.writeMicroseconds(1470);
  servoRight.writeMicroseconds(1580);
  servoTorreta.write(90);
  LedsFront();
  calculDistancia();
  tempsLecturaIfr();
  deteccioForats();
  tempsSo();
}

void reverse() {   //Enrere
  reAttach();
  servoLeft.write(102);
  servoRight.write(88);
}

void turnRight() {  //Dreta
  reAttach();
  servoLeft.write(88);
  servoRight.write(88);
}

void turnLeft() {   //Esquerra
  reAttach();
  servoLeft.write(102);
  servoRight.write(102);
}

void stopRobot() {    //Parada
  servoLeft.detach();
  servoRight.detach();
}

void reAttach() {  // Reconectar
  if(servoLeft.attached()== false)
    servoLeft.attach(9);
  if(servoRight.attached() == false)
    servoRight.attach(10);
}
//==========================================================================================================
void paraxocs(){    
  pbLeft = digitalRead(bumpLeft);  
  pbRight = digitalRead(bumpRight);
  
  // Si left bumper colpeja
  if (pbLeft == HIGH) {
    //endevant = 0;
    //LedsFrontFast();             // Control LED indicador
    reverse();
    digitalWrite(ledPinRear,HIGH);
    delay(500); 
    turnRight();
    delay(400);
    digitalWrite(ledPinRear,LOW);
  }  
  // Si right bumper colpeja
  if (pbRight == HIGH) {
    reverse();
    digitalWrite(ledPinRear,HIGH);
    delay(500); 
    turnLeft();
    delay(400);
    digitalWrite(ledPinRear,LOW);
  }
}
//===========================================================================================================
void llanterna(){ 
  if ((lightRight - lightLeft) > umbral) {  // Rutina per seguir una 
    turnRight();                            // llum, una llanterna, per ex.
    delay(500);
    forward();
    delay(1000);
  }
  
  if (lightLeft - lightRight > umbral) {
    turnLeft();
    delay(500);
    forward();
    delay(1000);
  }
}  
//=============================================================================================================

void calculDistancia() {
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2); // Added this line    
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;//29,1=1/0,0343 // vel sonido=343m/s = 0,0343 cm/microseg
  if (distance < 15) {
    //endevant = 0;
    //LedsFrontFast();  
    stopRobot();
    digitalWrite(ledPinRear,HIGH);
    delay(200);
    torretaScan();
    digitalWrite(ledPinRear,LOW);
  }
}  
//============================================================================================================
void torretaScan() {
  for (i=90;i>10;i=i-5){
    servoTorreta.write(i);
    delay(50);
  }  
  
  calculDistTorreta();
  while (distanceTorr <70){
    i=i+10;
    servoTorreta.write(i);
    delay(200);
    calculDistTorreta();
    if (i>165){
      reverse();
      digitalWrite(ledPinRear,HIGH);
      delay(1000);
      for (i;i>=90;i=i-5){
        servoTorreta.write(i);
        delay(50);
      }
      digitalWrite(ledPinRear,LOW);  
    }
  }  
  if (distanceTorr >= 70){
    if (i < 90){
      turnRight();
      delay(50);
      for (i;i<=90;i=i+5){
        servoTorreta.write(i);
        delay(50);
      }  
    }
    else {
      turnLeft();
      delay(50);
      for (i;i>=90;i=i-5){
        servoTorreta.write(i);
        delay(50);
      }
    }  
  }
}  
//=============================================================================================================      

void calculDistTorreta() {
  digitalWrite(trigPinTorr,LOW);
  delayMicroseconds(2); // Added this line    
  digitalWrite(trigPinTorr, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPinTorr, LOW);
  
  durationTorr = pulseIn(echoPinTorr, HIGH);
  distanceTorr = (durationTorr/2) / 29.1;//29,1=1/0,0343 // vel sonido=343m/s = 0,0343 cm/microseg
}  
//=============================================================================================================  

void tempsLecturaIfr() {
  time = millis();
  difTime = time - previousTime;
  if (difTime >= intervalLectIfr) {
    delay(100);
    lecturaInicialCentre = analogRead(ifrCentre);
    lectInLeft = analogRead(ifrLeft);
    lectInRight = analogRead(ifrRight);
    lectInRear = analogRead(ifrRear); 
    previousTime = time;
  }  
}
//==========================================================================================================
void deteccioForats() {
  delay(500);
  lectFiCentre = analogRead(ifrCentre);
  lectFiLeft = analogRead(ifrLeft);
  lectFiRight = analogRead(ifrRight);
  lectFiRear = analogRead(ifrRear);
  difCentre = lectFiCentre - lecturaInicialCentre;
  difLeft = lectFiLeft - lectInLeft;
  difRight = lectFiRight - lectInRight;
  difRear = lectFiRear - lectInRear; 
  Serial.print("darrera: ");                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
  Serial.println(lectInRear);
  Serial.println(lectFiRear);
  Serial.println(difRear);
  Serial.print("davant: ");
  Serial.println(lectInLeft);
  Serial.println(lectFiLeft);
  Serial.println(difLeft);
  
  if (time < intervalLectIfr){      //pq ignori els 3 primers segons 
    difCentre=difLeft=difRight=difRear = 0;
    
    
  }  
  if (difCentre > difLectures) {
    stopRobot();
    digitalWrite(ledPinRear,HIGH);
    delay(500);
    reverse();
    delay(500);
    turnLeft();
    delay(500);
    digitalWrite(ledPinRear,LOW);
  }
  if (difLeft > difLectures) {
    stopRobot();
    digitalWrite(ledPinRear,HIGH);
    delay(500);
    reverse();
    delay(500);
    turnRight();
    delay(300);
    digitalWrite(ledPinRear,LOW);
  }
  if (difRight > difLectures) {
    stopRobot();
    digitalWrite(ledPinRear,HIGH);
    delay(500);
    reverse();
    delay(500);
    turnLeft();
    delay(300);
    digitalWrite(ledPinRear,LOW);
  }
  if (difRear > difLectRear) {
    stopRobot();
    digitalWrite(ledPinRear,HIGH);
    delay(500);
    forward();
    delay(1000);
    turnLeft();
    delay(300);
    digitalWrite(ledPinRear,HIGH);
  }  
}  
//===========================================================================================================  

void soInici(){
  /* declaración de variables */
  servoLeft.detach();
  servoRight.detach();
                                            
  int c[5]={131,262,523,1046,2093};       // frecuencias 4 octavas de Do
  int cs[5]={139,277,554,1108,2217};      // Do#
  int d[5]={147,294,587,1175,2349};       // Re
  int ds[5]={156,311,622,1244,2489};    // Re#
  int e[5]={165,330,659,1319,2637};      // Mi
  int f[5]={175,349,698,1397,2794};       // Fa
  int fs[5]={185,370,740,1480,2960};     // Fa#
  int g[5]={196,392,784,1568,3136};     // Sol
  int gs[5]={208,415,831,1661,3322};   // Sol#
  int a[5]={220,440,880,1760,3520};      // La
  int as[5]={233,466,932,1866,3729};    // La#
  int b[5]={247,494,988,1976,3951};      // Si

  
  /*******************/
  /*     STAR WARS      */
  /*******************/
  /**** tema principal ****/
  nota(d[1],150);noTone(spk);delay(50);
  nota(d[1],150);noTone(spk);delay(50);
  nota(d[1],150);noTone(spk);delay(50);/*
  nota(g[1],900);noTone(spk);delay(150);
  nota(d[2],900);noTone(spk);delay(50);
  nota(c[2],150);noTone(spk);delay(50);
  nota(b[1],150);noTone(spk);delay(50);
  nota(a[1],150);noTone(spk);delay(50);
  nota(g[2],900);noTone(spk);delay(150);
  nota(d[2],900);noTone(spk);delay(100);
  nota(c[2],150);noTone(spk);delay(50);
  nota(b[1],150);noTone(spk);delay(50);
  nota(a[1],150);noTone(spk);delay(50);
  nota(g[2],900);noTone(spk);delay(150);
  nota(d[2],900);noTone(spk);delay(100);
  nota(c[2],150);noTone(spk);delay(50);
  nota(b[1],150);noTone(spk);delay(50);
  nota(c[2],150);noTone(spk);delay(50);
  nota(a[1],1200);noTone(spk);delay(2000);*/
  
  
}
void nota(int frec, int t){
  tone(spk,frec);      // suena la nota frec recibida
  delay(t);                // para despues de un tiempo t
}

void tempsSo(){
  timeSo = millis();
  difTimeSo = timeSo - previousTimeSo;
  if (difTimeSo>=2000) {
    sound();
    previousTimeSo = timeSo;
  }
}

void sound(){
  tone(spk,300);
  delay(100);
  tone(spk,500);
  delay(100);
  tone(spk,150);
  delay(100);
  noTone(spk);
  delay(50);
}  

void LedsFront(){
  //if(endevant = 1){
  timeLedFront = millis();
  difTimeFront = timeLedFront - prevTimeFront;
  if (difTimeFront >= 300) {
    digitalWrite(13,HIGH);
    if (difTimeFront >= 1500) {
      digitalWrite(13,LOW);  
      prevTimeFront = timeLedFront;
    }
  }
                             
}

void llumsFars(){
  if (lightRight<umbralFars && lightLeft<umbralFars){
    digitalWrite(ledPinFars,HIGH);
  } else {
      digitalWrite(ledPinFars,LOW);  
    }
}

/*
void LedsFrontFast(){
  //if(endevant = 0){
  timeLedFrontF = millis();
  difTimeFrontF = timeLedFrontF - prevTimeFrontF;
  Serial.print("nnn= ");
  Serial.println(difTimeFrontF);
  Serial.println(prevTimeFrontF);
  if (difTimeFrontF >= 200) {
    digitalWrite(13,HIGH);
    if (difTimeFrontF >= 400) {
      digitalWrite(13,LOW);  
      prevTimeFrontF = timeLedFrontF;  
    }
  }
}*/

































































































