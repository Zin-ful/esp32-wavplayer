#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include <FS.h>
#include <SdFat.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//please remember to change these for your personal configuration if you device to change any of this
#define UP 16
#define DOWN 17
#define SELECT 18
#define BACK 4
#define WAKE GPIO_NUM_4

//sd card
#define SCK 14
#define MISO 19 //if customizing, do NOT use pin 12. A basic SD module pulls it high preventing boot. Horseshit.
#define MOSI 13
#define CS 5 

#define CHAR_SIZE 1
#define CHAR_W (6 * CHAR_SIZE)
#define CHAR_H (8 * CHAR_SIZE)

#define ARR_SIZE 10
#define MAX_CHAR 20 //this is the rounded result of SCREEN_WIDTH / CHAR_H. No need to have room for more than can be displayed
#define MAX_FILES 1024
#define MAX_DIR 128

#define OLED_RESET -1
#define VISIBLE_LINES (SCREEN_HEIGHT / CHAR_H)

int scroll_offset = 0;

SPIClass spi = SPIClass(VSPI);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SdFs SD_FS;

//will be using magic numbers cause idk how tf else to do it

struct MenuCall {
  const char *name;
  void (*call)(void);
};

struct Menus {
  int index;
  char (*array)[MAX_CHAR];
};

char menu[ARR_SIZE][MAX_CHAR] = { //0
  "Play Random",
  "Song Selection",
  "Clock",
  "Settings",
  "Poweroff",
  ""
};


char settings_menu[ARR_SIZE][MAX_CHAR] = { //1
  "Brightness",
  "Create Folders",
  "Sleep Interval",
  "Time",
  "Check Battery",
  "Diagnostics",
  ""
};

char song_folders[MAX_DIR][MAX_CHAR] = {0}; //2
char songs[MAX_FILES][MAX_CHAR] = {0}; //3
char current_dir[MAX_CHAR] = {0};
int menu_index = 0;
int current_menu = 0;
unsigned long start = millis();
uint8_t current_brightness = 0x7F;

void print(const char *text, int x, int y, int clr = 0) {
  if (clr) display.clearDisplay();
  display.setCursor(x * CHAR_W, y * CHAR_H);
  display.println(text);
}

void print(int value, int x, int y, int clr = 0) {
  if (clr) display.clearDisplay();

  display.setCursor(x * CHAR_W, y * CHAR_H);
  display.println(value);
}

void print(float value, int x, int y, int clr = 0) {
  if (clr) display.clearDisplay();

  display.setCursor(x * CHAR_W, y * CHAR_H);
  display.println(value);
}

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(current_brightness);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  print("Checking SD card module..", 0, 0, 1);
  display.display();
  delay(1000);
  spi.begin(SCK, MISO, MOSI, CS);

  if (!SD.begin(CS, spi, 40000000)) {
    uint8_t cardType = SD.cardType();
    print("Mount Failed", 0, 0, 1);
    display.setCursor(0, CHAR_H * 3);
    display.printf("cardType(): %u\n", cardType);
   
    display.display();
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    print("No SD card attached", 0, 0, 1);
    display.display();
    delay(2000);
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("SD Card Type: ");
  if(cardType == CARD_MMC){
    display.println("MMC");
  } else if(cardType == CARD_SD){
    display.println("SDSC");
  } else if(cardType == CARD_SDHC){
    display.println("SDHC");
  } else {
    display.println("UNKNOWN");
  }
  display.display();
  delay(500);
  //stop before reaching to display errors
  draw_menu();
  pinMode(UP, INPUT_PULLDOWN);
  pinMode(DOWN, INPUT_PULLDOWN);
  pinMode(SELECT, INPUT_PULLDOWN);
}

int get_input() {
  if (digitalRead(UP) == HIGH) {
    return 1;
  } else if (digitalRead(DOWN) == HIGH) {
    return -1;
  } else if (digitalRead(SELECT) == HIGH) {
    return 3;
  } else if (digitalRead(BACK) == HIGH) {
    return 2;
  }
  return 0;
}

void previous_menu() {
  current_menu--;
  if (current_menu < 0) {
    current_menu = 0;
  }
  menu_index = 0;
  scroll_offset = 0;
}

void settings() {
  current_menu = 1;
  menu_index = 0;
  scroll_offset = 0;
  
}

void loop() {
  if (millis() - start >= 100) {
      exec(get_input());
      start = millis();
  }
  display.display();
}

