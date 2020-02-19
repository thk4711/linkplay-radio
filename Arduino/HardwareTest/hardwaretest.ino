/*
------------------------
| GPIO USAGE           |
------------------------
| Analog Button 1 | 34 |
| Analog Button 1 | 35 |
| SDA             | 16 |
| SCL             | 17 |
| Infrared        | 5  |
| cs              | 23 |
| sdi             | 22 |
| sclk            | 21 |
| mute            | 19 |
| ENCODER A       | 4  |
| ENCODER B       | 15 |
| Audio Relais    | 26 |
| Power Relais    | 12 |
| I2S switch S0   | 14 |
| I2S switch S1   | 27 |
------------------------
*/

#include <AiEsp32RotaryEncoder.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <IRrecv.h>
#include <IRutils.h>

bool draw_menue = true;

void setup() {
  Serial.begin(9600);
  }

bool check_for_end_request() {
  bool quit_request = false;
  while (Serial.available()) {
    byte from_serial = Serial.read();
    if (from_serial == 113 ) quit_request = true;
    }
  return quit_request;
  }

void test_analog_buttons() {
  analogReadResolution(9);
  int Button_Pin_1 = 34;
  int Button_Pin_2 = 35;
  while (!check_for_end_request()) {
    int button_value_1 = analogRead(Button_Pin_1);
  	int button_value_2 = analogRead(Button_Pin_2);
    Serial.print("Buttons_1: ");
    Serial.println(button_value_1);
    Serial.print("Buttons_2: ");
    Serial.println(button_value_2);
    delay(500);
    }
  }

void test_IR() {
  int IR_PIN = 5;
  IRrecv irrecv(IR_PIN);
  decode_results results;
  unsigned long ir_code = 0;
  irrecv.enableIRIn();
  while (!check_for_end_request()) {
    if (irrecv.decode(&results)) {
      ir_code = results.value;
      Serial.println(ir_code);
      irrecv.resume();
      }
    }
  }

void test_LCD_display() {
  int SDA_PIN = 16;
  int SCL_PIN = 17;
  Wire.begin(SDA_PIN, SCL_PIN);
  LiquidCrystal_I2C lcd(0x27, 16, 2);
  lcd.begin();
	lcd.backlight();
  delay(1000);
  lcd.noBacklight();
  delay(1000);
  lcd.backlight();
	lcd.setCursor(3, 0);
	lcd.print("test 123");
  int counter = 0;
  while (!check_for_end_request()) {
    lcd.setCursor(3, 1);
    lcd.print(counter);
    counter++;
    delay(500);
    }
  }

void test_pga2311() {
  int cs   = 23;   //pin in PGA2311->2
  int sdi  = 22;   //pin in PGA2311->3
  int sclk = 21;   //pin in PGA2311->6
  int mute = 19;   //pin in PGA2311->8

  pinMode(cs, OUTPUT);
  pinMode(sdi, OUTPUT);
  pinMode(sclk, OUTPUT);
  pinMode(mute, OUTPUT);
  while (!check_for_end_request()) {
    for (int i = 80; i < 180; i++) {
      Serial.println(i);
      volWritePGA2311(i, cs, sdi, sclk);
      delay(50);
      }
    for (int i = 180; i > 80; i--) {
      Serial.println(i);
      volWritePGA2311(i, cs, sdi, sclk);
      delay(50);
      }
    }
  }

void volWritePGA2311(int volNum, int cs, int sdi, int sclk) {
  Serial.println(volNum);
  int voldata[15];
  for (int i=0;i<8;i++){
    voldata[i] = bitRead(volNum,7-i);
    voldata[i+8] = bitRead(volNum,7-i);
    }
  digitalWrite(cs,LOW);
  delayMicroseconds(80);
  for(int i=0;i<16;i++){
    digitalWrite(sdi,voldata[i]);
    delayMicroseconds(80);
    digitalWrite(sclk,HIGH);
    delayMicroseconds(80);
    digitalWrite(sdi,LOW);
    delayMicroseconds(80);
    digitalWrite(sclk,LOW);
    delayMicroseconds(80);
    }
  delayMicroseconds(80);
  digitalWrite(cs,HIGH);
  }

