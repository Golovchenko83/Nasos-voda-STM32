#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <OneWire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GrayOLED.cpp>
#include <GyverTimer.h>
#include <EEPROM.h>
#define OLED_RESET 4

OneWire ds(PB15);
GTimer lsdup(MS);
GTimer analog(US);
GTimer digital(US);
GTimer menu_t(MS, 10000);
GTimer lsd_led(MS);
GTimer rash_litr(MS, 5000);
GTimer Dallos(MS, 3000);
Adafruit_SSD1306 display(OLED_RESET);
HardwareTimer pwm1 TIM1; //Активируем таймер

float bar, set_bar, raz_bar, gist,
           water, water_l, power, bar_lsd, dal_temp[2];

int menu, up, down, rash_tik, bar_tik_lsd,
    A_bar, A_bar_s, tik, i, start, data[12];

byte dal_stat, rash_stat, lsd_stat, menu_ok, menu_z, p_me, state, of, of_of;

const byte adrr[3][8] = {
    {40, 83, 214, 69, 146, 25, 2, 79},
    {40, 248, 43, 69, 146, 14, 2, 35},
    // {40, 23, 77, 69, 146, 14, 2, 163},
};

void EEPROM_float_write(int addr, float val)
{ /* запись в ЕЕПРОМ*/
  byte *x = (byte *)&val;
  for (byte i = 0; i < 4; i++)
    EEPROM.write(i + addr, x[i]);
}

float EEPROM_float_read(int addr)
{ /* чтение из ЕЕПРОМ*/
  byte x[4];
  for (byte i = 0; i < 4; i++)
    x[i] = EEPROM.read(i + addr);
  float *y = (float *)&x;
  return y[0];
}

void sinh()
{
  pwm1.setCaptureCompare(2, 0);
  pwm1.refresh();
  pwm1.setOverflow(i);
  pwm1.setCaptureCompare(2, 100);
  pwm1.resumeChannel(2);
}

void imstop()
{
  pwm1.pause();
}

void off()
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(45, 8);
  display.clearDisplay();
  display.println("OFF");
  display.display();

  detachInterrupt(PB4);
  i = 0;
  sinh();
  imstop();
  digitalWrite(PB8, LOW); // Вентелятор

  for (of; of < 10;)
  {

    delay(1000);
    if (digitalRead(PB12)) //Down
    {
      of++;
    }
  }
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(48, 8);
  display.clearDisplay();
  display.println("ON");
  display.display();
  delay(2000);
  of = 0;
  NVIC_SystemReset();
}

//Активность дисплея
void lsd_led_fun(int t)
{
  if (lsd_stat == 0)
  {
    lsd_stat = 1;
    lsdup.start();
  }
  lsd_led.setTimeout(t);
}

void error_davlenie()
{
  if (water_l == 0 && bar < 0.5 && i != 0)
  {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(35, 0);
    display.clearDisplay();
    display.println("Erorr");
    display.setCursor(20, 16);
    display.print("Davlenie");
    display.display();
    detachInterrupt(PB4);
    i = 0;
    sinh();
    imstop();

    for (size_t error = 0; error < 5;)
    {
      delay(1000);
      if (digitalRead(PB14)) //Меню
      {
        error++;
      }
      if (state)
      {
        tone(PB0, 1000, 50);
        delay(10);
        tone(PB0, 500, 100);
      }
      state = !state;
      if (digitalRead(PB12))
      {
        of_of++;
        if (of_of > 10)
        {
          of_of = 0;
          off();
        }
      }
    }
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(35, 0);
    display.clearDisplay();
    display.println("Reset");
    display.display();
    delay(2000);
    NVIC_SystemReset();
  }
}

void error_temperatura()
{
  if (dal_temp[0] > 75 || dal_temp[1] > 75)
  {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(35, 0);
    display.clearDisplay();
    display.println("Erorr");
    display.setTextSize(1);
    display.setCursor(0, 15);
    display.print("Sim:");
    display.print(dal_temp[0]);
    display.print(" ");
    display.print("Dvig:");
    display.print(dal_temp[1]);
    display.setCursor(30, 24);
    display.print("Peregrev !!!");
    display.display();

    detachInterrupt(PB4);
    i = 0;
    sinh();
    imstop();
    digitalWrite(PB8, HIGH); // Вентелятор

    for (size_t error = 0; error < 5;)
    {
      delay(1000);
      if (digitalRead(PB14)) //Меню
      {
        error++;
      }
      if (state)
      {
        tone(PB0, 1000, 50);
        delay(10);
        tone(PB0, 500, 100);
      }
      state = !state;
      if (digitalRead(PB12))
      {
        of_of++;
        if (of_of > 10)
        {
          of_of = 0;
          off();
        }
      }
    }
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(35, 0);
    display.clearDisplay();
    display.println("Reset");
    display.display();
    delay(2000);
    NVIC_SystemReset();
  }
}

