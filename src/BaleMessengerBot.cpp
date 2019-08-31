/*
   Copyright (c) 2019 Arash Kadkhodaei. All right reserved.

   BaleMessengerBot - Library to create your own Bale Bot using
   ESP8266 or ESP32 on Arduino IDE.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
   **** Note Regarding Client Connection Keeping ****
   Client connection is established in functions that directly involve use of
   client, i.e sendGetToBale, sendPostToBale, and
   sendMultipartFormDataToBale. It is closed at the end of
   sendMultipartFormDataToBale, but not at the end of sendGetToBale and
   sendPostToBale as these may need to keep the connection alive for respose
   / response checking. Re-establishing a connection then wastes time which is
   noticeable in user experience. Due to this, it is important that connection
   be closed manually after calling sendGetToBale or sendPostToBale by
   calling closeClient(); Failure to close connection causes memory leakage and
   SSL errors
 */

#include "BaleMessengerBot.h"

BaleMessengerBot::BaleMessengerBot(String token, Client &client) {
  _token = token;
  this->client = &client;
}

int BaleMessengerBot::init() {
  while(1){
    String command = "bot" + _token + "/getUpdates";
    DynamicJsonBuffer jsonBuffer;
    JsonObject &payload = jsonBuffer.createObject();
    payload["offset"] = last_message_received+1;
    payload["limit"] = maxlimitinit;
    String response = sendPostToBale(command,payload);
    if (response == "") {
      if (_debug)
        Serial.println(F("Received empty string in response!"));
      closeClient();
      return 0;
    }else{
      DynamicJsonBuffer jsonBuffer;
      JsonObject &root = jsonBuffer.parseObject(response);
      if (root.success()) {
        if (root.containsKey("result")) {
          int resultArrayLength = root["result"].size();
          if (resultArrayLength != 0) {
            JsonObject &result = root["result"][resultArrayLength-1];
            last_message_received = result["update_id"];
          }else{
            closeClient();
            return last_message_received;
          }
        }
      }
    }
  }
}

String BaleMessengerBot::sendGetToBale(String command) {
  String mess = "";
  long now;
  bool avail;

  // Connect if not already connected
  if (!client->connected()) {
    if (_debug)
      Serial.println(F("[BOT]Connecting to server"));
    if (!client->connect(HOST, SSL_PORT)) {
      if (_debug)
        Serial.println(F("[BOT]Conection error"));
    }
  }
  if (client->connected()) {

    if (_debug)
      Serial.println(F(".... connected to server"));

    String a = "";
    char c;
    int ch_count = 0;
    client->println("GET /" + command);
    
    now = millis();
    avail = false;
    while (millis() - now < longPoll * 1000 + waitForResponse) {
      while (client->available()) {
        char c = client->read();
        //if (_debug) Serial.write(c);
        if (ch_count < maxMessageLength) {
          mess = mess + c;
          ch_count++;
        }
        avail = true;
      }
      if (avail) {
        if (_debug) {
          Serial.println();
          Serial.println(mess);
          Serial.println();
        }
        break;
      }
    }
  }

  return mess;
}

