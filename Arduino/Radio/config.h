// ESP32 pin asignment
#define SDA_PIN 16                  // i2c sda for display
#define SCL_PIN 17                  // i2c slc for display
#define POWER_RELAIS_PIN 12         // power relais
#define ROTARY_ENCODER_A_PIN 4      // 1st encoder pin
#define ROTARY_ENCODER_B_PIN 15     // 2nd encoder pin
#define ROTARY_ENCODER_BUTTON_PIN 2 // if encoder with click button is used
#define ROTARY_ENCODER_VCC_PIN -1   // set to -1 (unused)
#define RECV_PIN 5                  //pin for IR receiver
#define CS_PIN 23                   //pin in PGA2311->2
#define SDI_PIN 22                  //pin in PGA2311->3
#define SCLK_PIN 21                 //pin in PGA2311->6
#define MUTE_PIN 19                 //pin in PGA2311->8
#define ANALOG_BUTTON_PIN_1 34      //analog in pin for 1st button group
#define ANALOG_BUTTON_PIN_2 35      //analog in pin for 2nd button group

// IR codes
#define IR_UP     0x77E1D055
#define IR_DOWN   0x77E1B055
#define IR_LEFT   0x77E11055
#define IR_RIGHT  0x77E1E055
#define IR_ENTER  0x77E1BA55
#define IR_MENU   0x77E14055
#define IR_PLAY   0x77E17A55
#define IR_REPEAT 0xFFFFFFFF

// sound sources
#define SOURCE_DEFAULT 0
#define SOURCE_TIDAL 1
#define SOURCE_TUNEIN 2
#define SOURCE_SPOTIFY 3
#define SOURCE_LINEIN 4
#define SOURCE_BLUETOOTH 5
#define SOURCE_RADIO 6
#define SOURCE_AIRPLAY 7
#define SOURCE_LINEIN2 8
#define SOURCE_USB 9
#define SOURCE_SPDIF 10

//analog button asignment
#define PLAY_BUTTON 3
#define SOURCE_BUTTON 2
#define NEXT_BUTTON 5
#define PREV_BUTTON 4
#define POWER_BUTTON 13
#define LIGHT_BUTTON_1 8
#define LIGHT_BUTTON_2 9

#define TIME_OFFSET 1 // time offset between UTC
#define SWITCH_BITS 3 // numer of GPIO's used to switch between sources

// sources and bit masks
const int gpio_mode[5] = { SOURCE_DEFAULT, SOURCE_USB, SOURCE_BLUETOOTH, SOURCE_LINEIN, SOURCE_SPDIF };
const int bit_masks[5] = { B011, B010, B001, B111, B000 };
const int gpios[3]     = { 14, 27, 26 };

const int button_array[] = {0, 66, 152, 219, 290, 345, 404};  // analog values representing a certain button

bool linein    = true;  // line in enabled or not
bool bluetooth = true;  // bluetooth enabled or not
bool usb       = true;  // USB enabled or not
bool spdif     = true;  // SPDIF enabled or not

// update intervalls
const int button_update_intervall = 100;
const int time_check_intervall = 30000;
const int update_intervall = 2000;
const int volume_update_intervall = 10;
const int api_volume_update_intervall = 2000;
const int volume_save_intervall = 10000;
const int auto_power_down_time = 3600000;
//const int auto_power_down_time = 60000;
const int auto_power_down_check_intervall = 500;


// define radio stations
const byte station_count = 9;
const String station_names[9]={ "DLF Nova",
                          "Deutschlandfunk",
                          "MDR AKTUELL",
                          "MDR JUMP",
                          "MDR KULTUR",
                          "MDR SPUTNIK",
                          "NDR2",
                          "Bayern 3",
                          "Bayern 1" };

const String stations[9]={  "http://st03.dlf.de/dlf/03/128/mp3/stream.mp3",
                      "http://st01.dlf.de/dlf/01/128/mp3/stream.mp3",
                      "http://mdr-284340-0.cast.mdr.de/mdr/284340/0/mp3/high/stream.mp3",
                      "http://mdr-284320-0.cast.mdr.de/mdr/284320/0/mp3/high/stream.mp3",
                      "http://mdr-284310-0.cast.mdr.de/mdr/284310/0/mp3/high/stream.mp3",
                      "http://mdr-284331-1.cast.mdr.de/mdr/284331/1/mp3/high/stream.mp3",
                      "http://ndr-ndr2-mecklenburgvorpommern.cast.addradio.de/ndr/ndr2/mecklenburgvorpommern/mp3/128/stream.mp3",
                      "http://br-br3-live.cast.addradio.de/br/br3/live/mp3/128/stream.mp3",
                      "http://br-br1-franken.cast.addradio.de/br/br1/franken/mp3/128/stream.mp3" };

// define wireless network of linkplay module
const String sid        = "Linkplay_1";
const String device_ip  = "10.10.10.254";
const String pwd        = "123456789";