void loop()
{
  //Дисплей текст
  if (lsdup.isReady())
  {

    if (menu_ok == 0)
    {
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.clearDisplay();
      display.print("Pow:");
      display.print(power, 1);
      display.print("%  ");
      // display.setCursor(90, 0);
      display.print(water_l);
      display.print("L/m");
      display.setCursor(120, 0);
      if (rash_stat)
      {
        display.print("-");
      }
      else
      {
        display.print("|");
      }
      display.setCursor(0, 8);
      display.print("Sim:");
      display.print(dal_temp[0]);
      display.print(" ");
      display.print("Dvig:");
      display.print(dal_temp[1]);
      display.setTextSize(2);
      display.setCursor(35, 16);
      bar_lsd = bar_lsd / bar_tik_lsd;
      display.print(bar_lsd, 2);
      bar_tik_lsd = 0;
      display.setTextSize(1);
      display.print(" kg/c");
      // display.setCursor(0, 24);
      // display.print("otklonenie:");
      // display.println(raz_bar, 2);
      display.display();
    }
    else if (menu_ok == 1)
    {
      if (p_me == 1)
      {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(20, 0);
        display.clearDisplay();
        display.println("Davlenie");
        display.setCursor(45, 16);
        display.print(set_bar);
        display.display();
      }
      else if (p_me == 2)
      {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(20, 0);
        display.clearDisplay();
        display.println("Start");
        display.setCursor(45, 16);
        // display.print(start);
        float power_set = map(start, 100, start, 1000, 100);
        power_set = power_set / 10;
        display.print(power);
        display.print("%");
        display.display();
      }
      else if (p_me == 3)
      {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(20, 0);
        display.clearDisplay();
        display.println("Gister");
        display.setCursor(45, 16);
        display.print(gist);
        display.display();
      }
    }
    lsdup.setTimeout(250);
  }
  //*******************

  if (menu_t.isReady()) //Обработка меню
  {
    menu_ok = 0;
    p_me = 0;
    menu = 0;

    if (EEPROM_float_read(0) != gist)
    {
      EEPROM_float_write(0, gist);
    }

    if (EEPROM_float_read(4) != set_bar)
    {
      EEPROM_float_write(4, set_bar);
    }

    if (EEPROM_float_read(8) != start)
    {
      EEPROM_float_write(8, start);
    }
  }

  // Опрос кнопок
  if (digital.isReady())
  {

    if (digitalRead(PB14)) //Меню
    {
      lsd_led_fun(20000);
      menu_t.reset();
      menu++;
      if (menu > 5000 && menu_ok == 0)
      {
        menu_ok = 1;
      }
      if (menu_ok == 1 && menu_z == 0)
      {
        menu_z = 1;
        if (p_me < 3)
        {
          p_me++;
        }
        else
        {
          p_me = 1;
        }
      }
    }
    else
    {

      if (menu_z == 1)
      {
        menu_z = 0;
      }
      if (menu > 0 && menu < 5000)
      {
        menu = 0;
      }
    }

    if (digitalRead(PB13)) //Up
    {
      lsd_led_fun(20000);
      menu_t.reset();
      switch (p_me)
      {
      case 1:
        set_bar = set_bar + 0.000005;
        break;
      case 2:
        start = start + 1;
        delay(5);
        break;
      case 3:
        gist = gist + 0.00001;
        break;
      }
    }
    if (digitalRead(PB12)) //Down
    {
      lsd_led_fun(20000);
      menu_t.reset();
      switch (p_me)
      {
      case 1:
        set_bar = set_bar - 0.000005;
        break;
      case 2:
        start = start - 1;
        delay(5);
        break;
      case 3:
        gist = gist - 0.00001;
        break;
      default:
        of_of++;
        if (of_of > 5000)
        {
          of_of = 0;
          off();
        }
      }
    }
    //Опрос датчика расхода воды
    if (digitalRead(PB7) && rash_stat == 0)
    {
      rash_stat = 1;
      rash_tik++;
    }
    else if (digitalRead(PB7) == 0 && rash_stat == 1)
    {
      rash_stat = 0;
      rash_tik++;
    }
    //*******************
    digital.setTimeout(30);
  }

  //Считывание датчиков
  if (analog.isReady())
  {
    //Считывание датчиков
    A_bar = A_bar + analogRead(PA0); //Считывание давления С PA0
    tik++;

    // расчет давления, управление Мощьностью насоса
    if (tik >= 20)
    {
      A_bar_s = A_bar / tik;
      A_bar = 0;
      tik = 0;
      if (A_bar_s <= 370)
      {
        bar = map(A_bar_s, 178, 370, 0, 100);
      }
      else if (A_bar_s <= 503)
      {
        bar = map(A_bar_s, 370, 503, 100, 180);
      }
      else
      {
        bar = map(A_bar_s, 503, 649, 180, 300);
      }

      bar = bar / 100;
      bar_lsd = bar_lsd + bar;
      bar_tik_lsd++;
      raz_bar = set_bar - bar;

      if (raz_bar > gist)
      {
        if (i == 0)
        {
          attachInterrupt(PB4, sinh, FALLING);
          i = start;
        }
        else
        {
          if (raz_bar > 0.1 && i > 100)
          {
            i = i - 5;
            if (i < 100)
            {
              i = 100;
            }
          }
        }
      }
      else
      {
        if (i != 0)
        {
          if (raz_bar < 0.1 && i < start && water != 0)
          {
            i = i + 15;
          }
        }
        //Отключение при достежений давления

        if (raz_bar <= -0.15)
        {
          detachInterrupt(PB4);
          i = 0;
          // time_ns = 0;
          // time_n = 0;
          sinh();
          imstop();
        }
        //Дакачка при нулевом расходе
        if (water == 0 && i != 0)
        {
          if (i > 100)
          {
            i = i - 1;
          }
        }
      }
      if (i == 0)
      {
        power = 0;
      }
      else
      {
        power = map(i, 100, start, 1000, 100);
        power = power / 10;
      }
    }

    //Расчет расхода воды
    if (rash_litr.isReady())
    {
      water = rash_tik;
      rash_tik = 0;

      if (water < 26)
      {
        water_l = map(water, 0, 26, 0, 860);
      }
      else if (water > 26 && water < 215)
      {
        water_l = map(water, 27, 215, 860, 3900);
      }
      else if (water > 215)
      {
        water_l = map(water, 215, 500, 3900, 11500);
      }

      water_l = water_l / 1000;
    }
    //*******************
    if (water > 0 || i > 0)
    {
      lsd_led_fun(20000);
    }

    if (lsd_led.isReady())
    {
      lsdup.stop();
      lsd_stat = 0;
      display.clearDisplay();
      display.display();
    }
    analog.setTimeout(150);
  }

  //Температура ***DS18B20**
  if (Dallos.isReady())
  {
    //проверка на ошибки
    error_davlenie();
    error_temperatura();

    if (dal_stat == 0)
    {
      ds.reset();
      ds.write(0xCC);
      ds.write(0x44);
      dal_stat = 1;
    }
    else
    {
      for (size_t i = 0; i < 3; i++)
      {
        ds.reset();
        ds.select(adrr[i]);
        ds.write(0xBE);

        for (byte i = 0; i < 9; i++)
        {
          data[i] = ds.read();
        }
        uint16_t raw = (data[1] << 8) | data[0];
        dal_temp[i] = (float)raw / 16.0;
      }
      dal_stat = 0;
      if (dal_temp[1] > 30 || i > 0)
      {
        digitalWrite(PB8, HIGH);
      }
      else if (dal_temp[1] < 27 && i == 0)
      {
        digitalWrite(PB8, LOW);
      }
      //ставим 0 если ошибка чтения
      if (dal_temp[0] > 200)
      {
        dal_temp[0] = 0;
      }
      if (dal_temp[1] > 200)
      {
        dal_temp[1] = 0;
      }
    }
  }
  //************************
}

