/*
  HashTableSPIFFS.cpp - library for making hash table in SPIFFS area
  Created by amin ;)
*/

#include "Arduino.h"
#include "HashTableSPIFFS.h"

#define dir "/members.db"
#define itemEnd	0xFFFF		// nextLength bytes 0xFF
#define uidLength 4		// size of uid. our NUIDs have 4 bytes
#define nextLength 2	// size of next address
#define counterLength 1		// size of arrays length counter.
// order: Content + Next (+ MemberCounter)

File file;
rowStruct rowStructure;
int length = 13;
int structLength = 5;		// size of struct

int SPIFFStable::hash(String uid) {
	int value = 0;
	for (int i = 0; i < uidLength; i++)
		value += uid[i];
	return (structLength+nextLength+counterLength)*((uidLength * value) % length);
}

String SPIFFStable::uidExtracter(rowStruct row) {
	String uid = "";
	char * rowPointer = reinterpret_cast<char*> (&row);
	for (int i=0 ; i<uidLength ; i++) {
		uid += (char) *(rowPointer+i);
	}
	return uid;
}

int SPIFFStable::readAddrs(int addrs) {
	file.seek(addrs, SeekSet);
	return file.read();
}

void SPIFFStable::writeAddrs(int addrs, int value) {
	file.seek(addrs, SeekSet);
	file.write(value);
}

void SPIFFStable::writeRowToAddrs(int addrs, rowStruct row) {
	file.seek(addrs, SeekSet);
	char * rowPointer = reinterpret_cast<char*> (&row);
	for (int i=0 ; i<structLength ; i++) {
		file.write(*(rowPointer+i));
	}
	nextWriter(addrs, itemEnd);
}

rowStruct SPIFFStable::returnCard(int pointer) {
	rowStruct row;
	char cardBuf = (int)0;		// we will write the row sequently in address of char cardBuf and after that 
	if (pointer != -1) {
		for (int i=0 ; i<structLength ; i++) {
			*(&cardBuf+i) = readAddrs(pointer+i);
		}
	}
	else {
		for (int i=0 ; i<structLength ; i++) {
			*(&cardBuf+i) = 0xFF;
		}
	}
	row = *(reinterpret_cast<rowStruct*>(&cardBuf));
	file.close();
	return row;
}

void SPIFFStable::nextWriter(int addrs, int value) {
	int big = value/256;
	int small = value%256;
	file.seek(addrs+structLength, SeekSet);
	file.write(big);
	file.write(small);
}

int SPIFFStable::nextReader(int addrs) {
	file.seek(addrs+structLength, SeekSet);
	int big = file.read();
	int small = file.read();
	return 256*big + small;
}

bool SPIFFStable::checkRowEqual(int addrs, String uid) {
	for(int i=0 ; i<uidLength ; i++) {
		if (readAddrs(addrs+i) != uid[i]) {
			return false;
		}
	}
	return true;
}

void SPIFFStable::changeNextPointer(int prevAddrs, int itemAddrs) {
	int nextAddrs = nextReader(itemAddrs);
	nextWriter(prevAddrs, nextAddrs);
}

SPIFFStable::SPIFFStable(int tableLength) {
	// get the length of array.
	if (tableLength <= 0) tableLength = 13;
	length = tableLength;
	
	structLength = sizeof(rowStructure);
	
	// init spiffs
	if (SPIFFS.begin()) {
		Serial.println("mounted file system");
	} 
	else {
		Serial.println("failed to mount FS");
	}
}

bool SPIFFStable::rstFile() {		// when calling rstFile, all the file values going to be 0xFF
	bool result = SPIFFS.remove(dir);
	file = SPIFFS.open(dir, "w+");		// creating new members.db for further usage
	String initItems = "";
	for (int i=0 ; i<length ; i++) {
		for (int j=0 ; j<structLength+nextLength ; j++) {
			initItems += (char)itemEnd;
		}
		for (int j=0 ; j<counterLength ; j++) {
			initItems += (char)0;
		}
	}
	file.print(initItems);
	file.close();
	if (result == true) {
		return true;
	}
	else {
		return false;
	}
}

void SPIFFStable::insertCard(rowStruct row) {
	file = SPIFFS.open(dir, "r+");
	if (file) {
		String uid = uidExtracter(row);
		int pointer = hash(uid);
		int next = nextReader(pointer);		// next is a pointer to the next item
		
		if (next != itemEnd || readAddrs(pointer+structLength+nextLength) != 0) {		// if the array member isn't empy: 
			writeAddrs(pointer+structLength+nextLength, readAddrs(pointer+structLength+nextLength)+1);		// increment the members counter
			
			while (next != itemEnd) {		// next address = itemEnd means its the end of list
				pointer = next;
				next = nextReader(pointer);
			}
			// now pointer points to the last item in Linked list
			nextWriter(pointer, file.size());
			writeRowToAddrs(file.size(), row);
		}
		else {		// its the first item of the array member.
			writeRowToAddrs(pointer, row);	// write content in the first place of array members Linked list
			writeAddrs(pointer+structLength+nextLength, 1);	// write 1 into the listLength
		}
	}
	else {
		Serial.println("file open failed");
	}
	file.close();
}