String BaleMessengerBot::sendPostToBale(String command, JsonObject &payload) {

  String body = "";
  String headers = "";
  long now;
  bool responseReceived;

  // Connect if not already connected
  if (!client->connected()) {
    if (_debug)
      Serial.println(F("[BOT Client]Connecting to server"));
    if (!client->connect(HOST, SSL_PORT)) {
      if (_debug)
        Serial.println(F("[BOT Client]Conection error"));
    }
  }
  if (client->connected()) {
    // POST URI
    client->print("POST /" + command);
    client->println(F(" HTTP/1.1"));
    // Host header
    client->print(F("Host:"));
    client->println(HOST);
    // JSON content type
    client->println(F("Content-Type: application/json"));

    // Content length
    int length = payload.measureLength();
    client->print(F("Content-Length:"));
    client->println(length);
    // End of headers
    client->println();
    // POST message body
    // json.printTo(client); // very slow ??
    String out;
    payload.printTo(out);
    client->println(out);

    int ch_count = 0;
    char c;
    now = millis();
    responseReceived = false;
    bool finishedHeaders = false;
    bool currentLineIsBlank = true;
    while (millis() - now < waitForResponse) {
      while (client->available()) {
        char c = client->read();
        responseReceived = true;

        if (!finishedHeaders) {
          if (currentLineIsBlank && c == '\n') {
            finishedHeaders = true;
          } else {
            headers = headers + c;
          }
        } else {
          if (ch_count < maxMessageLength) {
            body = body + c;
            ch_count++;
          }
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }

      if (responseReceived) {
        if (_debug) {
          Serial.println();
          Serial.println(body);
          Serial.println();
        }
        break;
      }
    }
  }

  return body;
}

String BaleMessengerBot::sendMultipartFormDataToBale(
    String command, String binaryProperyName, String fileName,
    String contentType, String chat_id, int fileSize,
    MoreDataAvailable moreDataAvailableCallback,
    GetNextByte getNextByteCallback) {

  String body = "";
  String headers = "";
  long now;
  bool responseReceived;
  String boundry = F("------------------------b8f610217e83e29b");

  // Connect if not already connected
  if (!client->connected()) {
    if (_debug)
      Serial.println(F("[BOT Client]Connecting to server"));
    if (!client->connect(HOST, SSL_PORT)) {
      if (_debug)
        Serial.println(F("[BOT Client]Conection error"));
    }
  }
  if (client->connected()) {

    String start_request = "";
    String end_request = "";

    start_request = start_request + "--" + boundry + "\r\n";
    start_request = start_request +
                    "content-disposition: form-data; name=\"chat_id\"" + "\r\n";
    start_request = start_request + "\r\n";
    start_request = start_request + chat_id + "\r\n";

    start_request = start_request + "--" + boundry + "\r\n";
    start_request = start_request + "content-disposition: form-data; name=\"" +
                    binaryProperyName + "\"; filename=\"" + fileName + "\"" +
                    "\r\n";
    start_request = start_request + "Content-Type: " + contentType + "\r\n";
    start_request = start_request + "\r\n";

    end_request = end_request + "\r\n";
    end_request = end_request + "--" + boundry + "--" + "\r\n";

    client->print("POST /bot" + _token + "/" + command);
    client->println(F(" HTTP/1.1"));
    // Host header
    client->print(F("Host: "));
    client->println(HOST);
    client->println(F("User-Agent: arduino/1.0"));
    client->println(F("Accept: */*"));

    int contentLength =
        fileSize + start_request.length() + end_request.length();
    if (_debug)
      Serial.println("Content-Length: " + String(contentLength));
    client->print("Content-Length: ");
    client->println(String(contentLength));
    client->println("Content-Type: multipart/form-data; boundary=" + boundry);
    client->println("");

    client->print(start_request);

    if (_debug)
      Serial.print(start_request);

    byte buffer[512];
    int count = 0;
    char ch;
    while (moreDataAvailableCallback()) {
      buffer[count] = getNextByteCallback();
      // client->write(ch);
      // Serial.write(ch);
      count++;
      if (count == 512) {
        // yield();
        if (_debug) {
          Serial.println(F("Sending full buffer"));
        }
        client->write((const uint8_t *)buffer, 512);
        count = 0;
      }
    }

    if (count > 0) {
      if (_debug) {
        Serial.println(F("Sending remaining buffer"));
      }
      client->write((const uint8_t *)buffer, count);
    }

    client->print(end_request);
    if (_debug)
      Serial.print(end_request);

    count = 0;
    int ch_count = 0;
    char c;
    now = millis();
    bool finishedHeaders = false;
    bool currentLineIsBlank = true;
    while (millis() - now < waitForResponse) {
      while (client->available()) {
        char c = client->read();
        responseReceived = true;

        if (!finishedHeaders) {
          if (currentLineIsBlank && c == '\n') {
            finishedHeaders = true;
          } else {
            headers = headers + c;
          }
        } else {
          if (ch_count < maxMessageLength) {
            body = body + c;
            ch_count++;
          }
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }

      if (responseReceived) {
        if (_debug) {
          Serial.println();
          Serial.println(body);
          Serial.println();
        }
        break;
      }
    }
  }

  closeClient();
  return body;
}

bool BaleMessengerBot::getMe() {
  String command = "bot" + _token + "/getMe";
  String response =
      sendGetToBale(command); // receive reply
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(response);

  closeClient();

  if (root.success()) {
    if (root.containsKey("result")) {
      String _name = root["result"]["first_name"];
      String _username = root["result"]["username"];
      name = _name;
      userName = _username;
      return true;
    }
  }

  return false;
}

/***************************************************************
 * GetUpdates - function to receive messages from Bale         *
 * (Argument to pass: the last+1 message to read)              *
 * Returns the number of new messages                          *
 ***************************************************************/
int BaleMessengerBot::getUpdates(long offset) {

  if (_debug)
    Serial.println(F("GET Update Messages"));

  String command = "bot" + _token + "/getUpdates";
  DynamicJsonBuffer jsonBuffer;
  JsonObject &payload = jsonBuffer.createObject();
  payload["offset"] = String(offset);
  payload["limit"] = String(HANDLE_MESSAGES);

  String response = sendPostToBale(command,payload);

  if (response == "") {
    if (_debug)
      Serial.println(F("Received empty string in response!"));
    // close the client as there's nothing to do with an empty string
    closeClient();
    return 0;
  } else {
    if (_debug) {
      Serial.print(F("incoming message length"));
      Serial.println(response.length());
      //Serial.println(response);
      Serial.println(F("Creating DynamicJsonBuffer"));
    }

    // Parse response into Json object
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(response);

    if (root.success()) {
      // root.printTo(Serial);
      if (_debug)
        Serial.println();
      if (root.containsKey("result")) {
        int resultArrayLength = root["result"].size();
        //Serial.println(resultArrayLength);
        if (resultArrayLength > 0) {
          int newMessageIndex = 0;
          // Step through all results
          for (int i = 0; i < resultArrayLength; i++) {
            JsonObject &result = root["result"][i];
            if (processResult(result, newMessageIndex)) {
              newMessageIndex++;
            }
          }
          // We will keep the client open because there may be a response to be
          // given
          return newMessageIndex;
        } else {
          if (_debug)
            Serial.println(F("no new messages"));
        }
      } else {
        if (_debug)
          Serial.println(F("Response contained no 'result'"));
      }
    } else { // Parsing failed
        if (_debug) Serial.println(F("Failed to parse update, the message could be too big for the buffer"));
        last_message_received++;
    }
    // Close the client as no response is to be given
    closeClient();
    return 0;
  }
}

bool BaleMessengerBot::processResult(JsonObject &result, int messageIndex) {
  int update_id = result["update_id"];
  // Check have we already dealt with this message (this shouldn't happen!)
  if (last_message_received != update_id) {
    last_message_received = update_id;
    messages[messageIndex].update_id = update_id;

    messages[messageIndex].text = F("");
    messages[messageIndex].from_id = F("");
    messages[messageIndex].from_name = F("");
    messages[messageIndex].longitude = 0;
    messages[messageIndex].latitude = 0;

    if (result.containsKey("message")) {
      JsonObject &message = result["message"];
      messages[messageIndex].type = F("message");
      messages[messageIndex].from_id = message["from"]["id"].as<String>();
      messages[messageIndex].from_name =
          message["from"]["first_name"].as<String>();

      messages[messageIndex].date = message["date"].as<String>();
      messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
      messages[messageIndex].chat_title = message["chat"]["title"].as<String>();

      if (message.containsKey("text")) {
        messages[messageIndex].text = message["text"].as<String>();

      } else if (message.containsKey("location")) {
        messages[messageIndex].longitude =
            message["location"]["longitude"].as<float>();
        messages[messageIndex].latitude =
            message["location"]["latitude"].as<float>();
      }
    } else if (result.containsKey("channel_post")) {
      JsonObject &message = result["channel_post"];
      messages[messageIndex].type = F("channel_post");

      messages[messageIndex].text = message["text"].as<String>();
      messages[messageIndex].date = message["date"].as<String>();
      messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
      messages[messageIndex].chat_title = message["chat"]["title"].as<String>();

    } else if (result.containsKey("callback_query")) {
      JsonObject &message = result["callback_query"];
      messages[messageIndex].type = F("callback_query");
      messages[messageIndex].from_id = message["from"]["id"].as<String>();
      messages[messageIndex].from_name =
          message["from"]["first_name"].as<String>();

      messages[messageIndex].text = message["data"].as<String>();
      messages[messageIndex].date = message["date"].as<String>();
      messages[messageIndex].chat_id =
          message["message"]["chat"]["id"].as<String>();
      messages[messageIndex].chat_title = F("");
    } else if (result.containsKey("edited_message")) {
      JsonObject &message = result["edited_message"];
      messages[messageIndex].type = F("edited_message");
      messages[messageIndex].from_id = message["from"]["id"].as<String>();
      messages[messageIndex].from_name =
          message["from"]["first_name"].as<String>();

      messages[messageIndex].date = message["date"].as<String>();
      messages[messageIndex].chat_id = message["chat"]["id"].as<String>();
      messages[messageIndex].chat_title = message["chat"]["title"].as<String>();

      if (message.containsKey("text")) {
        messages[messageIndex].text = message["text"].as<String>();

      } else if (message.containsKey("location")) {
        messages[messageIndex].longitude =
            message["location"]["longitude"].as<float>();
        messages[messageIndex].latitude =
            message["location"]["latitude"].as<float>();
      }
    }

    return true;
  }
  return false;
}

/***********************************************************************
 * SendMessage - function to send message to Bale                      *
 * (Arguments to pass: chat_id, text to transmit and markup(optional)) *
 ***********************************************************************/
bool BaleMessengerBot::sendMessage(String chat_id, String text, int reply_to_message_id) {

  DynamicJsonBuffer jsonBuffer;
  JsonObject &payload = jsonBuffer.createObject();

  payload["chat_id"] = chat_id;
  payload["text"] = text;

  if (reply_to_message_id != 0) {
    payload["reply_to_message_id"] = reply_to_message_id;
  }

  return sendPostMessage(payload);
}

/*
bool BaleMessengerBot::sendMessageWithInlineKeyboard(String chat_id,
                                                         String text,
                                                         String parse_mode,
                                                         String keyboard) {

  DynamicJsonBuffer jsonBuffer;
  JsonObject &payload = jsonBuffer.createObject();

  payload["chat_id"] = chat_id;
  payload["text"] = text;

  if (parse_mode != "") {
    payload["parse_mode"] = parse_mode;
  }

  JsonObject &replyMarkup = payload.createNestedObject("reply_markup");

  DynamicJsonBuffer keyboardBuffer;
  replyMarkup["inline_keyboard"] = keyboardBuffer.parseArray(keyboard);

  return sendPostMessage(payload);
}
*/

/***********************************************************************
 * SendPostMessage - function to send message to Bale                  *
 * (Arguments to pass: chat_id, text to transmit and markup(optional)) *
 ***********************************************************************/
bool BaleMessengerBot::sendPostMessage(JsonObject &payload) {

  bool sent = false;
  if (_debug)
    Serial.println(F("SEND Post Message"));
  long sttime = millis();

  if (payload.containsKey("text")) {
    while (millis() < sttime + 8000) { // loop for a while to send the message
      String command = "bot" + _token + "/sendMessage";
      String response = sendPostToBale(command, payload);
      if (_debug)
        Serial.println(response);
      sent = checkForOkResponse(response);
      if (sent) {
        break;
      }
    }
  }

  closeClient();
  return sent;
}

String BaleMessengerBot::sendPostPhoto(JsonObject &payload) {

  bool sent = false;
  String response = "";
  if (_debug)
    Serial.println(F("SEND Post Photo"));
  long sttime = millis();

  if (payload.containsKey("photo")) {
    while (millis() < sttime + 8000) { // loop for a while to send the message
      String command = "bot" + _token + "/sendPhoto";
      response = sendPostToBale(command, payload);
      if (_debug)
        Serial.println(response);
      sent = checkForOkResponse(response);
      if (sent) {
        break;
      }
    }
  }

  closeClient();
  return response;
}

String BaleMessengerBot::sendPhotoByBinary(
    String chat_id, String contentType, int fileSize,
    MoreDataAvailable moreDataAvailableCallback,
    GetNextByte getNextByteCallback) {

  if (_debug)
    Serial.println("SEND Photo");

  String response = sendMultipartFormDataToBale(
      "sendPhoto", "photo", "img.jpg", contentType, chat_id, fileSize,
      moreDataAvailableCallback, getNextByteCallback);

  if (_debug)
    Serial.println(response);

  return response;
}

String BaleMessengerBot::sendPhoto(String chat_id, String photo,
                                       String caption,
                                       int reply_to_message_id) {

  DynamicJsonBuffer jsonBuffer;
  JsonObject &payload = jsonBuffer.createObject();

  payload["chat_id"] = chat_id;
  payload["photo"] = photo;

  if (caption) {
    payload["caption"] = caption;
  }

  if (reply_to_message_id && reply_to_message_id != 0) {
    payload["reply_to_message_id"] = reply_to_message_id;
  }


  return sendPostPhoto(payload);
}

bool BaleMessengerBot::checkForOkResponse(String response) {
  int responseLength = response.length();

  for (int m = 5; m < responseLength + 1; m++) {
    if (response.substring(m - 10, m) ==
        "{\"ok\":true") { // Chek if message has been properly sent
      return true;
    }
  }

  return false;
}

void BaleMessengerBot::closeClient() {
  if (client->connected()) {
    if (_debug) {
      Serial.println(F("Closing client"));
    }
    client->stop();
  }
}
