// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci�n para Autoclave con Calderin
// Tiene Men�:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Pi�eros.
// JP Inglobal. 2015

#include <18F2550.h>
#device adc=10
#fuses HSPLL,WDT1024,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
//#use RS232(BAUD=19200, BITS=8, PARITY=N, XMIT=PIN_C6, RCV=PIN_C7, stream=printer)
#use i2c(slave, slow, sda=PIN_B0, scl=PIN_B1, address=0xA0)

#define USB_HID_DEVICE FALSE // deshabilitamos el uso de las directivas HID
#define USB_EP1_TX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for IN bulk/interrupt transfers
#define USB_EP1_RX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for OUT bulk/interrupt transfers
#define USB_EP1_TX_SIZE 32 // size to allocate for the tx endpoint 1 buffer
#define USB_EP1_RX_SIZE 32 // size to allocate for the rx endpoint 1 buffer
#include <pic18_usb.h> // Microchip PIC18Fxx5x Hardware layer for CCS's PIC USB driver
#include "header.h" // Configuraci�n del USB y los descriptores para este dispositivo
#include <usb.c> // handles usb setup tokens and get descriptor reports
const int8 Lenbuf = 32; 
int8 recbuf[Lenbuf];
int8 rxubuf[Lenbuf];

// Variables para Pt100
float d3=0.0;
float Temperatura=0.0,TempUSB=0.0,Temporal=0.0;

byte fstate;                     //Guardara el estado del bus I2C
byte posicion, buffer[0x20], txbuf[0x20],rxbuf[0x20];     //Buffer de memoria

#INT_SSP
void ssp_interupt (){
   int incoming;                //Variable donde se recibe el byte que manda el maestro
   
   fstate = i2c_isr_state();    //Lectura del estado del bus I2c la interrupci�n
   /* Solicitud de lectura del esclavo por el master */
   if(fstate == 0x80) {         
       //Manda al maestro la informaci�n contenida en la posici�n de memoria que le ha solicitado
      i2c_write (txbuf[posicion]);
   }else{                              //Sino es que hay dato en el bus I2C...
      incoming = i2c_read();           //... lo lee
      if (fState == 1) {          //Informaci�n recibida corresponde a la posicion
         posicion = incoming;          //Se guarda posici�n
      }else if (fState == 2) {          //Informaci�n recibida corresponde al dato
         rxbuf[posicion] = incoming;
      }
   }
}

void Lee_Vector(void){
   if(rxbuf[2]==255 && rxbuf[3]==255 && rxbuf[4]==128){
      Temporal=rxbuf[1];
      Temperatura=rxbuf[0];
      Temperatura=Temperatura+(Temporal/10);
   }
}


void main(){
   fState = 0;
   for (posicion=0;posicion<0x10;posicion++){
      buffer[posicion] = 0x00;
      txbuf[posicion] = 0x00;
      rxbuf[posicion] = 0x00;
   }
   
   Lee_Vector();
   enable_interrupts(INT_SSP);
   usb_init();
   usb_task();
   setup_wdt(WDT_ON);
   enable_interrupts(global);

while(true){
   Lee_Vector();
   restart_wdt();
   
//_Menu_De_USB--------------------------------------------------------------------------------------------------------------------------------------------    
if(usb_enumerated()){
   
   if(usb_kbhit(1)){
      usb_get_packet(1, rxubuf, Lenbuf);
   }
   
   TempUSB=Temperatura;
   recbuf[0]=(int8)(TempUSB);
   recbuf[1]=120;
   d3=TempUSB-(int8)TempUSB;
   d3=d3*10.0;
   recbuf[2]=(int8)d3;
   recbuf[3]=0;
   recbuf[4]=0;
   recbuf[5]=0;    
   recbuf[6]=0;
   recbuf[7]=0;
   recbuf[8]=0;
   recbuf[9]=0;
   recbuf[10]=0;
   recbuf[11]=0;     
   recbuf[13]=0;
   recbuf[14]=0;
                
    usb_put_packet(1, recbuf, Lenbuf, USB_DTS_TOGGLE); //y enviamos el mismo paquete de tama�o 32bytes del EP1 al PC
}
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    
   }
}
