/*
  HashTableSPIFFS.h - library for making hash table in SPIFFS area
  Created by amin ;)
*/
#ifndef SPIFFS_HashTable_H
#define SPIFFS_HashTable_H

#include <FS.h>		//It seems that it should be first
#include "Arduino.h"

struct rowStruct {		//just don't use String
	byte uid[4];
	int balance;
};

class SPIFFStable
{
  private:
    int hash(String uid);
	String uidExtracter(rowStruct row);
	
    int readAddrs(int addrs);
    void writeAddrs(int addrs, int value);
    void writeRowToAddrs(int addrs, rowStruct row);
	rowStruct returnCard(int pointer);
	
	void nextWriter(int addrs, int value);		//its not responsive to nextLength
	int nextReader(int addrs);		//its not responsive to nextLength
	
	bool checkRowEqual(int addrs, String uid);
	
	void changeNextPointer(int prevAddrs, int itemAddrs);
  
  public:
	SPIFFStable(int tableLength);
	
	bool rstFile();
	
	void insertCard(rowStruct row);
	bool removeCard(byte NUID[4]);
	rowStruct findCard(byte NUID[4]);
	
	int getLength();
	int getNumberOfCards(int memberNum);
	int getNumberOfCards(String content);
	int getNumberOfCards();
	int getFileSize();
	
	void printHistogram();
	void printHistogramDetailed();
	void printFileContent();
	
	//void printTable();
	//void printHistogram();
	
};

#endif