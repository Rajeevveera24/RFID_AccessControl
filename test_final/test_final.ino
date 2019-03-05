//Setup for the pins is given below

/*
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10        \\       10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */


//Include the necessary libraries...
#include <SPI.h>
#include <MFRC522.h>
#include<SD.h>


//Initialize pins for MFRC522
#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

File dataFile;


// Set Pins for led's, servo, buzzer and wipe button
/*constexpr uint8_t greenLed = 6;
constexpr uint8_t blueLed = 5;
constexpr uint8_t redLed = 7;
constexpr uint8_t ServoPin = 8;
constexpr uint8_t BuzzerPin = 4;
constexpr uint8_t wipeB = 3; */


//Declaring Variables
uint8_t successRead;  //Indicates whether a successful read operation happened from the RFID card reader 
boolean masterMode;   /*Denotes the current mode of the program - It can be in master mode(masterMode = true), when one of master cards has been swiped
                        In master mode, we can add/delete a project belonging to the same club as the master card.
                        In normal mode (masterMode = false), a card is scanned and is either granted access to correspong club if found, or is denied access
                      */ 
byte readCard[4];   //stores the UID of the card that has been scanned from the RFID reader...
int currentMaster;  //Represents the current master card...
int currentClub;    //Represents the club a scanned card belongs to...

//Declaring structure for storing the card IDs of masters and clubs

#define num_member 100 //This is the maximum number of members a student project/club can have
#define num_club 11    // This Represents the total number of clubs/student project

struct masterData{
  byte master_uid[4];   //UID of a master
  String club_name;   //Club name which master belongs to
  String club_file;   //This is the name of the .dat file which will be stored in the SD card and from which data will be retrieved
};

masterData md[num_club];

struct club_data{
  int card_count;
  byte card_id[num_member][4];
};

club_data cb;     //creating an object of type club_data
                  //This object stores the details of number of people, and the UIDs of these people from a particular student project
                  //In our actual implementation, this object will read from the SD card

//--------------------------------------------------------------Setup and Loop------------------------------------------------------------------------------------                  
//Entering setup()...

void setup() {
  Serial.begin(9600);     
  while (!Serial);    //Don't procede if serial connection is not set up
  SPI.begin();     
  SD.begin();
  mfrc522.PCD_Init();   
  mfrc522.PCD_DumpVersionToSerial();  //Show Card Details
 //Setting Master
  masterMode = false;
  //setCard();
  Serial.println("Normal Mode - Scan a tag");
}

//Entering Loop()...

void loop(){
  
  do{
    successRead = getID();  
  }while(!successRead);       //Keep reading from the RFID reader until a card is detected and it's UID is stored in readCard[] (by the function getID())
  
  if(masterMode){
    if(isMasterCard()){       //If we are presently in master mode and the master card is scanned again, we exit master mode and shift to normal mode...
      Serial.println("Exiting Program Mode");
      masterMode = false;
      delay(1000);
      Serial.println("Normal Mode - Scan a tag");   //Once in normal mode, we simply scan a tag(card) to see if it has access or not...
    }
    else{
      if(isPresentMaster()){
        deleteCard();       //If the card is present in database of the club, it is deleted by calling the function deleteCard()
      }                     // The function also sets masterMode to false, thus exiting out of master mode. Therefore, after deleting a card, we return to normal mode
      else{
        addCard();          //If the card is not present in database of the club, it is added by calling the function addCard()
      }                     // The function also sets masterMode to false, thus exiting out of masted mode
    }
  }
  else{                   // If this block is executed, then we are presently NOT in master mode
    if(isMasterCard()){   // If we are in normal mode and the master card is scanned, we enter master mode
      masterMode = true;
      Serial.println("Entering Master Mode for " +md[currentMaster].club_name);
      Serial.println("Scan a card to add/delete");
    }
    else{                 //We are not in master mode and a non master card has been scanned. Thus, it can either be granted access or not
      if(isPresentNormal()){  //If the card is present in the club's database, print access granted along with the club's name
        Serial.println("Access Granted for" + md[currentClub].club_name);
        delay(2000);        // After access is granted, we go back to normal mode
        Serial.println("Normal Mode - Scan a tag");
      }
      else{                 //If the card is NOT present in the club's database, print access denied along with the club's name
        Serial.println("Access Denied");
        delay(1000);        // After access is denied, we go back to normal mode
        Serial.println("Normal Mode - Scan a tag");
      }
    }
  }
}