void setup()
{

  pinMode(PB4, INPUT_PULLUP);                      // Ноль сети
                                                   // pinMode(PB15, INPUT_PULLUP); // Ноль сети
  pinMode(PB12, INPUT);                            // Кнопка 3
  pinMode(PB13, INPUT);                            // Кнопка 2
  pinMode(PB14, INPUT);                            // Кнопка 1
  pinMode(PB8, OUTPUT);                            // Вентелятор
  detachInterrupt(PB4);                            //
  pinMode(PB7, INPUT);                             // Датчик расхода воды
  pwm1.setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA9); // Оптопа симистора
  pwm1.attachInterrupt(2, imstop);
  pwm1.setPrescaleFactor(15);
  pwm1.setCaptureCompare(2, 100);
  Wire.setSDA(PB11);
  Wire.setSCL(PB10);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.setClock(400000);
  digital.setTimeout(30);
  analog.setTimeout(150);
  lsd_led.setTimeout(20000);
  lsdup.setTimeout(250);
  //EEPROM_float_write(0, 0.2);
  //EEPROM_float_write(4, 2.9);
 // EEPROM_float_write(8, 20000);
  gist = EEPROM_float_read(0);
  set_bar =EEPROM_float_read(4);
  start = EEPROM_float_read(8);
  
}
