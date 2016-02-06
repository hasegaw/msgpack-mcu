/*  in_xbee plugin for fluent-bit
 *  End-point device implementation example
 *  =======================================
 *
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2015-2016 Takeshi HASEGAWA <hasegaw@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
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
