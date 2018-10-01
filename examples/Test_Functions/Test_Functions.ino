#include <SPIFFS_HashTable.h>

SPIFFStable members(71);

rowStruct card;
  
void setup() {

  Serial.begin(115200);

  Serial.println("rstFile ...");
  members.rstFile();

  Serial.print("Starting process of inserting: ");
  unsigned long ms = millis();
  for (int i=0 ; i<100 ; i++) {   // makes 100 random card and adds them to data base.
    card = randomCard();
    members.insertCard(card);   // adds card to our data base.
    delay(10);
  }
  Serial.print(" time: ");
  Serial.println (millis() - ms);
  
  Serial.print("NUID exists? ");    // NUID is '17,18,19,20'
  byte cardNUID[4] = {17, 18, 19, 20};
  ms = millis();
  Serial.print(members.findCard(cardNUID).balance);   // if card doesn't exist, its balance will be -1
  Serial.print (" time: ");
  Serial.println (millis() - ms);
  
  Serial.println("adding NUID ...");
  card.uid[0] = 17;
  card.uid[1] = 18;
  card.uid[2] = 19;
  card.uid[3] = 20;
  card.balance = random(0, 0xFF);
  members.insertCard(card);
  
  Serial.print("NUID exists? ");
  ms = millis();
  Serial.print(members.findCard(cardNUID).balance);
  Serial.print (" time: ");
  Serial.println (millis() - ms);
  
  Serial.println("removing NUID ...");
  members.removeCard(cardNUID);
  
  Serial.print("NUID exists? ");
  ms = millis();
  Serial.print(members.findCard(cardNUID).balance);
  Serial.print (" time: ");
  Serial.println (millis() - ms);

  Serial.print("file size is: ");
  Serial.println(members.getFileSize());
  Serial.print("including ");
  Serial.print(members.getNumberOfCards());
  Serial.println(" Items");
  Serial.print("Table has ");
  Serial.print(members.getLength());
  Serial.println(" Rows");
  Serial.print("where Row 7 has ");
  Serial.print(members.getNumberOfCards(7));
  Serial.println(" Items");
  members.printHistogram();
  members.printHistogramDetailed();
  members.printFileContent();

}

void loop() {

  while (Serial.available()) {
    char input = Serial.read();
    if (input == 'r') {
      members.rstFile();
      Serial.println("rstFile Done");
    }
  }
  
}

rowStruct randomCard() {  // returns a random card
  rowStruct card;
  for (int i=0 ; i<4 ; i++) {
    card.uid[i] = random(0, 0xFF);
  }
  card.balance = random(0, 0xFF);
  return card;
}
