This repo consists of a KiCad schematic + model and the code needed to create an ESP32-C3 Mp3 style audio player with a focus on the wav format.

GPIO1  -> PCM5102A BCK  T#2
GPIO2  -> PCM5102A LRCK T#3
GPIO3  -> PCM5102A DATA T#4

GPIO4  -> SD SCK        T#5
GPIO5  -> SD MISO       B#1
GPIO6  -> SD MOSI       B#2
GPIO7  -> SD CS         B#3

GPIO8  -> OLED SDA      B#4
GPIO9  -> OLED SCL      B#5

PCM5102A BCK  GPIO1 ACD1_1
PCM5102A LRCK GPIO2 ACD1_2
PCM5102A DATA GPIO3 ACD1_3
PCM5102A 3.3V 3.3V
PCM5102A GND  GND
SD       SCK  GPIO4 A4 / SCK
SD       MISO GPIO5 A5 / MISO
SD       MOSI GPIO6 MOSI
SD       CS   GPIO7 SS/CS
SSD1306  SDA  GPIO8 SDA
SSD1306  SCL  GPIO9 SCL
