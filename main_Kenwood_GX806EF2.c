#define CLOCK 9600000
#include <inavr.h>

__regvar __no_init unsigned char mode @15;  // режим работы, 0: не перехватываем i2c
__regvar __no_init unsigned char xxx @14;   // временная переменная для поиска START последовательности

unsigned char check1(void)
{
  GIFR|=(1<<INTF0);
  while (!(GIFR&(1<<INTF0)));
  GIFR|=(1<<INTF0);
  if ((PINB&(1<<PB0))==0x00) {xxx=PINB; return 1;}
  return 0;
}

unsigned char check0(void)
{
  GIFR|=(1<<INTF0);
  while (!(GIFR&(1<<INTF0)));
  GIFR|=(1<<INTF0);
  if ((PINB&(1<<PB0))!=0x00) {xxx=PINB; return 1;}
  return 0;
}

void main(void)
{
  mode=0;

  DDRB=0x00;          // все ноги входы
  PORTB=0xFE;         // с подтяжками к питанию

  MCUCR|=0x03;        // прерывание
  GIMSK=(1<<INT0);    // по переднему фронту на INT0 (PB1, SCL)

  xxx=PINB;

  while(1)
  {
  waiting_for_start:
    // нажата ли кнопка?
    if ((PINB&(1<<PB3))==0x00)
    {
      __delay_cycles(CLOCK/1000*300);
      if (mode) mode=0; else mode=1;
    }

    if (mode)
    {
      if (xxx!=PINB)
      {
        if (((PINB&0x03)==0x02)&&(xxx&0x01))
        {
          goto capturing_data;
        }
        xxx=PINB;
      }
    }
    goto waiting_for_start;
  capturing_data:
    // 1
    if (check1()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 1
    if (check1()) goto waiting_for_start;
    // 1
    if (check1()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 1
    if (check1()) goto waiting_for_start;
    // 1
    if (check1()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // 0
    if (check0()) goto waiting_for_start;
    // перенастраиваем INT0 за задний фронт
    MCUCR&=~0x03;
    MCUCR|=0x02;
    GIFR|=(1<<INTF0);
    while (!(GIFR&(1<<INTF0)));
    GIFR|=(1<<INTF0);
    MCUCR|=0x03;

    PORTB|=(1<<PB0);DDRB|=(1<<PB0);

    // 0-1
    if (check1()) goto waiting_for_start;
    // 0-1
    if (check1()) goto waiting_for_start;
    // 0-1
    if (check1()) goto waiting_for_start;
    // 1
    if (check1()) goto waiting_for_start;

    // перенастраиваем INT0 за задний фронт
    MCUCR&=~0x03;
    MCUCR|=0x02;
    // ждём 2 задних фронта
    GIFR|=(1<<INTF0);
    while (!(GIFR&(1<<INTF0)));
    // давим SDA
    PORTB&=~(1<<PB0);DDRB|=(1<<PB0);
    // на один такт
    GIFR|=(1<<INTF0);
    while (!(GIFR&(1<<INTF0)));
    GIFR|=(1<<INTF0);
    MCUCR|=0x03;
    // отпускаем SDA
    DDRB&=~(1<<PB0);
    // идём ждать старта следующей посылки
    goto waiting_for_start;
  }
}
