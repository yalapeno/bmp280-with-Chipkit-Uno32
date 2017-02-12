/*
 * I2C Example project for the mcb32 toolchain
 * Demonstrates the temperature sensor and display of the Basic IO Shield
 * Make sure your Uno32-board has the correct jumper settings, as can be seen
 * in the rightmost part of this picture:
 * https://reference.digilentinc.com/_media/chipkit_uno32:jp6_jp8.png?w=300&tok=dcceb2
 */


#include <pic32mx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define DISPLAY_VDD_PORT PORTF
#define DISPLAY_VDD_MASK 0x40
#define DISPLAY_VBATT_PORT PORTF
#define DISPLAY_VBATT_MASK 0x20
#define DISPLAY_COMMAND_DATA_PORT PORTF
#define DISPLAY_COMMAND_DATA_MASK 0x10
#define DISPLAY_RESET_PORT PORTG
#define DISPLAY_RESET_MASK 0x200

/* Address of the temperature sensor on the I2C bus */
#define TEMP_SENSOR_ADDR 0x77


/* Temperature sensor internal registers */
typedef enum TempSensorReg TempSensorReg;
enum TempSensorReg {
  TEMP_SENSOR_REG_TEMP,
  TEMP_SENSOR_REG_CONF,
  TEMP_SENSOR_REG_HYST,
  TEMP_SENSOR_REG_LIMIT,
};

char basincyazi[12];
int basinc;
int derece;
char dereceyazi[12];
int timeoutcount = 0;
char ortalamayazi[12];

char textbuffer[4][16];

