#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <AiEsp32RotaryEncoder.h>
#include <ESP32Ping.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include "numbers.h"
#include "config.h"
#include <Wire.h>
#include <Arduino.h>
#include <IRrecv.h>

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

IRrecv irrecv(RECV_PIN);
decode_results results;

TaskHandle_t Update_Task;

// update display item
void update_display( int item, String update_string ) {
  if ( item == 1 ) {
    int space_count = int((16 - update_string.length())/2);
    lcd.setCursor(0, 1);
  	lcd.print("                ");
    lcd.setCursor(0, 1);
    for(int i = 0; i < space_count; i++) lcd.print(' ');
  	lcd.print(update_string);
    }
  else if ( item == 2 ) {
    lcd.setCursor(6, 0);
  	lcd.print("     ");
    lcd.setCursor(6, 0);
  	lcd.print(update_string);
    }
  else if ( item == 3 ) {
    lcd.setCursor(0, 0);
    int gain = int(update_string.toInt()*255/100/2)-96;
    if ( gain > 0 ) lcd.print("+" + String(gain) + " ");
  	else lcd.print(String(gain) + " ");
    }
  else if ( item == 4 ) {
    if ( update_string == "play" ) {
      lcd.setCursor(15, 0);
    	lcd.write(1);
      }
    else if ( update_string == "pause" ) {
      lcd.setCursor(15, 0);
    	lcd.write(0);
      }
    }
  }


// get time from Linkplay device
void get_time() {
  String payload = performe_get_request(base_url + "getStatus");
  payload.replace("{","");
  payload.replace("}","");
  payload.replace("\"","");
  bool iflag=false;
  String single = "";
  String time_string = "";
  String item = "";
  String value = "";
  for ( int i = 0; i < payload.length(); i++) {
    single = payload[i];
    if (single == ":" and iflag == false) iflag = true;
    else if (single == ",") {
      iflag = false;
      item.trim();
      if ( item == "time" ) time_string = value;
      item = "";
      value = "";
      }
    else {
      if (iflag) value += single;
      else item += single;
      }
    }
  time_string.trim();
  byte stage = 0;
  int hour=0;
  int min=0;
  String tmp_string = "";
  for ( int i = 0; i < time_string.length(); i++) {
    single = time_string[i];
    if ( single == ":") {
      if ( stage == 0 ) hour = tmp_string.toInt() + TIME_OFFSET;
      else if ( stage == 1 ) min = tmp_string.toInt();
      stage++ ;
      tmp_string = "";
      }
    else tmp_string += single;
    }
  if ( hour > 23 ) hour = hour - 24;
  char tmp_str[6];
  sprintf(tmp_str,"%02u:%02u",hour, min);
  text_3 = tmp_str;
  }

// update display with metadata
void get_status() {
  if ( source == SOURCE_USB or source == SOURCE_BLUETOOTH or source == SOURCE_LINEIN or source == SOURCE_SPDIF ) return;
  String payload = performe_get_request(base_url + "getPlayerStatus");
  payload.replace("{","");
  payload.replace("}","");
  payload.replace("\"","");
  bool iflag=false;
  String single = "";
  String item = "";
  String value = "";
  int api_volume;
  for ( int i = 0; i < payload.length(); i++) {
    single = payload[i];
    if (single == ":") iflag = true;
    else if (single == ",") {
      iflag = false;
      if ( item == "mode" ) mode = value.toInt();
      else if ( item == "vol" ) {
        api_volume = value.toInt();
        if ( api_volume != volume) {
          if ( api_volume_update_needed ) {
            performe_get_request(base_url + "setPlayerCmd:vol:" + String(volume));
            api_volume_update_needed = false;
            }
          else {
            volume_update_needed = true;
            volume_save_needed = true;
            volume = api_volume;
            rot_value = volume * 2;
            }
          }
        }
      else if ( item == "mute" ) mute = value.toInt();
      else if ( item == "status" ) {
        if ( value == "stop" ) play_status = 0;
        else if ( value == "play" ) play_status = 1;
        else if ( value == "pause" ) play_status = 2;
        }
      item = "";
      value = "";
      }
    else {
      if (iflag) value += single;
      else item += single;
      }
    }
  uri = "";
  if (value != "") uri = HEXtoString(value);
  source_string = "UNKNOWN";
  byte check_result = is_radio(uri);
  if ( uri.indexOf("tidal.com") > 0) { source_string = "TIDAL"; source=SOURCE_TIDAL;}
  else if ( uri.indexOf("addrad.io") >0 ) { source_string = "TUNEIN"; source=SOURCE_TUNEIN;}
  else if ( mode == 31 ) { source_string = "SPOTIFY"; source=SOURCE_SPOTIFY;}
  else if ( mode == 40 ) { source_string = "LINEIN"; source=SOURCE_LINEIN;}
  else if ( mode == 41 ) { source_string = "BLUETOOTH"; source=SOURCE_BLUETOOTH;}
  else if ( mode == 1 ) { source_string = "AIRPLAY"; source=SOURCE_AIRPLAY;}
  else if ( check_result > 0 and play_status != 0 ) {
    source_string = "RADIO";
    source=SOURCE_RADIO;
    current_station = check_result-1;
    text_1 =  station_names[current_station];
    }
  }

