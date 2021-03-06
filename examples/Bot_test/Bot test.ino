#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BaleMessengerBot.h>

//Change SSID, PASSWORD and BOTTOKEN

#define ssid "SSID"
#define password "PASSWORD"
#define BOTtoken "BOTTOKEN"

WiFiClientSecure client;
BaleMessengerBot bot(BOTtoken, client);

long Bot_lasttime;

void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages: ");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "hey " + from_name + ",\nwelcome to bale iot bot.";
      bot.sendMessage(chat_id, welcome);
    }

    else if (text == "1") {
      digitalWrite(LED_BUILTIN, LOW);
      bot.sendMessage(chat_id, "led is on!");
    }
    else if (text == "0") {
      digitalWrite(LED_BUILTIN, HIGH);
      bot.sendMessage(chat_id, "led is off!");
    }

    else {
      bot.sendMessage(chat_id, "What do you mean?");
    }
  }
}


void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setInsecure();
  bot.init();
  Serial.println("READY");
}

void loop() {
  if (millis() - Bot_lasttime > 1000)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    Bot_lasttime = millis();
  }
}