static const uint8_t const font[] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 2, 5, 2, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 94, 0, 0, 0, 0,
  0, 0, 4, 3, 4, 3, 0, 0,
  0, 36, 126, 36, 36, 126, 36, 0,
  0, 36, 74, 255, 82, 36, 0, 0,
  0, 70, 38, 16, 8, 100, 98, 0,
  0, 52, 74, 74, 52, 32, 80, 0,
  0, 0, 0, 4, 3, 0, 0, 0,
  0, 0, 0, 126, 129, 0, 0, 0,
  0, 0, 0, 129, 126, 0, 0, 0,
  0, 42, 28, 62, 28, 42, 0, 0,
  0, 8, 8, 62, 8, 8, 0, 0,
  0, 0, 0, 128, 96, 0, 0, 0,
  0, 8, 8, 8, 8, 8, 0, 0,
  0, 0, 0, 0, 96, 0, 0, 0,
  0, 64, 32, 16, 8, 4, 2, 0,
  0, 62, 65, 73, 65, 62, 0, 0,
  0, 0, 66, 127, 64, 0, 0, 0,
  0, 0, 98, 81, 73, 70, 0, 0,
  0, 0, 34, 73, 73, 54, 0, 0,
  0, 0, 14, 8, 127, 8, 0, 0,
  0, 0, 35, 69, 69, 57, 0, 0,
  0, 0, 62, 73, 73, 50, 0, 0,
  0, 0, 1, 97, 25, 7, 0, 0,
  0, 0, 54, 73, 73, 54, 0, 0,
  0, 0, 6, 9, 9, 126, 0, 0,
  0, 0, 0, 102, 0, 0, 0, 0,
  0, 0, 128, 102, 0, 0, 0, 0,
  0, 0, 8, 20, 34, 65, 0, 0,
  0, 0, 20, 20, 20, 20, 0, 0,
  0, 0, 65, 34, 20, 8, 0, 0,
  0, 2, 1, 81, 9, 6, 0, 0,
  0, 28, 34, 89, 89, 82, 12, 0,
  0, 0, 126, 9, 9, 126, 0, 0,
  0, 0, 127, 73, 73, 54, 0, 0,
  0, 0, 62, 65, 65, 34, 0, 0,
  0, 0, 127, 65, 65, 62, 0, 0,
  0, 0, 127, 73, 73, 65, 0, 0,
  0, 0, 127, 9, 9, 1, 0, 0,
  0, 0, 62, 65, 81, 50, 0, 0,
  0, 0, 127, 8, 8, 127, 0, 0,
  0, 0, 65, 127, 65, 0, 0, 0,
  0, 0, 32, 64, 64, 63, 0, 0,
  0, 0, 127, 8, 20, 99, 0, 0,
  0, 0, 127, 64, 64, 64, 0, 0,
  0, 127, 2, 4, 2, 127, 0, 0,
  0, 127, 6, 8, 48, 127, 0, 0,
  0, 0, 62, 65, 65, 62, 0, 0,
  0, 0, 127, 9, 9, 6, 0, 0,
  0, 0, 62, 65, 97, 126, 64, 0,
  0, 0, 127, 9, 9, 118, 0, 0,
  0, 0, 38, 73, 73, 50, 0, 0,
  0, 1, 1, 127, 1, 1, 0, 0,
  0, 0, 63, 64, 64, 63, 0, 0,
  0, 31, 32, 64, 32, 31, 0, 0,
  0, 63, 64, 48, 64, 63, 0, 0,
  0, 0, 119, 8, 8, 119, 0, 0,
  0, 3, 4, 120, 4, 3, 0, 0,
  0, 0, 113, 73, 73, 71, 0, 0,
  0, 0, 127, 65, 65, 0, 0, 0,
  0, 2, 4, 8, 16, 32, 64, 0,
  0, 0, 0, 65, 65, 127, 0, 0,
  0, 4, 2, 1, 2, 4, 0, 0,
  0, 64, 64, 64, 64, 64, 64, 0,
  0, 0, 1, 2, 4, 0, 0, 0,
  0, 0, 48, 72, 40, 120, 0, 0,
  0, 0, 127, 72, 72, 48, 0, 0,
  0, 0, 48, 72, 72, 0, 0, 0,
  0, 0, 48, 72, 72, 127, 0, 0,
  0, 0, 48, 88, 88, 16, 0, 0,
  0, 0, 126, 9, 1, 2, 0, 0,
  0, 0, 80, 152, 152, 112, 0, 0,
  0, 0, 127, 8, 8, 112, 0, 0,
  0, 0, 0, 122, 0, 0, 0, 0,
  0, 0, 64, 128, 128, 122, 0, 0,
  0, 0, 127, 16, 40, 72, 0, 0,
  0, 0, 0, 127, 0, 0, 0, 0,
  0, 120, 8, 16, 8, 112, 0, 0,
  0, 0, 120, 8, 8, 112, 0, 0,
  0, 0, 48, 72, 72, 48, 0, 0,
  0, 0, 248, 40, 40, 16, 0, 0,
  0, 0, 16, 40, 40, 248, 0, 0,
  0, 0, 112, 8, 8, 16, 0, 0,
  0, 0, 72, 84, 84, 36, 0, 0,
  0, 0, 8, 60, 72, 32, 0, 0,
  0, 0, 56, 64, 32, 120, 0, 0,
  0, 0, 56, 64, 56, 0, 0, 0,
  0, 56, 64, 32, 64, 56, 0, 0,
  0, 0, 72, 48, 48, 72, 0, 0,
  0, 0, 24, 160, 160, 120, 0, 0,
  0, 0, 100, 84, 84, 76, 0, 0,
  0, 0, 8, 28, 34, 65, 0, 0,
  0, 0, 0, 126, 0, 0, 0, 0,
  0, 0, 65, 34, 28, 8, 0, 0,
  0, 0, 4, 2, 4, 2, 0, 0,
  0, 120, 68, 66, 68, 120, 0, 0,
};
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value

uint32_t t_fine;
uint16_t dig_T1; //u16
int16_t dig_T2; //s16
int16_t dig_T3; //s16
uint16_t dig_P1; //u16
int16_t dig_P2;  //s16
int16_t dig_P3;  //s16
int16_t dig_P4;  //s16
int16_t dig_P5;  //s16
int16_t dig_P6;  //s16
int16_t dig_P7;  //s16
int16_t dig_P8;  //s16
int16_t dig_P9;  //s16

void delay_timer(int sec){
  /*start the timer*/
  T2CONSET = 0x8000;
  int k = sec*10;
  int i;
  for (i = 0; i < k; ++i)
  {
    while(!(IFS(0) &0x100)){

    }
    IFS(0) = 0;
  }
}
void delay(int cyc) {
  int i;
  for(i = cyc; i > 0; i--);
}

uint8_t spi_send_recv(uint8_t data) {
  while(!(SPI2STAT & 0x08));
  SPI2BUF = data;
  while(!(SPI2STAT & 0x01));
  return SPI2BUF;
}

