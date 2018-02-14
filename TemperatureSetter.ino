/*
    TemperatureSetter

    Arduino based circuit to set target temperature for my heating system
*/

#include <Arduino.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <avdweb_Switch.h>

// Update these with values suitable for your network.
const byte mac[] = {0x02, 0x4c, 0xce, 0xae, 0x63, 0x69};
const char* mqttServer = "mqtt.aelius.co.uk";
const int mqttPort = 1883;

const char* underfloor_target_topic = "heating/underfloor/target";
const char* underfloor_set_topic = "heating/underfloor/set";
const char* radiators_target_topic = "heating/radiators/target";
const char* radiators_set_topic = "heating/radiators/set";
const char* status_topic = "heating/temperature-setter";


#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define TEMPERATURE_ON     20
#define TEMPERATURE_OFF    10
#define TEMPERATURE_BOOST  22

#define RADIATORS_LED_PIN  (5)
#define BOOST_LED_PIN      (6)
#define UNDERFLOOR_LED_PIN (7)

#define RADIATOR_BUTTON_PIN   (A0)
#define BOOST_BUTTON_PIN      (A1)
#define UNDERFLOOR_BUTTON_PIN (A2)


int underfloor_target = -1;
int radiators_target = -1;

// Button to GND, internal pull-up resistor
Switch radiatorButton = Switch(RADIATOR_BUTTON_PIN, INPUT_PULLUP, LOW);
Switch boostButton = Switch(BOOST_BUTTON_PIN, INPUT_PULLUP, LOW);
Switch underfloorButton = Switch(UNDERFLOOR_BUTTON_PIN, INPUT_PULLUP, LOW);

EthernetClient ethClient;
PubSubClient client(ethClient);


void callback(char* topic, byte* payload, unsigned int length) {
    String str;

    // Payload it not a null-terminated string
    for (int i=0; i<length; i++) {
        str += (char)payload[i];
    }

    if (strcmp(topic, underfloor_target_topic) == 0) {
        Serial.print("Underfloor: ");
        underfloor_target = str.toInt();
        Serial.println(underfloor_target, DEC);
    } else if (strcmp(topic, radiators_target_topic) == 0) {
        Serial.print("Radiators: ");
        radiators_target = str.toInt();
        Serial.println(radiators_target, DEC);
    }

    // Now Set the LEDs
    if (radiators_target >= TEMPERATURE_BOOST) {
        digitalWrite(BOOST_LED_PIN, HIGH);
        digitalWrite(RADIATORS_LED_PIN, HIGH);
    } else if (radiators_target > TEMPERATURE_OFF) {
        digitalWrite(BOOST_LED_PIN, LOW);
        digitalWrite(RADIATORS_LED_PIN, HIGH);
    } else {
        digitalWrite(BOOST_LED_PIN, LOW);
        digitalWrite(RADIATORS_LED_PIN, LOW);
    }

    if (underfloor_target > TEMPERATURE_OFF) {
        digitalWrite(UNDERFLOOR_LED_PIN, HIGH);
    } else {
        digitalWrite(UNDERFLOOR_LED_PIN, LOW);
    }
}


void toggleRadiators()
{
    if (radiators_target > TEMPERATURE_OFF) {
        Serial.println(F("Setting Radiators to " STR(TEMPERATURE_OFF)));
        client.publish(radiators_set_topic, STR(TEMPERATURE_OFF));
    } else {
        Serial.println(F("Setting Radiators to " STR(TEMPERATURE_ON)));
        client.publish(radiators_set_topic, STR(TEMPERATURE_ON));
    }
}

void toggleBoost()
{
    if (radiators_target >= TEMPERATURE_BOOST) {
        Serial.println(F("Setting Radiators to " STR(TEMPERATURE_ON)));
        client.publish(radiators_set_topic, STR(TEMPERATURE_ON));
    } else {
        Serial.println(F("Setting Radiators to " STR(TEMPERATURE_BOOST)));
        client.publish(radiators_set_topic, STR(TEMPERATURE_BOOST));
    }
}

void toggleUnderfloor()
{
    if (underfloor_target > TEMPERATURE_OFF) {
        Serial.println(F("Setting Underfloor to " STR(TEMPERATURE_OFF)));
        client.publish(underfloor_set_topic, STR(TEMPERATURE_OFF));
    } else {
        Serial.println(F("Setting Underfloor to " STR(TEMPERATURE_ON)));
        client.publish(underfloor_set_topic, STR(TEMPERATURE_ON));
    }
}


void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print(F("Attempting MQTT connection..."));
        // Attempt to connect
        if (client.connect("temperature-setter", status_topic, /*qos*/ 1, /*retain*/ true, "offline")) {
            Serial.println(F("connected"));

            // Publish that we are now online
            client.publish(status_topic, "online", /*retain*/ true);

            // Subscribe
            client.subscribe(underfloor_target_topic);
            client.subscribe(radiators_target_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");

            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println(F("[TemperatureSetter]"));

    pinMode(RADIATORS_LED_PIN, OUTPUT);
    pinMode(BOOST_LED_PIN, OUTPUT);
    pinMode(UNDERFLOOR_LED_PIN, OUTPUT);

    // Allow the hardware to sort itself out
    delay(1500);

    // Start the Ethernet connection
    Serial.println(F("Configuring Ethernet"));
    if (Ethernet.begin(mac) == 0) {
        Serial.println(F("Failed to configure Ethernet using DHCP"));
        while(1);
    }

    // Print our the IP address
    Serial.print("IP=");
    Serial.println(Ethernet.localIP());

    // Configure MQTT client
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);

    Serial.println(F("Ready."));
}

void loop()
{
    radiatorButton.poll();
    if (radiatorButton.pushed()) {
        toggleRadiators();
    }

    boostButton.poll();
    if (boostButton.pushed()) {
        toggleBoost();
    }

    underfloorButton.poll();
    if (underfloorButton.pushed()) {
        toggleUnderfloor();
    }

    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}