//--------------------------------------------------------------------Functions------------------------------------------------------------------------------------

//This function sets the values of the master and club name

void initial_setup(){ //This function is called in set up 
  //RnC
  md[0].club_name = "RnC";
  md[0].club_file = "RNC.dat";
  md[0].master_uid[0] = 0;  //Mohit's UID needs to be stored here...
  md[0].master_uid[1] = 0;
  md[0].master_uid[2] = 0;
  md[0].master_uid[3] = 0;

  md[1].club_name = "Rugved";
  md[1].club_file = "RUGVED.dat";
  md[1].master_uid[0] = 0;  //Rugved head's UID needs to be stored here...
  md[1].master_uid[1] = 0;
  md[1].master_uid[2] = 0;
  md[1].master_uid[3] = 0;

  //ThrustMIT
  md[2].club_name = "ThrustMIT";
  md[2].club_file = "THRUSTMIT.dat";
  md[2].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[2].master_uid[1] = 0;
  md[2].master_uid[2] = 0;
  md[2].master_uid[3] = 0;

  //AeroMIT
  md[3].club_name = "AeroMIT";
  md[3].club_file = "AEROMIT.dat";
  md[3].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[3].master_uid[1] = 0;
  md[3].master_uid[2] = 0;
  md[3].master_uid[3] = 0;

  //SAE
  md[4].club_name = "SAE";
  md[4].club_file = "SAE.dat";
  md[4].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[4].master_uid[1] = 0;
  md[4].master_uid[2] = 0;
  md[4].master_uid[3] = 0;

  //MRM
  md[5].club_name = "MRM";
  md[5].club_file = "MRM.dat";
  md[5].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[5].master_uid[1] = 0;
  md[5].master_uid[2] = 0;
  md[5].master_uid[3] = 0;

  //Manas
  md[6].club_name = "Manas";
  md[6].club_file = "MANAS.dat";
  md[6].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[6].master_uid[1] = 0;
  md[6].master_uid[2] = 0;
  md[6].master_uid[3] = 0;

  //TMR
  md[7].club_name = "TMR";
  md[7].club_file = "TMR.dat";
  md[7].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[7].master_uid[1] = 0;
  md[7].master_uid[2] = 0;
  md[7].master_uid[3] = 0;  

  //FM
  md[8].club_name = "FM";
  md[8].club_file = "FM.dat";
  md[8].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[8].master_uid[1] = 0;
  md[8].master_uid[2] = 0;
  md[8].master_uid[3] = 0;
  
  //SM
  md[9].club_name = "Solar Mobile";
  md[9].club_file = "SM.dat";
  md[9].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[9].master_uid[1] = 0;
  md[9].master_uid[2] = 0;
  md[9].master_uid[3] = 0;

  //RM
  md[10].club_name = "Robo Manipal";
  md[10].club_file = "RM.dat";
  md[10].master_uid[0] = 0;  //head's UID needs to be stored here...
  md[10].master_uid[1] = 0;
  md[10].master_uid[2] = 0;
  md[10].master_uid[3] = 0;  
}


void write_to_card(){
  dataFile = SD.open(md[currentMaster].club_name,FILE_WRITE);
  dataFile.write((uint8_t *)&cb, sizeof(cb));
  dataFile.close();
}
/*void setCard(){
  cb.club_name = "RnC";
  cb.masterCard[0] = 0;
  cb.masterCard[1] = 0;
  cb.masterCard[2] = 0;
  cb.masterCard[3] = 0;
  cb.card_count = 0; //Represents the actual card count at present (ie, the number of cards that are stored in RnC)

  //Setting all UIDs to 0000H
  for(int i=0;i<num_member;i++){
    for(int j=0;j<4;j++){
      cb.card_id[i][j] = 0;
    }
  }
}*/


