/*  in_xbee plugin for fluent-bit
 *  End-point device implementation example
 *  =======================================
 *  Copyright (C) 2015 Takeshi HASEGAWA
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * You need to copy both of umsgpack.[ch] into this directory before
 * open and run this sketch. umsgpack.[ch] files are in MessagePack for MCU
 * (msgpack-mcu) distribution.
 * https://github.com/hasegaw/msgpack-mcu
 *
 * XBee2 module should be properly configured with API mode. This sketch
 * assumes serial tx pin is connected to XBee2 module. You may need to
 * convert voltage level between Arduino (5V) and XBee (3.3V).
 */

#define XBEE_SERIAL_SPEED 115200

extern "C" {
	#include "umsgpack.h"
}

void Serial_write(unsigned char i) {
	#if 1
	Serial.write(i);
	#else
	// Send in human-readable ascii code
	char hex_buf[10];
	snprintf(hex_buf, sizeof(hex_buf), "0x%2.2x, ", i & 0xFF);
	Serial.print(hex_buf);
	#endif
}

void xbee2_tx_broadcast(struct umsgpack_packer_buf *buf) {
	unsigned char checksum = 0xFF;  
	unsigned char write_cmd[14] = { 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00 };
	int i;
	int xbee_payload_len = sizeof(write_cmd) + umsgpack_get_length(buf);

	Serial_write((unsigned char) 0x7E);
	Serial_write((unsigned char) xbee_payload_len >> 8);
	Serial_write((unsigned char) xbee_payload_len & 0xFF);

	for (i = 0; i < sizeof(write_cmd); i++) {
		Serial_write(write_cmd[i] & 0xFF);
		checksum -= write_cmd[i] & 0xFF;
	}

	for (i = 0; i < (buf->pos); i++) {
		Serial_write(buf->data[i] & 0xFF);
		checksum -= buf->data[i] & 0xFF;
	}
	Serial_write(checksum & 0xFF);
}

void am2320_msgpack(struct umsgpack_packer_buf *buf, float temp, float humidity) {
	umsgpack_pack_map(buf, 2);
	umsgpack_pack_str(buf, (char *) "degC", 4);
	umsgpack_pack_float(buf, temp);
	umsgpack_pack_str(buf, (char *) "humidity", 8);
	umsgpack_pack_float(buf, humidity);
}

void setup() {
	delay(2000);
	Serial.begin(XBEE_SERIAL_SPEED);
	Serial.println("start");
}

void loop() {
	char mp_buf[100];
	struct umsgpack_packer_buf *buf = (struct umsgpack_packer_buf*)&mp_buf;
	float temp = 23.4F;
	float humidity = 51.2F;

	umsgpack_packer_init(buf, sizeof(mp_buf));
	am2320_msgpack(buf, temp, humidity);

	xbee2_tx_broadcast(buf);
	delay(2000);
}
