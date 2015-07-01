`msgpack` for MCU
=================

It's like JSON but small and fast.

Overview
--------

[MessagePack](http://msgpack.org/) is an efficient binary serialization
format, which lets you exchange data among multiple languages like JSON,
except that it's faster and smaller. Small integers are encoded into a
single byte while typical short strings require only one extra byte in
addition to the strings themselves.

msgpack-mcu is a purpose-built seralizer that targets MCUs
(Micro Controller Units) such as PIC, AVR, and other embedded
microcontrollers.

Typical MCUs usually have limited memory resource (e.g. 32KB for text and 32KB for data). We cannot assume POSIX features and malloc() availability. Most of MCUs have 16bit or 8bit registers, and machine-specific floating point. This library tries to provide those processors unified method to seralize
in MessagePack format.

Example
-------
```c
char mp_buf[100];
struct umsgpack_packer_buf *buf = (struct umsgpack_packer_buf*)&mp_buf;
float temp = 23.4F;
float humidity = 51.2F;

umsgpack_packer_init(buf, sizeof(mp_buf));
umsgpack_pack_map(buf, 2);
umsgpack_pack_str(buf, (char *) "degC", 4);
umsgpack_pack_float(buf, temp);
umsgpack_pack_str(buf, (char *) "humidity", 8);
umsgpack_pack_float(buf, humidity);

for (i = 0; i < umsgpack_get_length(buf); i++) {
    printf("%2.2x",  (char) buf->data[i] & 0xFF);
}
printf("\n");
```

Supported Platforms
-------------------

Please note this project is stil early stage of development and
may not work properly on specific/all platforms.

- NXP JN5148/JN5168: ba2-gcc
- ATMEL AVR Series (incl. Arduino): avr-gcc
- x86, x86_64: gcc, clang

