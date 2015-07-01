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
char buf_[100];
struct msgpack_lite_packer_buf *buf = (struct msgpack_lite_packer_buf*) &buf_;
char sensor_id[10];
double temp = 28.0;
double humidity = 45.0;
int i;

buf->pos = 0;
buf->length = 90;

msgpack_lite_pack_map(buf, 4);
msgpack_lite_pack_str(buf, (char *) "id", 2);
snprintf(&sensor_id, sizeof(sensor_id), "%x", getDeviceSerial());
msgpack_lite_pack_str(buf, &sensor_id, strlen(sensor_id));
msgpack_lite_pack_str(buf, (char *) "degC", 4);
msgpack_lite_pack_float(buf, temp);
msgpack_lite_pack_str(buf, (char *) "humidity", 8);
msgpack_lite_pack_float(buf, humidity);
        
for (i = 0; i < buf->pos; i++) {
    printf("%2.2x",  (char) buf->data[i] & 0xFF);
}
printf("\n");
```