void display_init() {
  DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
  delay(10);
  DISPLAY_VDD_PORT &= ~DISPLAY_VDD_MASK;
  delay(1000000);
  
  spi_send_recv(0xAE);
  DISPLAY_RESET_PORT &= ~DISPLAY_RESET_MASK;
  delay(10);
  DISPLAY_RESET_PORT |= DISPLAY_RESET_MASK;
  delay(10);
  
  spi_send_recv(0x8D);
  spi_send_recv(0x14);
  
  spi_send_recv(0xD9);
  spi_send_recv(0xF1);
  
  DISPLAY_VBATT_PORT &= ~DISPLAY_VBATT_MASK;
  delay(10000000);
  
  spi_send_recv(0xA1);
  spi_send_recv(0xC8);
  
  spi_send_recv(0xDA);
  spi_send_recv(0x20);
  
  spi_send_recv(0xAF);
}

void display_string(int line, char *s) {
  int i;
  if(line < 0 || line >= 4)
    return;
  if(!s)
    return;
  
  for(i = 0; i < 16; i++)
    if(*s) {
      textbuffer[line][i] = *s;
      s++;
    } else
      textbuffer[line][i] = ' ';
}

void display_update() {
  int i, j, k;
  int c;
  for(i = 0; i < 4; i++) {
    DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
    spi_send_recv(0x22);
    spi_send_recv(i);
    
    spi_send_recv(0x0);
    spi_send_recv(0x10);
    
    DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
    
    for(j = 0; j < 16; j++) {
      c = textbuffer[i][j];
      if(c & 0x80)
        continue;
      
      for(k = 0; k < 8; k++)
        spi_send_recv(font[c*8 + k]);
    }
  }
}

/* Wait for I2C bus to become idle */
void i2c_idle() {
  while(I2C1CON & 0x1F || I2C1STAT & (1 << 14)); //TRSTAT
}

/* Send one byte on I2C bus, return ack/nack status of transaction */
bool i2c_send(uint8_t data) {
  i2c_idle();
  I2C1TRN = data;
  i2c_idle();
  return !(I2C1STAT & (1 << 15)); //ACKSTAT
}

/* Receive one byte from I2C bus */
uint8_t i2c_recv() {
  i2c_idle();
  I2C1CONSET = 1 << 3; //RCEN = 1
  i2c_idle();
  I2C1STATCLR = 1 << 6; //I2COV = 0
  return I2C1RCV;
}

/* Send acknowledge conditon on the bus */
void i2c_ack() {
  i2c_idle();
  I2C1CONCLR = 1 << 5; //ACKDT = 0
  I2C1CONSET = 1 << 4; //ACKEN = 1
}

/* Send not-acknowledge conditon on the bus */
void i2c_nack() {
  i2c_idle();
  I2C1CONSET = 1 << 5; //ACKDT = 1
  I2C1CONSET = 1 << 4; //ACKEN = 1
}

/* Send start conditon on the bus */
void i2c_start() {
  i2c_idle();
  I2C1CONSET = 1 << 0; //SEN
  i2c_idle();
}

/* Send restart conditon on the bus */
void i2c_restart() {
  i2c_idle();
  I2C1CONSET = 1 << 1; //RSEN
  i2c_idle();
}

/* Send stop conditon on the bus */
void i2c_stop() {
  i2c_idle();
  I2C1CONSET = 1 << 2; //PEN
  i2c_idle();
}