// init 16x2 LDC display
void init_lcd() {
  Serial.println("Starting LCD");
	lcd.begin();
	lcd.backlight();
	lcd.createChar(0, pauseChar);
  lcd.createChar(1, playChar);
	lcd.setCursor(15, 0);
	lcd.write(0);
	lcd.setCursor(0, 0);
	lcd.print("                ");
  lcd.setCursor(0, 1);
	lcd.print("                ");
	}

// setup some things at the beginning
void setup()
  {
  Serial.begin(9600);
  Serial.print("main programm running on core ");
  Serial.println(xPortGetCoreID());
  Wire.begin(SDA_PIN, SCL_PIN);
  init_lcd();
  analogReadResolution(9);
  set_gpios(SOURCE_RADIO);
  pinMode(POWER_RELAIS_PIN, OUTPUT);
  digitalWrite(POWER_RELAIS_PIN, LOW);
  irrecv.enableIRIn();
  //define_radio_stations();
  rotaryEncoder.begin();
  rotaryEncoder.setBoundaries(-10000, 10000, false);
  init_sound();
  mute_on();
  for (int n=0; n < SWITCH_BITS; n++) { pinMode(gpios[n], OUTPUT); }
  update_display(1, "Starting System");
  connect_to_wifi();
  get_status();
  preferences.begin("radio", false);
  current_station = preferences.getInt("station", 0);
  int prev_volume = preferences.getInt("volume", 40);
  Serial.println("Preferences - station: " + String(current_station));
  Serial.println("Preferences - volume : " + String(prev_volume));
  preferences.end();
  Serial.println("init done");
  text_1 =  station_names[current_station];
  play_status = true;
  get_time();
  digitalWrite(POWER_RELAIS_PIN, HIGH);
  power_state = false;
  delay(400);
  if ( prev_volume > 50 ) prev_volume = 50;
  restore_volume(prev_volume);
  Serial.println("starting update task");
  xTaskCreatePinnedToCore(
                    update_from_linkplay,   /* Task function. */
                    "Task1",                /* name of task. */
                    10000,                  /* Stack size of task */
                    NULL,                   /* parameter of the task */
                    1,                      /* priority of the task */
                    &Update_Task,           /* Task handle to keep track of created task */
                    0);                     /* pin task to core 0 */
  delay(500);
  Serial.println("init done");
  mute_off();
  }

void update_from_linkplay( void * pvParameters ) {
  Serial.print("update_from_linkplay running on core ");
  Serial.println(xPortGetCoreID());
  for(;;) {
    if ( now - last_status_update > update_intervall or status_update_needed ) {
      get_status();
      last_status_update = now;
      status_update_needed = false;
      }
    if ( now - last_time_checked > time_check_intervall ) {
      last_time_checked = now;
      get_time();
      }
    delay(300);
    }
  }

// restore volume
void restore_volume(int res_volume) {
  last_volume = 0;
  volume = res_volume-1;
  rot_value = res_volume-1 + res_volume-1;
  set_pga2311_Volume(res_volume-1);
  performe_get_request(base_url + "setPlayerCmd:vol:" + String(res_volume-1));
  if ( millis() > 20000 ) {
    delay(1000);
    last_volume = 0;
    volume = res_volume;
    rot_value = res_volume + res_volume;
    set_pga2311_Volume(res_volume);
    performe_get_request(base_url + "setPlayerCmd:vol:" + String(res_volume));
    }
  }

