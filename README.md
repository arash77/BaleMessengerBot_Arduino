# BaleMessengerBot Arduino
Bale Messenger Bot Library for Arduino

Use the example (that is builded for esp8266) to understand more about working with the library

to download, click the DOWNLOADS button in the top right corner. Place the BaleMessengerBot_Arduino library folder your Arduino/libraries/ folder. Restart the IDE.

## Usage
for using this library you need to be connected to internet either with esp8266, esp32 or other modules.

### using in code



	#include <BaleMessengerBot.h>
	#include <WiFiClientSecure.h>
  
	WiFiClientSecure client;
	BaleMessengerBot bot(BOTtoken, client);
  
	bot.getUpdates(offset); #get new messages
	bot.sendMessage(chat_id,text,reply_to_message_id); #Send Message
	bot.last_message_received #update id of last message received
	bot.messages[i].chat_id #get user chat_id
	bot.messages[i].text #get user text
	bot.messages[i].from_name #get user name
  


## To Do
* editMessageText
* deleteMessage
* sendAudio
* sendDocument
* sendVideo
* sendVoice
* sendLocation
* sendContact
* getFile
* getChat
* sendInvoice