uint8_t byte_me(int choice, uint8_t current) {
  const uint8_t min = 0x00;
  const uint8_t max = 0xFF;

  if (choice == 1) {
    if (current < max) current++;
  } 
  else if (choice == -1) {
    if (current > min + 100) current--;
  }

  return current;
}

void set_brightness() {
  delay(100);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Set Brightness");
  display.setCursor(0, CHAR_H * 2);
  display.println("UP to increase");
  display.setCursor(0, CHAR_H * 4);
  display.println("DOWN to decrease");
  display.display();
  while (1) {
    if (millis() - start >= 50) {

      int inp = get_input();

      if (inp == 3) {
        break;
      }

      if (inp == 1 || inp == -1) {
        current_brightness = byte_me(inp, current_brightness);

        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command(current_brightness);
        display.clearDisplay();
        display.display();
        display.setCursor(0, 0);
        display.println("Set Brightness");
        display.setCursor(0, CHAR_H * 2);
        display.println("UP to increase");
        display.setCursor(0, CHAR_H * 4);
        display.println("DOWN to decrease");
        display.display();
        
      }

      start = millis();
    }
  }
}

void play(char *path) {
  print("Playing", 0, 0, 1);
  display.setCursor(0, adjust(1));
  display.printf("%s", path);
}
void start_clock(){}
void play_random(){}

void select_songs(){
  ls("/", 2);
  current_menu = 2;
  menu_index = 0;
  scroll_offset = 0;
  draw_menu();
}

void poweroff(){
  esp_sleep_enable_ext0_wakeup(WAKE, 1);
  display.clearDisplay();
  display.display();
  esp_deep_sleep_start();
}

void set_sleep(){}

void set_time(){
  delay(200);
  int waittime = 0;
  int num = 0;
  print("Timer:", 0, 0, 1);
  print(waittime, 3, 4, 0);
  display.display();
  delay(200);
  while (1) {
    if (millis() - start >= 100) {
        num = get_input();
        if (num == 3) {
          print(num, 0, 0, 1);
          delay(2000);
          break;
        } else if (num != 0) {
          waittime += num;
          print(waittime, 3, 4, 0);
          display.display();
        }
        start = millis();
    }
  if (waittime) {
    while (waittime) {
      delay(1000);
      waittime--;
      print(waittime, 3, 4, 0);
      display.display();
    }
      while (1) {
      display.fillScreen(WHITE);
      display.display();
      display.fillScreen(BLACK);
      display.display();
        if (millis() - start >= 1000) {
          if (get_input()) {
            break;
          }
          start = millis();
        }
      }      
    } else {
      print("Stopwatch:", 0, 0, 1);
      float time = 0.0;
      print(time, 3, 4, 0);
      display.display();
      while (1) {
        if (millis() - start >= 100) {
          time = get_input();
          if (time) {
            time = 0;
             while (num != 3) {
            start = millis();
            num = get_input();
            delay(100);
            time += 0.1;
            print(time, 3, 4, 1);
            display.display();
            }
            return;
          }
        }
      }
    }
  }
}

void battery_info(){}

void diag() {
  print("Memory Stats:", 0 ,0 ,1);
  display.setCursor(0, adjust(2));
  display.printf("Total:%dkb", (ESP.getHeapSize() / 1024));
  display.setCursor(0, adjust(4));
  display.printf("Free:%dkb", (ESP.getFreeHeap() / 1024));
  display.setCursor(0, adjust(5));
  display.printf("Aval:%dkb", (ESP.getMaxAllocHeap() / 1024));
  display.display();
  delay(1000);
  wait();
}
int add_folder(const char *name) {
  int i = 0;
  for (i; song_folders[i][0] != '\0' && i <= MAX_DIR; i++);
  if (i > MAX_DIR) {
    display.clearDisplay();
    print("Maximum directories reached.", 0, 0, 0);
    print("Directories allowed: 128", 0, 1, 0);
    display.display();
    wait();
    return 0;
  }
  strcpy(song_folders[i], name);
  return 1;
}

int add_file(const char *name) {
  int i = 0;
  for (i; songs[i][0] != '\0' && i <= MAX_FILES; i++);
  if (i > MAX_FILES) {
    display.clearDisplay();
    print("Maximum files reached.", 0, 0, 0);
    print("files allowed per folder: 1024", 0, 1, 0);
    display.display();
    wait();
    return 0;
  }
  strcpy(songs[i], name);
  return 1;
}