// init the volume control
void init_sound() {
  pinMode(CS_PIN, OUTPUT);
  pinMode(SDI_PIN, OUTPUT);
  pinMode(SCLK_PIN, OUTPUT);
  pinMode(MUTE_PIN, OUTPUT);
  digitalWrite(MUTE_PIN, LOW);
  delay(500);
  digitalWrite(MUTE_PIN, HIGH);
  }

// disable power amplifier
void mute_on() {
  digitalWrite(MUTE_PIN, LOW);
  }

// enable power amplifier
void mute_off() {
  digitalWrite(MUTE_PIN, HIGH);
  }

// connect to network
void connect_to_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  bool connected = false;
  String host = "";
  String SID = "";
  int discoverd_networks = WiFi.scanNetworks();
  if (discoverd_networks == 0) Serial.println("no networks found");
  else {
    for (int n = 0; n < discoverd_networks; ++n) {
      if ( WiFi.SSID(n) == sid and connected == false ) {
        SID=sid;
        char mysid[sid.length()+1] = "";
        char mypwd[pwd.length()+1] = "";
        sid.toCharArray(mysid, sid.length()+1);
        pwd.toCharArray(mypwd, pwd.length()+1);
        Serial.print("Now connecting to |");
        Serial.print(mysid);
        Serial.print("|");
        Serial.print(mypwd);
        Serial.println("|");
        WiFi.begin(mysid, mypwd);
        base_url = "http://" + device_ip + "/httpapi.asp?command=";
        connected = true;
        host = device_ip;
        }
      }
    }
  byte fail_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
    fail_count++;
    if ( fail_count > 30 ) {
      Serial.println("\nRestarting ESP");
      ESP.restart();
      }
    }
  Serial.println("\nConnection established!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if ( SID != "SAP-Guest") check_ping(host);
  }

// check_device with ping
void check_ping ( String host ) {
  Serial.println("pinging " + host);
  bool ok = false;
  byte count = 0;
  char remote_host[host.length()+1] = "";
  host.toCharArray(remote_host, host.length()+1);
  while ( !ok ) {
    Serial.print(".");
    if( Ping.ping(remote_host) ) {
      Serial.println("\nping OK");
      ok = true;
      }
    count ++;
    }
  }

// update display
void check_update() {
  if ( source != last_source ) {
    set_gpios(source);
    Serial.print("SOURCE: ");
    Serial.println(source);
    text_1 = source_string;
    Serial.println("Switching to " + source_string);
    last_source = source;
    }
  if (volume != last_volume) {
    volume_update_needed = true;
    volume_save_needed = true;
    api_volume_update_needed = true;
    update_display(3, String(volume));
    last_volume = volume;
    }
  if ( last_play_status != play_status and source ) {
    if ( play_status == 0 or play_status == 2 ) update_display(4, "pause");
    else update_display(4, "play");
    last_play_status = play_status;
    }
  if ( last_text_1 != text_1 ) {
    update_display(1, text_1);
    last_text_1 = text_1;
    }
  if ( last_text_3 != text_3 ) {
    update_display(2, text_3);
    last_text_3 = text_3;
    }
  }

// check if currently a radio station is played
byte is_radio(String url) {
  byte ret = 0;
  for (int i = 0; i < station_count; i++) {
    if (stations[i] == url) ret = i+1;
    }
  return ret;
  }

// perform a http get request and return string with return
String performe_get_request(String url) {
  http.begin(url);
  String ret = "";
  int httpCode = http.GET();
  if (httpCode > 0) ret = http.getString();
  return ret;
  }

// convert HEX encoded string to normal string
String HEXtoString(String input) {
  char temp[3];
  char c;
  int i;
  int val;
  String ret = "";
  for (i = 0; i < input.length() - 1; i += 2) {
    temp[0] = input[i];
    temp[1] = input[i + 1];
    val = ASCIIHexToInt(temp[0]) * 16;
    val += ASCIIHexToInt(temp[1]);
    c = toascii(val);
    ret += c;
    }
  return ret;
  }

// convert HEX character into int
int ASCIIHexToInt(char c) {
  int ret = 0;
  if ((c >= '0') && (c <= '9')) ret = (ret << 4) + c - '0';
  else ret = (ret << 4) + toupper(c) - 'A' + 10;
  return ret;
  }