int bmp280_compensate_T_double(uint32_t adc_T)
{
double var1, var2, T, gad;
double d1 = (double)dig_T1;
double d2 = (double)dig_T2;
double d3 = (double)dig_T3;
var1 = (double)adc_T;
var1 /= 16384.0;
var1 -= (d1/1024.0);
var1 *=d2;
//var1 = (((double)adc_T)/16384.0 – ((double)dig_T1)/1024.0) * ((double)dig_T2);

var2 = (double)adc_T;
var2 /= 131072.0;
var2 -= (d1/8192.0);
var2 = var2*var2;
var2 *= d3 ;


//var2 = ((((double)adc_T)/131072.0 – ((double)dig_T1)/8192.0) * (((double)adc_T)/131072.0 – ((double) dig_T1)/8192.0)) * ((double)dig_T3);
t_fine = (uint32_t)(var1 + var2);
T = (var1 + var2) / 5120.0;

T *= 100;
int Ti = (int) T;
return Ti;
}
int bmp280_compensate_P_double(uint32_t adc_P)
{
  int r;
  double var1,var2,p;
  double t_fine_double = (double) t_fine;
  double adc_P_double = (double)adc_P;

  double d1 = (double)dig_P1;
  double d2 = (double)dig_P2;
  double d3 = (double)dig_P3;
  double d4 = (double)dig_P4;
  double d5 = (double)dig_P5;
  double d6 = (double)dig_P6;
  double d7 = (double)dig_P7;
  double d8 = (double)dig_P8;
  double d9 = (double)dig_P9;

  var1 = (t_fine_double/2.0) - 64000.0;

  var2 = var1*var1*d6/32768.0;
  var2 = var2 + var1*d5*2.0;
  var2 = (var2/4.0) +(d4*65536.0);

  var1 = d3*var1*var1/524288.0 + (d2*var1)/524288.0;
  var1 = (1.0+var1/32768.0)*d1;
  if(var1==0.0){
    return 0; //undvik exception devide by zero
  }
  p = 1048576.0 - adc_P_double;
  p = (p-(var2/4096.0)) *6250.0/var1;
  var1 = d9*p*p/2147483648.0;
  var2 = p*d8/32768.0;
  p = p + (var1 + var2 + d7)/16.0;
  r = (int)p;
  return p;

}


