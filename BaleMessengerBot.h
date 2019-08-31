/*
   Copyright (c) 2019 Arash Kadkhodaei. All right reserved.

   BaleMessengerBot - Library to create your own Bale Bot using
   ESP8266 or ESP32 on Arduino IDE.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef BaleMessengerBot_h
#define BaleMessengerBot_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>

#define HOST "tapi.bale.ai"
#define SSL_PORT 443
#define HANDLE_MESSAGES 1

typedef bool (*MoreDataAvailable)();
typedef byte (*GetNextByte)();

struct baleMessage {
  String text;
  String chat_id;
  String chat_title;
  String from_id;
  String from_name;
  String date;
  String type;
  float longitude;
  float latitude;
  int update_id;
};

class BaleMessengerBot {
public:
  BaleMessengerBot(String token, Client &client);
  String sendGetToBale(String command);
  String sendPostToBale(String command, JsonObject &payload);
  String
  sendMultipartFormDataToBale(String command, String binaryProperyName,
                                  String fileName, String contentType,
                                  String chat_id, int fileSize,
                                  MoreDataAvailable moreDataAvailableCallback,
                                  GetNextByte getNextByteCallback);

  bool getMe();

  bool sendMessage(String chat_id, String text, int reply_to_message_id = 0);
  bool sendMessageWithInlineKeyboard(String chat_id, String text,
                                     String parse_mode, String keyboard);


  bool sendPostMessage(JsonObject &payload);
  String sendPostPhoto(JsonObject &payload);
  String sendPhotoByBinary(String chat_id, String contentType, int fileSize,
                           MoreDataAvailable moreDataAvailableCallback,
                           GetNextByte getNextByteCallback);
  String sendPhoto(String chat_id, String photo, String caption = "",
                   int reply_to_message_id = 0);

  int getUpdates(long offset);
  bool checkForOkResponse(String response);
  baleMessage messages[HANDLE_MESSAGES];
  long last_message_received;
  String name;
  String userName;
  int longPoll = 0;
  bool _debug = false;
  int waitForResponse = 1500;
  int init();

private:
  // JsonObject * parseUpdates(String response);
  String _token;
  Client *client;
  bool processResult(JsonObject &result, int messageIndex);
  void closeClient();
  const int maxMessageLength = 100000;
  const int maxlimitinit = 10;
};

#endif