bool playback_controls() {
  if ( source == SOURCE_USB or source == SOURCE_BLUETOOTH or source == SOURCE_LINEIN or source == SOURCE_SPDIF ) return false;
  else return true;
  }

// what to do if a button is pressed
int handle_buttons(byte button_number) {
  Serial.println("Button: " + String(button_number));
  if ( button_number == LIGHT_BUTTON_1 ) {
    performe_get_request("http://192.168.178.46:1234/action=toggle-group-by-name/group=hell");
    }
  if ( button_number == LIGHT_BUTTON_2 ) {
    performe_get_request("http://192.168.178.46:1234/action=toggle-group-by-name/group=warm");
    }
  if ( source == SOURCE_RADIO ) {
    if ( button_number == PLAY_BUTTON ) {
      if ( play_status ) {
        performe_get_request(base_url + "setPlayerCmd:stop");
        text_1 = "RADIO";
        play_status = false;
        }
      else {
        performe_get_request(base_url + "setPlayerCmd:play:" + stations[current_station]);
        text_1 =  station_names[current_station];
        play_status = true;
        }
      }
    else if ( button_number == PREV_BUTTON ) {
      if ( current_station > 0 ) current_station -= 1;
      else  current_station = station_count-1;
      performe_get_request(base_url + "setPlayerCmd:play:" + stations[current_station]);
      text_1 =  station_names[current_station];
      play_status = true;
      preferences.begin("radio", false);
      preferences.putInt("station", current_station);
      preferences.end();
      }
    else if ( button_number == NEXT_BUTTON and playback_controls() ) {
      if ( current_station < station_count-1 ) current_station += 1;
      else current_station = 0;
      performe_get_request(base_url + "setPlayerCmd:play:" + stations[current_station]);
      text_1 =  station_names[current_station];
      play_status = true;
      preferences.begin("radio", false);
      preferences.putInt("station", current_station);
      preferences.end();
      }
    }
  else if ( playback_controls() ) {
    if ( button_number == PLAY_BUTTON ) {
      if ( play_status == 1 ) {
        performe_get_request(base_url + "setPlayerCmd:pause");
        play_status = false;
        }
      else if ( play_status == 2 ){
        performe_get_request(base_url + "setPlayerCmd:resume");
        play_status = true;
        }
      }
    else if ( button_number == PREV_BUTTON ) {
      performe_get_request(base_url + "setPlayerCmd:prev");
      play_status = true;
      }
    else if ( button_number == NEXT_BUTTON ) {
      performe_get_request(base_url + "setPlayerCmd:next");
      play_status = true;
      }
    }
  if ( button_number == SOURCE_BUTTON ) shift_mode();
  status_update_needed = true;
  }

// change operations mode
void shift_mode() {
  bool done = false;
  if ( source == SOURCE_RADIO and !done) {
    source = SOURCE_LINEIN;
    if (linein) {
      source_string = "LINE IN";
      Serial.println("LINE IN enabled");
      done = true;
      performe_get_request(base_url + "setPlayerCmd:stop");
      }
    }
  if ( source == SOURCE_LINEIN and !done) {
    source = SOURCE_BLUETOOTH;
    if (bluetooth) {
      source_string = "BLUETOOTH";
      Serial.println("BLUETOOTH enabled");
      done = true;
      performe_get_request(base_url + "setPlayerCmd:stop");
      }
    }
  if ( source == SOURCE_BLUETOOTH and !done) {
    source = SOURCE_USB;
    if (usb) {
      Serial.println("USB enabled");
      done = true;
      source_string = "USB";
      performe_get_request(base_url + "setPlayerCmd:stop");
      }
    }
  if ( source == SOURCE_USB and !done) {
    source = SOURCE_SPDIF;
    if (spdif) {
      Serial.println("SPDIF enabled");
      done = true;
      source_string = "SPDIF";
      performe_get_request(base_url + "setPlayerCmd:stop");
      }
    }
  if ( !done ) {
    done = true;
    source = SOURCE_RADIO;
    source_string = "RADIO";
    performe_get_request(base_url + "setPlayerCmd:play:" + stations[current_station]);
    text_1 =  station_names[current_station];
    play_status = true;
    }
  check_update();
  }

//  handle rotary button click
void rotary_onButtonClick() {
  handle_buttons(2);
  }

