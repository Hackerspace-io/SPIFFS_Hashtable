# SPIFFS_HashTable
makes a HashTable in SPIFFS area.

This library makes a DataBase in SPIFFS area of ESP8266 in HashTables structure.
Hash Table is a structure that helps to search and find your data so fast. for more info about HashTable refer to my repo about HashTable: [HashTable](https://github.com/Black3rror/HashTable_Tutorial)

informations that stores in data base should have a structure that defined in 'SPIFFS_HashTable.h' file.
u can easily change this structure for your purposes. just don't use pointers in this structure.

u can learn how to use functions in Test_Functions example. this examples data base members are cards which have a 4Byte UID and a Balance.
we will add them to data base, search for them, and remove them.