int main(void) {
  int r = 0;
  uint32_t temp;
  uint32_t pressure;
  char buf[32], *s, *t;

  /* Set up peripheral bus clock */
  OSCCON &= ~0x180000;
  OSCCON |= 0x080000;
  
  /* Set up output pins */
  AD1PCFG = 0xFFFF;
  ODCE = 0x0;
  TRISECLR = 0xFF;
  PORTE = 0x0;
  
  /* Output pins for display signals */
  PORTF = 0xFFFF;
  PORTG = (1 << 9);
  ODCF = 0x0;
  ODCG = 0x0;
  TRISFCLR = 0x70;
  TRISGCLR = 0x200;
  
  /* Set up input pins */
  TRISDSET = (1 << 8);
  TRISFSET = (1 << 1);
  
  /* Set up SPI as master */
  SPI2CON = 0;
  SPI2BRG = 4;
  
  /* Clear SPIROV*/
  SPI2STATCLR &= ~0x40;
  /* Set CKP = 1, MSTEN = 1; */
        SPI2CON |= 0x60;
  
  /* Turn on SPI */
  SPI2CONSET = 0x8000;
  
  /* Set up i2c */
  I2C1CON = 0x0;
  /* I2C Baud rate should be less than 400 kHz, is generated by dividing
  the 40 MHz peripheral bus clock down */
  I2C1BRG = 0x0C2;
  I2C1STAT = 0x0;
  I2C1CONSET = 1 << 13; //SIDL = 1
  I2C1CONSET = 1 << 15; // ON = 1
  temp = I2C1RCV; //Clear receive buffer
  
  /* Set up input pins */
  TRISDSET = (1 << 8);
  TRISFSET = (1 << 1);
  /*set up timer*/

  T2CON = 0x70;
  TMR2 = 0;

  PR2 = (80000000/256)/10;


  TRISDSET = 1<<5;

  
  
  display_init();
  display_string(0, "");
  display_string(1, "");
  display_string(2, "");
  display_string(3, "");
  display_update();
  
  /* Send start condition and address of the temperature sensor with
  write mode (lowest bit = 0) until the temperature sensor sends
  acknowledge condition */

  /*start the timer*/
  
  do {
    i2c_start();
  } while(!i2c_send(TEMP_SENSOR_ADDR << 1));
  display_string(1,"lala");
  display_update();
  /* Send register number we want to access */
  //set register 0xf4 "ctrl_meas" to 
  // 0010 0011 = 0x23
  //001 001 11
  i2c_send(0xf4);

  i2c_send(0x27);
  /* Send stop condition */
  i2c_stop();



  //läs calibration parametrar
  do {
      i2c_start();
    } while(!i2c_send(TEMP_SENSOR_ADDR << 1));

  i2c_send(0x88);

  do {
      i2c_start();
    } while(!i2c_send((TEMP_SENSOR_ADDR << 1) | 1));
  dig_T1 = i2c_recv();//lsb
  i2c_ack();
  dig_T1 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_T2 = i2c_recv();//lsb
  i2c_ack();
  dig_T2 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_T3 = i2c_recv();//lsb
  i2c_ack();
  dig_T3 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P1 = i2c_recv();//lsb
  i2c_ack();
  dig_P1 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P2 = i2c_recv();//lsb
  i2c_ack();
  dig_P2 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P3 = i2c_recv();//lsb
  i2c_ack();
  dig_P3 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P4 = i2c_recv();//lsb
  i2c_ack();
  dig_P4 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P5 = i2c_recv();//lsb
  i2c_ack();
  dig_P5 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P6 = i2c_recv();//lsb
  i2c_ack();
  dig_P6 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P7 = i2c_recv();//lsb
  i2c_ack();
  dig_P7 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P8 = i2c_recv();//lsb
  i2c_ack();
  dig_P8 |=(i2c_recv()<<8);//msb
  i2c_ack();
  dig_P9 = i2c_recv();//lsb
  i2c_ack();
  dig_P9 |=(i2c_recv()<<8);//msb
  i2c_nack();
  i2c_stop();

  int how_much_time = 0;
  int counter = 0;
  int count_amount = 0;
  bool counting = false;
  bool counting_p = false;

  for(;;) {
    /* Send start condition and address of the temperature sensor with
    write flag (lowest bit = 0) until the temperature sensor sends
    acknowledge condition */
    r++;
    temp=0; 
    pressure = 0;  
    do {
      i2c_start();
    } while(!i2c_send(TEMP_SENSOR_ADDR << 1));
    
    /* Send register number we want to access */
    i2c_send(0xF7); //msb of temp

    
    /* Now send another start condition and address of the temperature sensor with
    read mode (lowest bit = 1) until the temperature sensor sends
    acknowledge condition */
    do {
      i2c_start();
    } while(!i2c_send((TEMP_SENSOR_ADDR << 1) | 1));
    
    /* Now we can start receiving data from the sensor data register */
    pressure = i2c_recv()<<12; //MSB pressure
    i2c_ack();
    pressure |= (i2c_recv()<< 4);
    i2c_ack();
    pressure |= (i2c_recv()>>4);
    i2c_ack();

    temp = i2c_recv()<<12; //MSB temp
    i2c_ack();
    temp |= (i2c_recv()<< 4);
    i2c_ack();
    temp |= (i2c_recv()>>4);


  /* To stop receiving, send nack and stop */
    i2c_nack();
    i2c_stop();

    //temp = bmp280_compensate_T_int32(temp);
    derece = bmp280_compensate_T_double(temp);
    //derece = (int) temp;
    
    //basinc = (int)pressure;
    basinc = bmp280_compensate_P_double(pressure);
    //basinc = 9999;
    



    
    
    if (getbtns() & 0b100)
    {
      how_much_time = (getsw()& 0b0111);//de sista 3  switches repr tiden för beräkna medelvärdet
      counting = true;
    }
    else if(getbtns() & 0b10){
      how_much_time = (getsw()& 0b0111);
      counting_p = true;
    }


    if (counting)
    {
      if (counter == how_much_time)
      {
        counting = false;
        counter = 0;
        count_amount = count_amount/how_much_time;
        sprintf(ortalamayazi,"A:%i",count_amount);
        //ortalamayazi = itoaconv(count_amount);
        count_amount = 0;

      }
      else{
        counter++;
        count_amount += derece;
      }
    }
    if (counting_p)
    {
      if (counter == how_much_time)
      {
        counting_p = false;
        counter = 0;
        count_amount = count_amount/how_much_time;
        sprintf(ortalamayazi,"A:%i",count_amount);
        //ortalamayazi = itoaconv(count_amount);
        count_amount = 0;

      }
      else{
        counter++;
        count_amount += basinc;
      }
    }
    sprintf(basincyazi,"P:%i",basinc);
    sprintf(dereceyazi,"T:%i",derece);
    //dereceyazi = itoaconv(derece);
    //basincyazi = itoaconv(basinc); 
    //display_string(1, dereceyazi);
    display_string(1, dereceyazi);

    display_string(2, basincyazi);
    display_string(3, ortalamayazi);
    display_update();
    delay_timer(1);
    
    if (r==30)
    { 

      display_string(3, "");
      display_update();
      r=0;
    }

    
    

  }
  
  return 0;
}