// set GPIO's based on current mode
void set_gpios(byte new_source) {
  Serial.println("Setting bits for " + String(new_source));
  byte index = 0;
  for (int n=0; n < sizeof(gpio_mode)-1; n++) {
    if (new_source == gpio_mode[n] ) index = n;
    }
  //Serial.println("found " + String(index));
  for (int n=0; n < SWITCH_BITS; n++){
    //if ( bitRead(bit_masks[index],n) ) Serial.println("Setting HIGH " + String(gpios[n]));
    //else Serial.println("Setting LOW " + String(gpios[n]));
    if ( bitRead(bit_masks[index],n) ) digitalWrite( gpios[n] , HIGH);
    else digitalWrite( gpios[n] , LOW);
    }
  }

// adjust volume_control
void adjust_vol(int16_t diff) {
  rot_value = rot_value + diff;
  if ( rot_value < 0 ) rot_value = 0;
  else if (rot_value > 200) rot_value = 200;
  volume = int(rot_value/2);
  }

//  check rotary encoder for changes
void rotary_loop() {
  if (rotaryEncoder.currentButtonState() == BUT_RELEASED) rotary_onButtonClick();
  int16_t encoderDelta = rotaryEncoder.encoderChanged();
  if (encoderDelta == 0) return;
  else {
    last_power_down_counter_reset = millis();
    if ( waiting_for_power_down ) return;
    adjust_vol(encoderDelta);
    }
  }

// set pga2311 volume
void set_pga2311_Volume(int vol){
  uint8_t vol_level = map(vol, 0, 100, 0, 255);
  Serial.println("setting PGA volume to " + String(vol_level));
  if ( current_volume < vol_level ) {
    for (int i=current_volume;i<=vol_level;i++){
      talk_to_pga2311(i);
      delay(10);
      }
    }
  if ( current_volume > vol_level ) {
    for (int i=current_volume;i>=vol_level;i--){
      talk_to_pga2311(i);
      delay(10);
      }
    }
  current_volume = vol_level;
  }

// send data to pga2311 chip
void talk_to_pga2311(int vol){
  int voldata[15];
  for (int i=0;i<8;i++){
    voldata[i] = bitRead(vol,7-i);
    voldata[i+8] = bitRead(vol,7-i);
    }
  digitalWrite(CS_PIN,LOW);
  delayMicroseconds(80);
  for(int i=0;i<16;i++){
    digitalWrite(SDI_PIN,voldata[i]);
    delayMicroseconds(80);
    digitalWrite(SCLK_PIN,HIGH);
    delayMicroseconds(80);
    digitalWrite(SDI_PIN,LOW);
    delayMicroseconds(80);
    digitalWrite(SCLK_PIN,LOW);
    delayMicroseconds(80);
    }
  delayMicroseconds(80);
  digitalWrite(CS_PIN,HIGH);
  }

// decode IR code
void decode_ir( unsigned long ir_code ) {
  String cmd = "";
  if ( ir_code == IR_REPEAT ) {
    ir_code =  last_ir_code;
    repeat_count += 1;
    if ( repeat_count < 4 ) return;
    }
  else {
    last_ir_code = ir_code;
    repeat_count = 0;
    }
  if      ( ir_code == IR_UP ) handle_buttons(NEXT_BUTTON);
  else if ( ir_code == IR_DOWN ) handle_buttons(PREV_BUTTON);
  else if ( ir_code == IR_LEFT ) adjust_vol(-1);
  else if ( ir_code == IR_RIGHT ) adjust_vol(1);
  else if ( ir_code == IR_ENTER ) {
    if ( backlight ) {
      lcd.noBacklight();
      backlight = false;
      }
    else {
      lcd.backlight();
      backlight = true;
      }
    }
  else if ( ir_code == IR_MENU ) handle_buttons(SOURCE_BUTTON);
  else if ( ir_code == IR_PLAY ) handle_buttons(PLAY_BUTTON);
  else {
    last_ir_code = 0;
    repeat_count = 0;
    }
  if (last_ir_code != 0 )  Serial.println(cmd);
  }

