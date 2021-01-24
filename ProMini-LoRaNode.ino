/*
	LoRa Simple Gateway/Node Exemple

	This code uses InvertIQ function to create a simple Gateway/Node logic.

	Gateway - Sends messages with enableInvertIQ()
					- Receives messages with disableInvertIQ()

	Node		- Sends messages with disableInvertIQ()
					- Receives messages with enableInvertIQ()

	With this arrangement a Gateway never receive messages from another Gateway
	and a Node never receive message from another Node.
	Only Gateway to Node and vice versa.

	This code receives messages and sends a message every second.

	InvertIQ function basically invert the LoRa I and Q signals.

	See the Semtech datasheet, http://www.semtech.com/images/datasheet/sx1276.pdf
	for more on InvertIQ register 0x33.

	created 05 August 2018
	by Luiz H. Cassettari
*/

#include <SPI.h>							 // include libraries
#include <LoRa.h>

#define SERIAL_BAUD	 57600
#define NODE_ID			 3				// NodeId of this LoRa Node
#define MAX_PACKET_SIZE	10

//Message max 30 bytes
struct Payload {
	byte nodeId;
	byte msg [MAX_PACKET_SIZE - 1];
} txPayload;

const long frequency = 866E6;	// LoRa Frequency

const int csPin = 15;					// LoRa radio chip select
const int resetPin = 14;			 // LoRa radio reset
const int irqPin = 2;					// change for your board; must be a hardware interrupt pin


void setup() {
	Serial.begin(SERIAL_BAUD);									 // initialize serial
	while (!Serial);

	Serial.println();
	Serial.print("[LORA-NODE.");
	Serial.print(NODE_ID);
	Serial.println("]");

	LoRa.setPins(csPin, resetPin, irqPin);

	if (!LoRa.begin(frequency)) {
		Serial.println("LoRa init failed. Check your connections.");
		while (true);											 // if failed, do nothing
	}

	Serial.println("LoRa init succeeded.");
	Serial.println();
	Serial.println("Only receive messages from gateways");
	Serial.println("Tx: invertIQ disable");
	Serial.println("Rx: invertIQ enable");
	Serial.println();

	//LoRa.setTxPower(20);
	LoRa.enableCrc();
	LoRa.onReceive(onReceive);
	LoRa.onTxDone(onTxDone);
	LoRa_rxMode();
}


void loop() {
	if (runEvery(10000)) { // repeat every 1000 millis

		txPayload.nodeId = NODE_ID;
		txPayload.msg[0] = 1;
		txPayload.msg[1] = 0x11;
		txPayload.msg[2] = 0x22;
		txPayload.msg[3] = 0x33;
		txPayload.msg[4] = 0x44;
		txPayload.msg[5] = 0x55;
		txPayload.msg[6] = 0x66;
		txPayload.msg[7] = millis() & 0xFF;

		LoRa_sendMessage(txPayload, 9); // send a message

		Serial.println("Send Message!");
	}
}

void LoRa_rxMode(){
	LoRa.enableInvertIQ();								// active invert I and Q signals
	LoRa.receive();											 // set receive mode
}

void LoRa_txMode(){
	LoRa.idle();													// set standby mode
	LoRa.disableInvertIQ();							 // normal mode
}

void LoRa_sendMessage(Payload payload, byte payloadLen) {
	LoRa_txMode();												// set tx mode
	LoRa.beginPacket();									 // start packet
	LoRa.write((byte*) &payload, payloadLen); // add payload
	LoRa.endPacket(true);								 // finish packet and send it
}

void onReceive(int packetSize) {
	byte rxPayload [MAX_PACKET_SIZE];

	byte i = 0, rxByte;

	while (LoRa.available()) {
		rxByte = (byte)LoRa.read();
		if (i < MAX_PACKET_SIZE) {
			rxPayload[i] = rxByte;
			i++;
		}
	}

	// Only accept messages with our NodeId
	if (rxPayload[0] == NODE_ID) {
		Serial.print("Rx packet OK "); // Start received message
		for (char i = 0; i < packetSize; i++) {
				Serial.print(rxPayload[i], DEC);
				Serial.print(' ');
		}
		Serial.println();
	}
}

void onTxDone() {
	Serial.println("TxDone");
	LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
	static unsigned long previousMillis = 0;
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= interval)
	{
		previousMillis = currentMillis;
		return true;
	}
	return false;
}