void test_encoder() {
  int ENCODER_A_PIN = 4;
  int ENCODER_B_PIN = 15;
  int ENCODER_BUTTON_PIN = 2;
  int ROTARY_ENCODER_VCC_PIN = -1;
  AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN);
  rotaryEncoder.begin();
  rotaryEncoder.setBoundaries(-10000, 10000, false);
  while (!check_for_end_request()) {
    if (rotaryEncoder.currentButtonState() == BUT_RELEASED) {
      Serial.println("Button pressed");
      }
    int16_t encoderDelta = rotaryEncoder.encoderChanged();
    if (encoderDelta != 0) {
      Serial.println(encoderDelta);
      }
    }
  }

void test_audio_relais() {
  int Relais_pin = 26;
  pinMode(Relais_pin, OUTPUT);
  while (!check_for_end_request()) {
    Serial.println("switching relais on");
    digitalWrite(Relais_pin, HIGH);
    delay(3000);
    Serial.println("switching relais off");
    digitalWrite(Relais_pin, LOW);
    delay(3000);
    }
  }

void test_power_relais() {
  int Relais_pin = 12;
  pinMode(Relais_pin, OUTPUT);
  while (!check_for_end_request()) {
    Serial.println("switching relais on");
    digitalWrite(Relais_pin, HIGH);
    delay(3000);
    Serial.println("switching relais off");
    digitalWrite(Relais_pin, LOW);
    delay(3000);
    }
  }

void test_i2s_switch() {
  int S0_pin = 14;
  int S1_pin = 27;

  pinMode(S0_pin, OUTPUT);
  pinMode(S1_pin, OUTPUT);

  digitalWrite(S0_pin, HIGH);
  digitalWrite(S1_pin, HIGH);

  return;

  while (!check_for_end_request()) {
    Serial.println("S0=0 S1=0");
    digitalWrite(S0_pin, LOW);
    digitalWrite(S1_pin, LOW);
    delay(3000);
    Serial.println("S0=1 S1=0");
    digitalWrite(S0_pin, HIGH);
    digitalWrite(S1_pin, LOW);
    delay(3000);
    Serial.println("S0=0 S1=1");
    digitalWrite(S0_pin, LOW);
    digitalWrite(S1_pin, HIGH);
    delay(3000);
    Serial.println("S0=1 S1=1");
    digitalWrite(S0_pin, HIGH);
    digitalWrite(S1_pin, HIGH);
    delay(3000);
    }
  }

void loop() {
  if (draw_menue) {
    Serial.println("select test");
    Serial.println("------------------");
    Serial.println("analog buttons [1]");
    Serial.println("I2S switch     [2]");
    Serial.println("audio relais   [3]");
    Serial.println("rotary encoder [4]");
    Serial.println("PGA2311 volume [5]");
    Serial.println("LCD display    [6]");
    Serial.println("infrared       [7]");
    Serial.println("power relais   [8]");
    draw_menue = false;
    }
  while (Serial.available()) {
    byte from_serial = Serial.read();
    Serial.print("selection: ");
    Serial.println(from_serial);
    if ( from_serial == 49 ) test_analog_buttons();
    else if ( from_serial == 50 ) test_i2s_switch();
    else if ( from_serial == 51 ) test_audio_relais();
    else if ( from_serial == 52 ) test_encoder();
    else if ( from_serial == 53 ) test_pga2311();
    else if ( from_serial == 54 ) test_LCD_display();
    else if ( from_serial == 55 ) test_IR();
    else if ( from_serial == 56 ) test_power_relais();
    draw_menue = true;
    }
  }

//0, 66, 152, 219, 290, 345, 404
//0, 66, 152, 219, 290, 344, 403