//This function is used to add a card to the club's database...
//This function can be used as it is...
void addCard(){
  
  for(int j=0;j<4;j++){
    cb.card_id[cb.card_count][j] = readCard[j];
  }
  
  cb.card_count++;
  write_to_card();
  
  Serial.println("Card Added");
  
  masterMode = false;         //Once the card is added, set masterMode to false, so that we can go to normal mode...
  
  delay(1000);
  Serial.println("Exiting Program Mode");
  
  delay(1000);
  Serial.println("Normal Mode - Scan a tag");
}


//This function deletes a card from the club's database...
//This function can be used as it is....
void deleteCard(){
  
  int flag = 0,flag1;
  int i=0;
  
  while(!flag){
    for(int j=0;j<4;j++){
      flag1 = 1;
      if(readCard[j] != cb.card_id[i][j]){
        flag1 = 0;
        i++;
        break;
      }
    }
    flag = flag1;
  }

  for(;i<cb.card_count-1;i++){
    for(int j=0;j<4;j++){
      cb.card_id[i][j] = cb.card_id[i+1][j];
    }
  }
  
  for(int j = 0 ;j<4;j++){
    cb.card_id[cb.card_count-1][j] = 0;
  }
  
  cb.card_count--;
  write_to_card();
  
  Serial.println("Card Deleted");
  
  masterMode = false;         //Once a card is deleted, set masterMode to false to return to normal mode...
  
  delay(1000);
  Serial.println("Exiting Program Mode");
  
  delay(1000);
  Serial.println("Normal Mode - Scan a tag");
}


//This function checks if the card that has been scanned is a master card or not
boolean isMasterCard(){
  int flag;
  for(int k=0;k<num_club;k++){
    flag = 0;
    for(int j=0;j<4;j++){
      if(readCard[j] != md[k].master_uid[j]){
        break;
      }
      if(j==3){
        flag = 1;
      }
    }
    if(flag == 1){
      currentMaster = k;
      return true;
    }
  }
  return false;
}


//This function checks if a card is present in the database of the club under consideration.
//The function returns true if the card is found, else it returns false

boolean isPresentMaster(){    //Called in master mode to check if a card belongs to the club of the current master;

  //Insert Code here to initiate communciation with SD card..
  dataFile = SD.open(md[currentMaster].club_file,FILE_READ);
  dataFile.read((uint8_t *)&cb,sizeof(cb));
  dataFile.close();
  
  int flag;
  for(int i=0;i<cb.card_count;i++){
    flag = 0;
    for(int j=0;j<4;j++){
      if(readCard[j] != cb.card_id[i][j]){
        break;
      }
      if(j == 3){
        flag = 1;
      }
    }
    if(flag == 1){
      return true;
    }
  }
  return false;
}

boolean isPresentNormal(){    //Used to check if a card is present in the database of any of the clubs (ie, this function is used in normal mode for simple access granting/denying)

  int flag;
  
  for(int k=0;k<num_club;k++){
    
    dataFile = SD.open(md[k].club_file,FILE_READ);
    dataFile.read((uint8_t *)&cb,sizeof(cb));
    dataFile.close();

    for(int i=0;i<cb.card_count;i++){

      flag = 0;

      for(int j=0;j<4;j++){
        if(readCard[j] != cb.card_id[i][j]){
          break;
        }
        if(j == 3){
          flag = 1;
        }
      }
      
      if(flag == 1){
        currentClub = k;
        return true;
      }
    }
  }
  return false;
}
//This function is used to scane the UID of the card placed in front of the RFID
//This function is not to be altered with...
uint8_t getID() {
  
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If no tag is placed, return 0 to successRead
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a Tag is placed get Serial and continue
    return 0;
  }
  for ( uint8_t i = 0; i < 4; i++) {  //Store the UID of the card from RFID reader and store it in readCard 
    readCard[i] = mfrc522.uid.uidByte[i];
  }
  mfrc522.PICC_HaltA(); // Stop reading from RFID reader
  return 1;  
}

