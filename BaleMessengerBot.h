/*
Copyright (c) 2018 Arash Kadkhodaei. All right reserved.

BaleMessengerBot - Library to create your own bale Bot using
ESP8266 or ESP32 on Arduino IDE.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
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

struct telegramMessage {
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
  String sendGetToTelegram(String command);
  String sendPostToTelegram(String command, JsonObject &payload);
  String
  sendMultipartFormDataToTelegram(String command, String binaryProperyName,
                                  String fileName, String contentType,
                                  String chat_id, int fileSize,
                                  MoreDataAvailable moreDataAvailableCallback,
                                  GetNextByte getNextByteCallback);

  bool getMe();

  bool sendSimpleMessage(String chat_id, String text, String parse_mode);
  bool sendMessage(String chat_id, String text, String parse_mode = "");
  bool sendMessageWithReplyKeyboard(String chat_id, String text,
                                    String parse_mode, String keyboard,
                                    bool resize = false, bool oneTime = false,
                                    bool selective = false);
  bool sendMessageWithInlineKeyboard(String chat_id, String text,
                                     String parse_mode, String keyboard);

  bool sendChatAction(String chat_id, String text);

  bool sendPostMessage(JsonObject &payload);
  String sendPostPhoto(JsonObject &payload);
  String sendPhotoByBinary(String chat_id, String contentType, int fileSize,
                           MoreDataAvailable moreDataAvailableCallback,
                           GetNextByte getNextByteCallback);
  String sendPhoto(String chat_id, String photo, String caption = "",
                   bool disable_notification = false,
                   int reply_to_message_id = 0, String keyboard = "");

  int getUpdates(long offset);
  bool checkForOkResponse(String response);
  telegramMessage messages[HANDLE_MESSAGES];
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
  const int maxMessageLength = 1300;
};

#endif