void wait() {
  while (get_input() == 0) {
    continue;
  }
  delay(100);
}

void ls(const char *name, int menu_type){

  File root = SD.open(name);
  if(!root){
    display.println("Failed to open card");
    return;
  }
  if(!root.isDirectory()){
    display.println("Won't open non-dir");
    return;
  }

  File file = root.openNextFile();
  while (file){
    if (file.isDirectory()){
      if (menu_type == 2) {
        add_folder(file.name());
      }
    } else {
      if (menu_type == 3) {
        add_file(file.name());
      }
    }
    file = root.openNextFile();
  }
}

int adjust(int pos) {
  return pos * CHAR_H;
}

void format(){
  print("Defaulting folders", 0, 0, 1);
  print("please make sure you", 0, 1, 0);
  print("have formatted this", 0, 2, 0);
  print("card in FAT32", 0, 3, 0);
  print("Otherwise, poweroff", 0, 4, 0);
  print("Press to cont.", 0, 6, 0);
  display.display();
  wait();
  display.clearDisplay();
  delay(300);
  print("THIS WILL ATTEMPT", 0, 0, 1);
  print("TO REMOVE ALL", 0, 1, 0);
  print("FOLDERS", 0, 2, 0);
  print("ENSURE THIS IS", 0, 3, 0);
  print("INTENDED.", 0, 4, 0);
  print("Otherwise, poweroff", 0, 5, 0);
  print("Press to cont.", 0, 6, 0);
  display.display();
  wait();
  display.clearDisplay();

  if (!SD_FS.cardBegin(SdSpiConfig(CS, SHARED_SPI))) {
    uint8_t errCode = SD_FS.sdErrorCode();
    uint8_t errData = SD_FS.sdErrorData();

    switch (SD_FS.sdErrorCode()) {
    case SD_CARD_ERROR_CMD0:
        print("No card / wiring", 0, 0, 0);
        return;

    case SD_CARD_ERROR_CMD8:
        print("Card rejected CMD8", 0, 0, 0);
        return;

    case SD_CARD_ERROR_ACMD41:
        print("Init timeout", 0, 0, 0);
        return;

    default:
        print("Unknown SD error", 0, 0, 0);
        return;
    }
    display.display();
    delay(1500);
    display.clearDisplay();
    return;
  } else {
    print("Card has been", 0, 0, 0);
    print("initialized", 0, 1,0);
    display.display();
    delay(1500);
    display.clearDisplay();
  }
  if (!SD_FS.format()) {
    uint8_t code = SD_FS.sdErrorCode();
    uint8_t data = SD_FS.sdErrorData();
    print("Format failed.", 0, 0, 1);
    switch (SD_FS.sdErrorCode()) {
      case SD_CARD_ERROR_CMD0:
          print("No card / bad wiring", 0, 1, 0);
          return;

      case SD_CARD_ERROR_CMD8:
          print("CMD8 failed", 0, 1, 0);
          return;

      case SD_CARD_ERROR_ACMD41:
          print("Card init timeout", 0, 1, 0);
          return;

      case SD_CARD_ERROR_CMD17:
          print("Read block failed", 0, 1, 0);
          return;

      case SD_CARD_ERROR_CMD24:
          print("Write block failed", 0, 1, 0);
          return;

      default:
          print("Unknown SD error", 0, 1, 0);
          return;
      }
    
    display.display();
    delay(1500);
    display.clearDisplay();
    return;
  } else {
    print("Card has been", 0, 0, 1);
    print("formatted", 0, 1, 0);
    display.display();
    delay(1500);
    display.clearDisplay();
  }

  if (!SD_FS.volumeBegin()) {
    print("Card mount failed.", 0, 0, 1);
    print("Please restart device", 0, 1, 0);
    print("and reformat on a PC.", 0, 2, 0);
    display.display();
    delay(1500);
    display.clearDisplay();
    return;
  } else {
    print("Card has been", 0, 0, 1);
    print("remounted.", 0, 1, 0);
    display.display();
    delay(1500);
    display.clearDisplay();
  }

  print("Removed all folders", 0, 0, 1);
  display.display();
  delay(500);
  display.clearDisplay();


  if (SD.mkdir("/Rock")) {
    print("Created:", 0, 0, 0);
    print("Rock",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("Rock",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
  if (SD.mkdir("/Country")) {
    print("Created:", 0, 0, 0);
    print("Country",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("Country",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
  if (SD.mkdir("/HipHop")) {
    print("Created:", 0, 0, 0);
    print("Hip Hop",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("Hip Hop",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
  if (SD.mkdir("/Lo-Fi")) {
    print("Created:", 0, 0, 0);
    print("Lo-Fi",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("Lo-Fi",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
  if (SD.mkdir("/RnB")) {
    print("Created:", 0, 0, 0);
    print("RnB",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("RnB",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
  if (SD.mkdir("/Pop")) {
    print("Created:", 0, 0, 0);
    print("Pop",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("Pop",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
  if (SD.mkdir("/Classical")) {
    print("Created:", 0, 0, 0);
    print("Classical",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("Classical",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
  if (SD.mkdir("/Misc")) {
    print("Created:", 0, 0, 0);
    print("Misc",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("Misc",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
  if (SD.mkdir("/Audio Books")) {
    print("Created:", 0, 0, 0);
    print("Audio Books",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  } else {
    print("Failed to make:", 0, 0, 0);
    print("Audio Books",0,1, 0);
    display.display();
    delay(500);
    display.clearDisplay();
  }
}

struct Menus menus[] {
  {0, menu},
  {1, settings_menu},
  {2, song_folders},
  {4, songs}
};

struct MenuCall Funcs[] = {
  {"Play Random", play_random},
  {"Song Selection", select_songs},
  {"Clock", start_clock},
  {"Settings", settings},
  {"Poweroff", poweroff},
  {"Brightness", set_brightness},
  {"Sleep Interval", set_sleep},
  {"Time", set_time},
  {"Check Battery", battery_info},
  {"Create Folders", format},
  {"Diagnostics", diag},
  {"Back", previous_menu}
};

void exec(int choice) {
  if (choice == 2) {
    previous_menu();
    draw_menu();
  }
  int array_size = arrlen();

  if (choice && choice < 2) {
    menu_index += choice;

    if (menu_index < 0) {
      menu_index = 0;
    }
    else if (menu_index >= array_size) {
      menu_index = array_size - 1;
    }

    if (menu_index < scroll_offset) {
      scroll_offset = menu_index;
    }

    if (menu_index >= scroll_offset + VISIBLE_LINES) {
      scroll_offset = menu_index - VISIBLE_LINES + 1;
    }

    draw_menu();
  }

  else if (choice == 3) {
    if (current_menu == 2) {
      strcpy(current_dir, "/");
      strcat(current_dir, menus[current_menu].array[menu_index]);
      ls(current_dir, 3);
      current_menu = 3;
      menu_index = 0;
      scroll_offset = 0;
      draw_menu();
    } else if (current_menu == 3) {
      char full_path[256];
      strcpy(full_path, current_dir);
      strcat(full_path, menus[current_menu].array[menu_index]);
      menu_index = 0;
      scroll_offset = 0;
      play(full_path);
    } else {
      for (int i = 0; i < 12; i++) {
        if (strcmp(Funcs[i].name,
                  menus[current_menu].array[menu_index]) == 0) {
          delay(250);
          Funcs[i].call();

          scroll_offset = 0;
          draw_menu();
          delay(250);
          break;
        }
      }
    }
  }
}

int arrlen() {
  int i = 0;
  for (i; menus[current_menu].array[i][0] != '\0'; i++);
  return i;
}

void update(int direction) {
  //resetting previous line
  display.setTextColor(BLACK);
  display.setCursor(0, CHAR_H * (menu_index - direction));
  display.print(">");
  display.setTextColor(WHITE);
  display.println(menus[current_menu].array[menu_index - direction]);
  display.setCursor(0, CHAR_H * menu_index);
  
  display.print(">");
  display.println(menus[current_menu].array[menu_index]);

  display.display();
}

void draw_menu() {
  display.clearDisplay();
  int array_size = arrlen();
  if (!array_size) {
    print("There is nothing within this menu!", 0, 0, 0);
    display.display();
    wait();
    previous_menu();
    return;
  }
  for (int line = 0; line < VISIBLE_LINES; line++) {

    int item = scroll_offset + line;

    if (item >= array_size)
      break;

    display.setCursor(0, line * CHAR_H);

    if (item == menu_index)
      display.print(">");
    else
      display.print(" ");

    display.println(menus[current_menu].array[item]);
  }

  display.display();
}