// check if button is pressed
void check_sony_buttons() {
	int button_value_1 = analogRead(ANALOG_BUTTON_PIN_1);
	int button_value_2 = analogRead(ANALOG_BUTTON_PIN_2);
	bool pressed = false;
	int result = 0;
	for (int i=0; i<6; i++) {
		if (button_value_1 > button_array[i] -15 and button_value_1 < button_array[i] +15) {
			result = i+1;
			pressed = true;
			}
		if (button_value_2 > button_array[i] -15 and button_value_2 < button_array[i] +15) {
			result = i+8;
			pressed = true;
			}
		}
	if (pressed and !last_pressed) {
    last_power_down_counter_reset = millis();
    if ( waiting_for_power_down ) return;
		Serial.print("Button: ");
		Serial.println(result);
    if ( result == 8 ) handle_buttons(LIGHT_BUTTON_1);
    if ( result == 9 ) handle_buttons(LIGHT_BUTTON_2);
    if ( result == 5 and !power_state ) handle_buttons(NEXT_BUTTON);
    // if ( result == 12 ) mute_on();
    // if ( result == 11 ) mute_off();
    else if ( result == 12 ) {
      source = SOURCE_RADIO;
      source_string = "RADIO";
      performe_get_request(base_url + "setPlayerCmd:play:" + stations[current_station]);
      text_1 =  station_names[current_station];
      play_status = true;
      }
    else if ( result == 11 ) {
      source = SOURCE_USB;
      source_string = "USB";
      performe_get_request(base_url + "setPlayerCmd:stop");
      }
    else if ( result == 3 and !power_state ) handle_buttons(PREV_BUTTON);
    else if ( result == 2 and !power_state ) handle_buttons(PLAY_BUTTON);
    else if ( result == 4 and !power_state ) handle_buttons(SOURCE_BUTTON);
    else if ( result == 1 and !power_state ) {
      if ( backlight ) {
        lcd.noBacklight();
        backlight = false;
        }
      else {
        lcd.backlight();
        backlight = true;
        }
      }
    else if ( result == POWER_BUTTON ) {
      if ( power_state ) power_state = false;
      else power_state = true;
      }
		}
	last_pressed = pressed;
	}

// perform auto power down if needed
void check_auto_power_down(unsigned long current_time) {
  if (  now - last_power_down_counter_reset > auto_power_down_time and now - last_power_down_counter_reset < auto_power_down_time + 30000 ) {
    waiting_for_power_down = true;
    if ( auto_power_down_muted ) {
      mute_off();
      auto_power_down_muted = false;
      }
    else {
      mute_on();
      auto_power_down_muted = true;
      }
    }
  else if ( now - last_power_down_counter_reset > auto_power_down_time + 30000 ) {
    power_state = true;
    waiting_for_power_down = false;
    }
  else {
    waiting_for_power_down = false;
    if ( auto_power_down_muted ) {
      mute_off();
      auto_power_down_muted = false;
      }
    }
  }

//  main loop
void loop() {
  now = millis();
  if ( old_power_state != power_state ) {
    Serial.print("powerstate: ");
    Serial.println(power_state);
    old_power_state = power_state;
    if ( power_state ) {
      mute_on();
      performe_get_request(base_url + "setPlayerCmd:pause");
      lcd.noBacklight();
      backlight = false;
      digitalWrite(POWER_RELAIS_PIN, LOW);
      }
    else {
      performe_get_request(base_url + "setPlayerCmd:play:" + stations[current_station]);
      lcd.backlight();
      backlight = true;
      digitalWrite(POWER_RELAIS_PIN, HIGH);
      delay(800);
      restore_volume(volume);
      mute_off();
      }
    }
  if ( power_state == false ) {
    rotary_loop();
    if ( now - last_auto_power_down_checked > auto_power_down_check_intervall) {
      check_auto_power_down(now);
      last_auto_power_down_checked = now;
      }
    if (irrecv.decode(&results)) {
      last_power_down_counter_reset = now;
      if ( waiting_for_power_down ) return;
      decode_ir(results.value);
      irrecv.resume(); // Receive the next value
      }
    if ( volume_update_needed and now-last_volume_update > volume_update_intervall) {
      last_volume_update = now;
      volume_update_needed = false;
      set_pga2311_Volume(volume);
      }
    if ( volume_save_needed and now-last_volume_saved > volume_save_intervall) {
      last_volume_saved = now;
      volume_save_needed = false;
      Serial.println("Saving volume to flash");
      preferences.begin("radio", false);
      preferences.putInt("volume", volume);
      preferences.end();
      }
    check_update();
    }
    if ( now - last_button_update > button_update_intervall) {
    	last_button_update = now;
    	check_sony_buttons();
    	}
  }