bool SPIFFStable::removeCard(byte NUID[uidLength]) {
	file = SPIFFS.open(dir, "r+");
	if (file) {
		String uid = "";
		for (int i=0 ; i<uidLength ; i++) {
			uid += (char)NUID[i];
		}
		int pointer = hash(uid);
		int hashAddrs = pointer;	//for decreasing the memberCounter
		int next = nextReader(pointer);		// next is a pointer to the next item
		bool cardFounded = checkRowEqual(pointer, uid);
		if (cardFounded == true) {	// we should just delete the content and decrease the memberCounter
			for(int i=0 ; i<structLength ; i++) {
				writeAddrs(pointer+i, itemEnd);
			}
			writeAddrs(hashAddrs+structLength+nextLength, readAddrs(hashAddrs+structLength+nextLength)-1);
			file.close();
			return true;
		}
		while(next != itemEnd) {
			cardFounded = checkRowEqual(next, uid);
			if (cardFounded == true) {
				changeNextPointer(pointer, next);	// next refers to the Item that should be deleted, pointer refers to the Item before nexts Item
				writeAddrs(hashAddrs+structLength+nextLength, readAddrs(hashAddrs+structLength+nextLength)-1);
				file.close();
				return true;
			}
			pointer = next;
			next = nextReader(pointer);
		}
		file.close();
		return false;
	}
	else {
		Serial.println("file open failed");
		file.close();
		return false;
	}
	file.close();
}

rowStruct SPIFFStable::findCard(byte NUID[uidLength]) {
	file = SPIFFS.open(dir, "r");
	if (file) {
		String uid = "";
		for (int i=0 ; i<uidLength ; i++) {
			uid += (char)NUID[i];
		}
		int pointer = hash(uid);
		int next = nextReader(pointer);		// next is a pointer to the next item
		bool cardFounded = checkRowEqual(pointer, uid);
		if (cardFounded == true) {
			return returnCard(pointer);
			file.close();
		}
		while(next != itemEnd) {
			pointer = next;
			cardFounded = checkRowEqual(pointer, uid);
			if (cardFounded == true) {
				return returnCard(pointer);
			}
			next = nextReader(pointer);
		}
		return returnCard(-1);
	}
	else {
		Serial.println("file open failed");
		return returnCard(-1);
	}
	file.close();
}

int SPIFFStable::getLength() {
	return length;
}

int SPIFFStable::getNumberOfCards(int memberNum) {
	if (memberNum < 0 || memberNum > length-1) {
		return -1;
	}
	file = SPIFFS.open(dir, "r");
	if (file) {
		int pointer = memberNum*(structLength+nextLength+counterLength);
		return readAddrs(pointer+structLength+nextLength);
		file.close();
	}
	else {
		Serial.println("file open failed");
		return -1;
	}
	file.close();
}

int SPIFFStable::getNumberOfCards(String uid) {
	file = SPIFFS.open(dir, "r");
	if (file) {
		int pointer = hash(uid);
		return readAddrs(pointer+structLength+nextLength);
		file.close();
	}
	else {
		Serial.println("file open failed");
		return -1;
	}
	file.close();
}

int SPIFFStable::getNumberOfCards() {
	int numbers = 0;
	for (int i=0 ; i<length ; i++) {
		numbers += getNumberOfCards(i);
	}
	return numbers;
}


int SPIFFStable::getFileSize() {
	file = SPIFFS.open(dir, "r");
	if (file) {
		int fileSize = file.size();
		file.close();
		return fileSize;
	}
	else {
		Serial.println("file open failed");
		file.close();
		return -1;
	}
}

void SPIFFStable::printHistogram() {
	Serial.print("\n\nHash Table Contains ");
	Serial.print(getNumberOfCards());
	Serial.print(" Items in total\n");
	for (int i=0 ; i<length ; i++) {
		Serial.print(i);
		Serial.print(":\t");
		for (int j=0 ; j<getNumberOfCards(i) ; j++) {
			Serial.print("x");
		}
		Serial.print("\n");
	}
	Serial.print("\n");
}

void SPIFFStable::printHistogramDetailed() {
	Serial.print("\n\nHash Table Contains ");
	Serial.print(getNumberOfCards());
	Serial.print(" Items in total\n");
	for (int i=0 ; i<length ; i++) {
		int pointer = i*(structLength+nextLength+counterLength);
		Serial.print(i);
		Serial.print(":\t");
		for (int j=0 ; j<getNumberOfCards(i) ; j++) {
			for (int k=0 ; k<uidLength ; k++) {
				if ((int)readAddrs(pointer+k) < 16)
					Serial.print("0");
				Serial.print((int)readAddrs(pointer+k), HEX);
				//Serial.print(" ");
			}
			Serial.print(" ");
			pointer = nextReader(pointer);
		}
		Serial.print("\n");
	}
	Serial.print("\n");
}

void SPIFFStable::printFileContent() {
	file = SPIFFS.open(dir, "r");
	if (file) {
		Serial.println("====== Reading from SPIFFS file =======");
		int readedByte;
		int pointer = 0;
		while (file.available()){
			readedByte = file.read();
			Serial.print(pointer);
			Serial.print(":\t");
			Serial.println(readedByte);
			++pointer;
		}
		Serial.print("\n");
	}
	else {
		Serial.println("file open failed");
	}
	file.close();
}