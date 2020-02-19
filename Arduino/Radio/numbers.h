IPAddress ip;
HTTPClient http;
Preferences preferences;

unsigned long  last_ir_code = 0;
unsigned long now;
unsigned long last_status_update;
unsigned long last_volume_update;
unsigned long last_api_volume_update;
unsigned long last_time_checked;
unsigned long last_button_update = 0;
unsigned long last_volume_saved = 0;
unsigned long last_power_down_counter_reset = 0;
unsigned long last_auto_power_down_checked = 0;

int mode = 10;
int mute = 0;
int volume = 20;
int rot_value = 40;
int last_volume = 0;
int current_volume = 0;
int play_status = 0;
int last_play_status = 10;

byte current_station = 0;
byte source = 0;
byte last_source = 100;
byte repeat_count = 0;

byte pauseChar[8] = { B11011, B11011, B11011, B11011, B11011, B11011, B11011, B00000 };
byte playChar[8] = { B01000, B01100, B01110, B01111, B01110, B01100, B01000, B00000 };

String text_1 = "";
String last_text_1 = " ";
String text_3 = "00:00";
String last_text_3 = "";
String source_string = "";
String uri = "";
String formattedDate;
String dayStamp;
String timeStamp;
String base_url;

bool backlight = true;
bool power_state = true;
bool old_power_state = true;
bool volume_update_needed;
bool api_volume_update_needed;
bool last_pressed = false;
bool status_update_needed = false;
bool volume_save_needed = false;
bool auto_power_down_muted = false;
bool waiting_for_power_down = false;
