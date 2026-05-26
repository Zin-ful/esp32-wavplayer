#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include <FS.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//please remember to change these for your personal configuration if you device to change any of this
#define UP 16
#define DOWN 17
#define SELECT 18

//sd card
#define SCK 14
#define MISO 19 //if customizing, do NOT use pin 12. A basic SD module pulls it high preventing boot. Horseshit.
#define MOSI 13
#define CS 5 

#define CHAR_SIZE 1
#define CHAR_W (6 * CHAR_SIZE)
#define CHAR_H (8 * CHAR_SIZE)

#define ARR_SIZE 9
#define MAX_FILES 1024
#define MAX_DIR 128

#define OLED_RESET -1

SPIClass spi = SPIClass(VSPI);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//will be using magic numbers cause idk how tf else to do it

struct MenuCall {
  const char *name;
  void (*call)(void);
};

struct Menus {
  int index;
  char (*array)[32];
};

char menu[ARR_SIZE][32] = {  // 0
  "Play Random",
  "Song Selection",
  "Clock",
  "Settings",
  "Poweroff",
  ""
};


char settings_menu[ARR_SIZE][32] = {  // 1
  "Brightness",
  "Rescan SD Card",
  "Create Default Folders",
  "Set Sleep Interval",
  "Set Time",
  "Check Battery",
  "Back",
  ""
};

char song_folders[MAX_DIR][32] = { 0 };  // 2
char songs[MAX_FILES][32] = { 0 };         // 3

int menu_index = 0;
int current_menu = 0;
unsigned long start = millis();
uint8_t current_brightness = 0x7F;



void print(char *text, int x, int y, int clr) {
  if (clr) {
    display.clearDisplay();
  }
  display.setCursor(x * CHAR_W, y * CHAR_H);
  display.println(text);
}

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.ssd1306_command(SSD1306_SETCONTRAST);

  display.ssd1306_command(current_brightness);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Checking SD card module..");
  display.display();
  delay(1000);
  display.clearDisplay();

  spi.begin(SCK, MISO, MOSI, CS);

  if (!SD.begin(CS, spi, 40000000)) {
    uint8_t cardType = SD.cardType();
    display.println("Mount Failed");
    display.setCursor(0, CHAR_H * 3);
    display.printf("cardType(): %u\n", cardType);
   
    display.display();
    return;
}
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    display.println("No SD card attached");
    display.display();
    delay(2000);
    display.clearDisplay();
  }

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
  delay(2000);
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
  }
  return 0;
}

void previous_menu() {
  current_menu = 0;
  menu_index = 0;
  
}


void settings() {
  current_menu = 1;
  menu_index = 0;
  
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

void start_clock(){}
void play_random(){}
void select_songs(){}
void poweroff(){}


void scan(){}
void set_sleep(){}
void set_time(){}
void battery_info(){}

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
  while (!get_input()) {
    continue;
  }
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

void format(){
  display.clearDisplay();
  print("Defaulting Directories", 0, 0, 0);
  print("Please make sure you", 0, 1, 0);
  print("Have formatted this card", 0, 2, 0);
  print("In FAT32", 0, 3, 0);
  print("Otherwise, poweroff", 0, 4, 0);
  print("Press to cont.", 0, 6, 0);
  display.display();
  wait();
  display.clearDisplay();
  if (!SD.mkdir("Rock")) {
    print("Failed to make:", 0, 0, 0);
    print("Rock",0,1, 0);
    display.display();
  }
  if (!SD.mkdir("Country")){
    print("Failed to make:", 0, 0, 0);
    print("Country",0,1, 0);
    display.display();
  }
  if (!SD.mkdir("Hip Hop")){
    print("Failed to make:", 0, 0, 0);
    print("Hip Hop",0,1, 0);
    display.display();
  }
  if (!SD.mkdir("Lo-Fi")){
    print("Failed to make:", 0, 0, 0);
    print("Lo-Fi",0,1, 0);
    display.display();
  }
  if (!SD.mkdir("RnB")){
    print("Failed to make:", 0, 0, 0);
    print("RnB",0,1, 0);
    display.display();
  }
  if (!SD.mkdir("Pop")){
    print("Failed to make:", 0, 0, 0);
    print("Pop",0,1, 0);
    display.display();
  }
  if (!SD.mkdir("Classical")){
    print("Failed to make:", 0, 0, 0);
    print("Classical",0,1, 0);
    display.display();
  }
  if (!SD.mkdir("Misc")){
    print("Failed to make:", 0, 0, 0);
    print("Misc",0,1, 0);
    display.display();
  }
  if (!SD.mkdir("Audio Books")){
    print("Failed to make:", 0, 0, 0);
    print("Audio Books",0,1, 0);
    display.display();
  }
}

struct Menus menus[] {
  {0, menu},
  {1, settings_menu}
};

struct MenuCall Funcs[] = {
  {"Play Random", play_random},
  {"Song Selection", select_songs},
  {"Clock", start_clock},
  {"Settings", settings},
  {"Poweroff", poweroff},
  {"Brightness", set_brightness},
  {"Rescan SD Card", scan},
  {"Set Sleep Interval", set_sleep},
  {"Set Time", set_time},
  {"Check Battery", battery_info},
  {"Create Default Folders", format},
  {"Back", previous_menu}
};



void exec(int choice) {
  int array_size = arrlen();
  if (choice && choice < 2) {
    menu_index += choice;
    if (menu_index < 0) {
      menu_index = 0;
    } else if (menu_index > array_size - 1) {
      menu_index = array_size - 1;
    }
    update(choice);
  } else if (choice == 3) {
    for (int i = 0; i < 11; i++) {
      if (strcmp(Funcs[i].name, menus[current_menu].array[menu_index]) == 0) {
        Funcs[i].call();
        draw_menu();
        delay(100);
        break;
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
  for (int i = 0; i < ARR_SIZE; i++) {
    int y = i * CHAR_H;

    display.setCursor(0, y);

    display.print(" ");
    display.println(menus[current_menu].array[i]);
  }

  display.display();
}
