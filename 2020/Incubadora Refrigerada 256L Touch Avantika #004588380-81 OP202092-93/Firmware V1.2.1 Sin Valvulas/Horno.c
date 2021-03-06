#include <18F4685.h>
#device adc=10
#device HIGH_INTS=TRUE //Activamos niveles de prioridad
#fuses HS,WDT8192,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,NOPUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR
#use delay(clock=20000000)
#use i2c(Master,slow,sda=PIN_C4,scl=PIN_C3)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N)
#include <DS1302.c>
#include <math.h>

// Definici?n de teclado - NO
#define   UP            !input(PIN_B0)
#define   DOWN          !input(PIN_B1)
#define   RIGHT         !input(PIN_B2)
#define   LEFT          !input(PIN_B3)

#define   DOOR          input(PIN_B7)
#define   TERM          input(PIN_B5)
#define   TERM2         input(PIN_B6)

#define Vref  0
#define Vdiv  1023
#define VCC   5.053
#define Rfijo 70.1

// Definici?n de Salidas
#define   Buzzer_on         output_bit(PIN_D7,1)
#define   Buzzer_off        output_bit(PIN_D7,0)
#define   SSR2_on           output_bit(PIN_E2,1)// SSR para calentamiento de Incubadora
#define   SSR2_off          output_bit(PIN_E2,0)// SSR para calentamiento de Incubadora
#define   SSR_on            output_bit(PIN_E1,1)// SSR para defrost
#define   SSR_off           output_bit(PIN_E1,0)// SSR para defrost
#define   Ventilador_on     output_bit(PIN_D2,0)
#define   Ventilador_off    output_bit(PIN_D2,1)
#define   Compresor_on      output_bit(PIN_D3,0)
#define   Compresor_off     output_bit(PIN_D3,1)


#define OFF 0
#define ON  1
#define RX_BUFFER_SIZE  10
char Rx_Buffer[RX_BUFFER_SIZE+1];
char Rx_Buffer2[RX_BUFFER_SIZE+1];
char RX_Wr_Index=0;


/* Tabla de Modulos, Primera Fila A?o Regular y el siguiente a?o Bisiesto
E   F   M   A   M   J   J   A   S   O   N   D
0   3   3   6   1   4   6   2   5   0   3   5
0   3   4   0   2   5   0   3   6   1   4   6
*/
int8 Regular[12]={0,3,3,6,1,4,6,2,5,0,3,5};
int8 Bisiesto[12]={0,3,4,0,2,5,0,3,6,1,4,6};

float promedio[90]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float X[2]={0.0,0.0};
short Desconectado=0,entro=OFF,GuardaEEPROM=OFF,Horas=OFF,Minutos=OFF,PruebaEstados=OFF,EstadoR1=OFF,EstadoR2=OFF,EstadoR3=OFF,EstadoR4=OFF,EstadoR5=OFF;
short Infinito=0,Finalizo=0,flag_clear=0,flagTemperatura=OFF,EstadoR6=OFF,EstadoR7=OFF,flagTDefrost=OFF,flagDefrost=OFF;
short flagControlT=OFF,flagControlH=OFF,rampa=OFF,flagRampa=OFF,flag_calcula=OFF,Humidificar=OFF,x1=ON,x10=OFF,x100=OFF;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=10,tiemporecta=0,flag_ganancia=0,Lectura2=0,Enfria=0,Controlando=0,Ajusto=10;
int8 Derivativo=40,Integral=0,z=0,Dato2=0,Inicio=0,TipoClave=0,Modulo=0,Opcion2=0,Segmentos=0,Segmento=0,Signo=10;
float mEstp=0,hEstp=0,mEstpAnt=0,hEstpAnt=0,mEst=0,hEst=0;
short Lectura=0,flag_ajuste=0,flag_ajuste2=0,EstadoBuzzer=OFF;
short flag_clear2=0,flag_clear3=0,flag_tiempocalentamiento=0,flag_setpoint=0;
int8 y=0,tiemporeset=0;
int8 Menu=0, i=0,Pausa=0,TiempoControl=0;
int8 Opcion=1,tiempo_esterilizacion=0,decimal=0,entero=0,Dato_Exitoso=0,MenuAntt=240;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,tinicio=0,tiempocalentamiento=0,tiemposetpoint=0,Tp=0,Tc=0,tDefrost=0,tCongelamiento=0,tApagadoCompresor=0,derivativo_ant=0;
float Potencia=0.0,Gananciamax=0.0;
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
float V0=0.0,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=2.0,GananciaR=2.0,desvio=0.5,Coeficiente=44.5,Pt=0.0,Humedad=0.0;
float Aumento=0.0,Temperatura=0.0,Saturacion=10.0,Setpoint=0,Temporal=0.0,SetpointH=0.0,TemperaturaEvaporador=0.0,Histeresis=0.0;
float a=0.0011,b=2.3302,c=-244.0723;
float TempeInicial=20,Tiempo=0,TiempoActual=0,PendienteActual=0,HorasActual=0,MinutosActual=0,TiempoRampa=0,Vfijo=0.0,Vfijotmp=0.0;
float PotenciaEntregada=0.0,Pendiente=0.0,DifPendiente=0.0;
float TemperaturaIdeal=0,penCamara=2.5,SetpointAnt=0.0;

int8 Year=18,Mes=9,Dia=13,Hora=0,Minuto=0,Segundo=0,dow=0;
int8 YearTx=0,MesTx=0,DiaTx=0,HoraTx=0,MinutoTx=0,dowTx=0,CicloLibre=0;

//Comunicacion//------------------------
byte SlaveA0Tx[0X20];
byte dato=0;            //Contendr? la informaci?n a enviar o recibir
byte direccion=0;       //Contendr? la direcci?n del esclavo en el bus I2C
byte posicion=0;        //Contendr? la posici?n en el buffer de memoria del esclavo donde se almacenar? o de donde se recuperar? el dato

typedef struct{
   int8 TemperaturaDec;
   int8 TemperaturaInt;
   int8 Signo;
   float Horas;
   float Minutos;
   float Temperatura;
}Personalizado;

typedef struct{
   float BN;
   float AN;
   float V0;
   float Pt;
   float RPT100;
   int8 l;
   int8 h;
   float Prom;
   float Temperature;
   int8 Ajuste;
   int8 Ajusten;
   float promedio[80];
   float Gain;
}SensorPT100;

SensorPT100 Sensor[2];
Personalizado rampas[20];

#include "Funciones.h"

#int_rda HIGH
void serial_rx_isr(){
   Dato2=fgetc();
   if(Dato2==0x65){//Inicio Comunicacion
      Inicio=1;
      RX_Wr_Index =0;
   }
   
   if(Inicio==1){
      Rx_Buffer2[RX_Wr_Index] = Dato2;
      RX_Wr_Index++;
   }
   if(RX_Wr_Index >= RX_BUFFER_SIZE){
      RX_Wr_Index =0;
      Inicio=0;
   }

   if(RX_Wr_Index==0){
      if(Rx_Buffer2[0]==0x65 && Rx_Buffer2[1]==0xff && Rx_Buffer2[2]==0xff && Rx_Buffer2[8]==0x00 && Rx_Buffer2[9]==0xff ){
         Dato_Exitoso=5;
      }else{
         Dato_Exitoso=10;
      }
   }
   //output_toggle(PIN_D7);
   //if(Menu==20)
     //Buzzer_on;
}

#int_TIMER1
void temp1s(void){
   set_timer1(40536);
   tiempos++;
   tiempos3++;
   tiempoctrl++;
   
   if(tiempos3>=200)
      Lectura2=1;
   
   if(Controlando==1){
      if(tiempoctrl<Tp){ // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
         SSR2_on;
      }else{
         SSR2_off;         
      }
      
      if(tiempoctrl>=Tc) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control      
         tiempoctrl=0;// tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
   }
    
   if(tiempos>=200){  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
      tiempos=0;Lectura=1;tiemporeset++;TiempoControl++;tiemporecta++;
   
      if(input_state(PIN_D3)){ //Salida Compresor
         if(tApagadoCompresor<250){
            tApagadoCompresor++;
         }
      }
      
      if(flagTDefrost)
         tCongelamiento++;
         
      if(flagDefrost)
         tDefrost++;
         
      if(flag_setpoint==1)
         tiemposetpoint++;
         
      if(tiempo_esterilizacion==1)
         tinicio++;
      
      if(flag_tiempocalentamiento==1)
         tiempocalentamiento++;
      
   }        
}

void main()
{
   ApagaSalidas();
   enable_interrupts(int_rda);
   enable_interrupts(global);
   //delay_ms(1000);   
   ConfiguraInterrupciones();
   LeeEEPROM();
   LimitaValores();
   rtc_init();
   delay_ms(1);
   Buzzer_on;
   delay_ms(500);
   Buzzer_off;
   printf("page Bienvenida");
   SendDataDisplay();
   printf("bkcmd=0");
   SendDataDisplay();
   delay_ms(3500); 
   printf("page Clave");
   SendDataDisplay();
   output_bit(PIN_A2,0);
   output_bit(PIN_A3,0);
   output_bit(PIN_A5,0);
   output_bit(PIN_E0,0);
   output_bit(PIN_C5,0);
   output_bit(PIN_D1,0);
   output_bit(PIN_D0,0);
   output_bit(PIN_C2,0);
   output_bit(PIN_C1,0);
   output_bit(PIN_C0,0);
      
   if(Setpoint<Temperatura){
      Enfria=1;
      Ganancia=GananciaR*(Setpoint/44.5);
      //Ganancia=4;
   }else{
      Enfria=0;
      Ganancia=GananciaR*(Setpoint/44.5);
      Gananciamax=(0.21)*Setpoint;
   }   
   
   if(SetpointH<Humedad){
      Humidificar=1;            
   }else{
      Humidificar=0;
   }   
   
   if((mEstp==0)&&(hEstp==0))
      Infinito=1;
   else
      Infinito=0;
   
   for(i=0;i<20;i++){
      if(rampas[i].Horas==0 && rampas[i].Minutos==0){
         Segmentos=i;  
         break;
      }else{
         Segmentos=i+1;  
      }
   }
   Sensor[0].Gain=105.22;
   Sensor[1].Gain=105.22;
   Vfijotmp=(VCC*Rfijo)/(6800+Rfijo);
   Vfijo=(Vfijotmp/VCC)*Vdiv;
   for(i=0;i<80;i++){
      Temperatura=LeerPT100(0,MediaMovil);
      TemperaturaEvaporador=LeerPT100(1,MediaMovil);
   }
   
   while(true)
   {
      Envio_Esclavos();
      rtc_get_date(Dia,Mes,Year,dow);
      rtc_get_time(Hora,Minuto,Segundo);
      LeeDisplay();
      restart_wdt();

      if(Lectura2==1){
         Temperatura=LeerPT100(0,MediaMovil);
         TemperaturaEvaporador=LeerPT100(1,MediaMovil);
         Lectura2=0;
      }
   
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==240){ //Menu de Bienvenida.
      printf("bkcmd=0");
      SendDataDisplay();
      delay_ms(1500);
      printf("page Clave");
      SendDataDisplay();
      ApagaSalidas();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==0){ //Menu de Contrase?a de acceso.
      //ApagaSalidas();
      entro=OFF;
      
      if(RX_Buffer[4]==0x11){//11
         if(TipoClave!=0){
            printf("page Principal");
            SendDataDisplay();
         }
      }
      
      if(TipoClave==3){
         printf("titulo.txt=\"Clave Nueva\"");
         SendDataDisplay();
         printf("doevents");
         SendDataDisplay();
      }else if(TipoClave==2){
         printf("titulo.txt=\"Clave Actual\"");
         SendDataDisplay();
         printf("doevents");
         SendDataDisplay();
      }else if(TipoClave==1 || TipoClave==4){
         printf("titulo.txt=\"Clave Tecnico\"");
         SendDataDisplay();
         printf("doevents");
         SendDataDisplay();
      }else if(TipoClave==0){
         printf("titulo.txt=\"Ingrese Clave\"");
         SendDataDisplay();
      }
      
      if(RX_Buffer[3]==0x0f){//0f, recibe caracteres ingresados desde el Display
         if(TipoClave==0){
            if(RX_Buffer[4]==0x33&&RX_Buffer[5]==0x38&&RX_Buffer[6]==0x39&&RX_Buffer[7]==0x32){ // Si Ingresa clave para reset general del sistema.
               write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contrase?a de Fabrica y reinicia Programa.
               write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
               reset_cpu();
            }
            if(RX_Buffer[4]>=0x30 && RX_Buffer[5]>=0x30 && RX_Buffer[6]>=0x30 && RX_Buffer[7]>=0x30
            && RX_Buffer[4]<=0x39 && RX_Buffer[5]<=0x39 && RX_Buffer[6]<=0x39 && RX_Buffer[7]<=0x39){
            if((RX_Buffer[4]==Password[0]+0x30)&&(RX_Buffer[5]==Password[1]+0x30)&&(RX_Buffer[6]==Password[2]+0x30)&&(RX_Buffer[7]==Password[3]+0x30)){
                  printf("page ClaveCorrecta");
                  SendDataDisplay();
                  //RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
               }else{
                  printf("page ClaveBad");
                  SendDataDisplay();
                  //RX_Buffer[3]=0x00;RX_Buffer2[3]=0x00;  
               } 
            }
         }else if(TipoClave==1){
            if(RX_Buffer[4]==0x34&&RX_Buffer[5]==0x34&&RX_Buffer[6]==0x34&&RX_Buffer[7]==0x34){ // Si Ingresa clave de Servicio Tecnico
               printf("page Config");
               SendDataDisplay();
            }else{
               printf("page Principal");
               SendDataDisplay();
            }
         }else if(TipoClave==2){
            if(RX_Buffer[4]>=0x30 && RX_Buffer[5]>=0x30 && RX_Buffer[6]>=0x30 && RX_Buffer[7]>=0x30
            && RX_Buffer[4]<=0x39 && RX_Buffer[5]<=0x39 && RX_Buffer[6]<=0x39 && RX_Buffer[7]<=0x39){
            if((RX_Buffer[4]==Password[0]+0x30)&&(RX_Buffer[5]==Password[1]+0x30)&&(RX_Buffer[6]==Password[2]+0x30)&&(RX_Buffer[7]==Password[3]+0x30)){
               printf("page Clave");
               SendDataDisplay();
               printf("titulo.txt=\"Clave Nueva\"");
               SendDataDisplay();
               TipoClave=3;
               RX_Buffer[3]=0x00;
               RX_Buffer2[3]=0x00;
            }else{
               printf("page Principal");
               SendDataDisplay();
               //RX_Buffer[3]=0x00;
               //RX_Buffer2[3]=0x00;
            } 
            }
         }else if(TipoClave==3){
            printf("titulo.txt=\"Clave Nueva\"");
            SendDataDisplay();
            printf("page Principal");
            SendDataDisplay();
            if(!GuardaEEPROM){
               write_eeprom(10,RX_Buffer[4]-0x30);delay_ms(20);write_eeprom(11,RX_Buffer[5]-0x30);delay_ms(20);
               write_eeprom(12,RX_Buffer[6]-0x30);delay_ms(20);write_eeprom(13,RX_Buffer[7]-0x30);delay_ms(20);
               GuardaEEPROM=ON;
            }
            Password[0]=RX_Buffer[4]-0x30;Password[1]=RX_Buffer[5]-0x30;Password[2]=RX_Buffer[6]-0x30;Password[3]=RX_Buffer[7]-0x30;
            //RX_Buffer[3]=0x00;
            //RX_Buffer2[3]=0x00;
         }else if(TipoClave==4){
            if(RX_Buffer[4]==0x34&&RX_Buffer[5]==0x34&&RX_Buffer[6]==0x34&&RX_Buffer[7]==0x34){ // Si Ingresa clave de Servicio Tecnico
               printf("page Test");
               SendDataDisplay();
            }else{
               printf("page Principal");
               SendDataDisplay();
            }
            //RX_Buffer[3]=0x00;
            //RX_Buffer2[3]=0x00;
         }
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==2){ //Menu de Tiempo de Duraci?n
         printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
         SendDataDisplay();
         
         if(!rampa){
            if(RX_Buffer[4]==0x0a){//Selecciono Minutos
               Minutos=OFF;
               Horas=ON;
            }
            
            if(RX_Buffer[4]==0x0b){//Selecciono Segundos
               Minutos=ON;
               Horas=OFF;
            }           
            
            if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
               delay_ms(200);
               if(RX_Buffer[4]==0x11 || LEFT){
                  Minutos=OFF;
                  Horas=OFF;
                  printf("page Principal");
                  SendDataDisplay();  
                  if(!GuardaEEPROM){
                     write_eeprom(0,mEstp);
                     delay_ms(10);
                     write_eeprom(1,hEstp);
                     delay_ms(10);
                     //mEst=mEstp;
                     //hEst=hEstp;
                     if((mEstp==0)&&(hEstp==0))
                        Infinito=1;
                     else
                        Infinito=0;
                     GuardaEEPROM=ON;
                  }
               }
            }
            
            if(RIGHT){
               delay_ms(200);
               if(RIGHT){
                  if(!Minutos){
                     Minutos=ON;
                     Horas=OFF;
                     printf("tsecest.pco=65535");
                     SendDataDisplay();  
                     printf("tminest.pco=0");
                     SendDataDisplay();  
                  }else if(!Horas){
                     Minutos=OFF;
                     Horas=ON;
                     printf("tsecest.pco=0");
                     SendDataDisplay();  
                     printf("tminest.pco=65535");
                     SendDataDisplay();  
                  }
               }
            }
            
            if(UP){
               delay_ms(200);
               if(UP){
                  if(Minutos){
                     mEstp+=1.0;
                  }
                  if(Horas){
                     hEstp+=1.0;
                  }    
               }
            }
            
            if(DOWN){
               delay_ms(200);
               if(DOWN){
                  if(Minutos && mEstp>0){
                     mEstp-=1.0;
                  }
                  if(Horas){
                     hEstp-=1.0;
                  }    
               }
            }
            
            if(RX_Buffer[4]==0x0c || RX_Buffer[4]==0x0d){//Tecla arriba Oprimida
               if(Minutos){
                  mEstp+=1.0;
               }
               if(Horas){
                  hEstp+=1.0;
               }  
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;
            }
            
            if(RX_Buffer[4]==0x0f || RX_Buffer[4]==0x0e){//Tecla abajo oprimida
               if(Minutos && mEstp>0){
                  mEstp-=1.0;
               }
               if(Horas){
                  hEstp-=1.0;
               }
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;
            }
            
            if(hEstp>99)hEstp=1;
            if(mEstp>59)mEstp=0;
               
            printf("tminsec.txt=\"%02.0f\"",hEstp);
            SendDataDisplay();
            printf("tsecsec.txt=\"%02.0f\"",mEstp);
            SendDataDisplay();
         }else{
            if(RX_Buffer[4]==0x0a){//Selecciono Minutos
               Minutos=OFF;
               Horas=ON;
            }
            
            if(RX_Buffer[4]==0x0b){//Selecciono Segundos
               Minutos=ON;
               Horas=OFF;
            }           
            
            if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
               delay_ms(200);
               if(RX_Buffer[4]==0x11 || LEFT){
                  Minutos=OFF;
                  Horas=OFF;
                  printf("page Rampas");
                  SendDataDisplay();  
                  if(!GuardaEEPROM){
                     write_eeprom(103+((CicloLibre-1)*5),(int8)rampas[CicloLibre-1].Horas);
                     delay_ms(10);
                     write_eeprom(104+((CicloLibre-1)*5),(int8)rampas[CicloLibre-1].Minutos);
                     delay_ms(10);
                     GuardaEEPROM=ON;
                  }
               }
            }
            
            if(RIGHT){
               delay_ms(200);
               if(RIGHT){
                  if(!Minutos){
                     Minutos=ON;
                     Horas=OFF;
                     printf("tsecest.pco=65535");
                     SendDataDisplay();  
                     printf("tminest.pco=0");
                     SendDataDisplay();  
                  }else if(!Horas){
                     Minutos=OFF;
                     Horas=ON;
                     printf("tsecest.pco=0");
                     SendDataDisplay();  
                     printf("tminest.pco=65535");
                     SendDataDisplay();  
                  }
               }
            }
            
            if(UP){
               delay_ms(200);
               if(UP){
                  if(Minutos){
                     rampas[CicloLibre-1].Minutos+=1.0;
                  }
                  if(Horas){
                     rampas[CicloLibre-1].Horas+=1.0;
                  }    
               }
            }
            
            if(DOWN){
               delay_ms(200);
               if(DOWN){
                  if(Minutos && rampas[CicloLibre-1].Minutos>0){
                     rampas[CicloLibre-1].Minutos-=1.0;
                  }
                  if(Horas && rampas[CicloLibre-1].Horas>0){
                     rampas[CicloLibre-1].Horas-=1.0;
                  }    
               }
            }
            
            if(RX_Buffer[4]==0x0c || RX_Buffer[4]==0x0d){//Tecla arriba Oprimida
               if(Minutos){
                  rampas[CicloLibre-1].Minutos+=1.0;
               }
               if(Horas){
                  rampas[CicloLibre-1].Horas+=1.0;
               }  
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;
            }
            
            if(RX_Buffer[4]==0x0f || RX_Buffer[4]==0x0e){//Tecla abajo oprimida
               if(Minutos && rampas[CicloLibre-1].Minutos>0){
                  rampas[CicloLibre-1].Minutos-=1.0;
               }
               if(Horas  && rampas[CicloLibre-1].Horas>0){
                  rampas[CicloLibre-1].Horas-=1.0;
               }
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;
            }
            
            if(rampas[CicloLibre-1].Horas>99)rampas[CicloLibre-1].Horas=1;
            if(rampas[CicloLibre-1].Minutos>59)rampas[CicloLibre-1].Minutos=0;
               
            printf("tminsec.txt=\"%02.0f\"",rampas[CicloLibre-1].Horas);
            SendDataDisplay();
            printf("tsecsec.txt=\"%02.0f\"",rampas[CicloLibre-1].Minutos);
            SendDataDisplay();
         }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
    if(Menu==3){ //Menu de Temperatura
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      
        if(Rx_Buffer[4]==0x20){//Selecciono x1
            x1=ON;
            x10=OFF;
            x100=OFF;
            printf("c0.val=1");
            SendDataDisplay();
            printf("c1.val=0");
            SendDataDisplay();
            printf("c2.val=0");
            SendDataDisplay();
            Rx_Buffer[4]=0x00;  
            Rx_Buffer2[4]=0x00;     
        }else if(Rx_Buffer[4]==0x21){//Selecciono x10
            x1=OFF;
            x10=ON;
            x100=OFF;
            printf("c0.val=0");
            SendDataDisplay();
            printf("c1.val=1");
            SendDataDisplay();
            printf("c2.val=0");
            SendDataDisplay();
            Rx_Buffer[4]=0x00;  
            Rx_Buffer2[4]=0x00;     
        }else if(Rx_Buffer[4]==0x22){//Selecciono x100
            x1=OFF;
            x10=OFF;
            x100=ON;
            printf("c0.val=0");
            SendDataDisplay();
            printf("c1.val=0");
            SendDataDisplay();
            printf("c2.val=1");
            SendDataDisplay();
            Rx_Buffer[4]=0x00;  
            Rx_Buffer2[4]=0x00;     
        }
      
      if(!rampa){
         if(Setpoint<-12.0)
            Setpoint=60.0;
         if(Setpoint>60.0)
            Setpoint=-12.0;
         
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            Horas=OFF;
            Minutos=OFF;
            printf("page Principal");
            SendDataDisplay();  
            if(!GuardaEEPROM){
               if(Setpoint<0){
                  write_eeprom(79,20);
                  delay_ms(10);
               }else{
                  write_eeprom(79,10);
                  delay_ms(10);
               }
               write_eeprom(80,(int8)abs(Setpoint));
               delay_ms(10);
               write_eeprom(81,extraeDecimal(abs(Setpoint)));
               delay_ms(10);
               GuardaEEPROM=ON;
            }
            if(Setpoint<Temperatura){
               Enfria=1;
               Ganancia=GananciaR*(Setpoint/44.5);
               //Ganancia=4;
            }else{
               Enfria=0;
               Ganancia=GananciaR*(Setpoint/44.5);
               Gananciamax=(0.21)*Setpoint;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            if(x1)
                Setpoint+=0.1;    
            else if(x10)    
                Setpoint+=1;    
            else if(x100)    
                Setpoint+=10; 
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            if(x1)
                Setpoint-=0.1;    
            else if(x10)    
                Setpoint-=1;    
            else if(x100)    
                Setpoint-=10;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;                        
         }
         printf("tsettem.txt=\"%2.1f\"",Setpoint);
         SendDataDisplay();
      }else{
         if(rampas[CicloLibre-1].Temperatura<-12.0)
            rampas[CicloLibre-1].Temperatura=60.0;
         if(rampas[CicloLibre-1].Temperatura>60.0)
            rampas[CicloLibre-1].Temperatura=-12.0;
         
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            Horas=OFF;
            Minutos=OFF;
            printf("page Rampas");
            SendDataDisplay();  
            if(!GuardaEEPROM){
               write_eeprom(100+((CicloLibre-1)*5),(int8)abs(rampas[CicloLibre-1].Temperatura));
               delay_ms(10);
               write_eeprom(101+((CicloLibre-1)*5),extraeDecimal(abs(rampas[CicloLibre-1].Temperatura)));
               delay_ms(10);
               if(rampas[CicloLibre-1].Temperatura<0){
                  write_eeprom(102+((CicloLibre-1)*5),20);
                  delay_ms(10);
               }else{
                  write_eeprom(102+((CicloLibre-1)*5),10);
                  delay_ms(10);
               }
               delay_ms(10);
               GuardaEEPROM=ON;
            }
         }
         
         if(RX_Buffer[4]==0x0d){//Tecla Arriba Oprimida
            if(x1)
                rampas[CicloLibre-1].Temperatura+=0.1;    
            else if(x10)    
                rampas[CicloLibre-1].Temperatura+=1;    
            else if(x100)    
                rampas[CicloLibre-1].Temperatura+=10;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
         }
         
         if(RX_Buffer[4]==0x0c){//Tecla Abajo Oprimida
            if(x1)
                rampas[CicloLibre-1].Temperatura-=0.1;    
            else if(x10)    
                rampas[CicloLibre-1].Temperatura-=1;    
            else if(x100)    
                rampas[CicloLibre-1].Temperatura-=10;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;                        
         }
         printf("tsettem.txt=\"%2.1f\"",rampas[CicloLibre-1].Temperatura);
         SendDataDisplay();
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       


//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==6){ // Menu de Prueba de Componentes
         PruebaEstados=ON;
         Controlando=0;
         if(RX_Buffer[4]==0x01){//Oprimio EV Suministro
            EstadoR1=!EstadoR1;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoR1){
            printf("b0.bco=2016");
            SendDataDisplay(); 
            if(tApagadoCompresor>=240){
               Compresor_on;
               tApagadoCompresor=0;
            }
         }else{
            printf("b0.bco=63488");
            SendDataDisplay(); 
            Compresor_off;
         }
         
         if(RX_Buffer[4]==0x02){//Oprimio EV Suministro
            EstadoR2=!EstadoR2;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoR2){
            printf("b1.bco=2016");
            SendDataDisplay(); 
            Ventilador_on;
         }else{
            printf("b1.bco=63488");
            SendDataDisplay(); 
            Ventilador_off;
         }
         
         if(RX_Buffer[4]==0x03){//Oprimio EV Suministro
            EstadoR3=!EstadoR3;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoR3){
            printf("b2.bco=2016");
            SendDataDisplay(); 
            SSR_on;
         }else{
            printf("b2.bco=63488");
            SendDataDisplay(); 
            SSR_off;
         }
         
         if(RX_Buffer[4]==0x04){//Oprimio EV Suministro
            EstadoR4=!EstadoR4;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoR4){
            printf("b3.bco=2016");
            SendDataDisplay(); 
            SSR2_on;
         }else{
            printf("b3.bco=63488");
            SendDataDisplay(); 
            SSR2_off;
         }
         
         if(RX_Buffer[4]==0x05){//Oprimio EV Suministro
            EstadoR5=!EstadoR5;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(RX_Buffer[4]==0x06){//Oprimio EV Suministro
            EstadoR6=!EstadoR6;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         
         if(RX_Buffer[4]==0x07){//Oprimio EV Suministro
            EstadoR7=!EstadoR7;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(DOOR){
            printf("b8.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b8.bco=63488");
            SendDataDisplay(); 
         }
         
         if(TERM){
            printf("b9.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b9.bco=63488");
            SendDataDisplay(); 
         }
         
         if(TERM2){
            printf("b10.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b10.bco=63488");
            SendDataDisplay(); 
         }
         
         /*
         if(PRES){
            printf("b11.bco=2016");
            SendDataDisplay(); 
         }else{
            printf("b11.bco=63488");
            SendDataDisplay(); 
         }
         */
         if(RX_Buffer[4]==0x13)//Oprimio Buzzer
         {
            EstadoBuzzer=!EstadoBuzzer;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;
         }
         
         if(EstadoBuzzer){
            printf("b12.bco=2016");
            SendDataDisplay(); 
            Buzzer_on;
         }
         else{
            printf("b12.bco=63488");
            SendDataDisplay(); 
            Buzzer_off;
         }
                  
         printf("b13.txt=\"ADC0:%3.0f\"",sensores(0));         
         SendDataDisplay();
         printf("b14.txt=\"ADC1:%3.0f\"",sensores(1));         
         SendDataDisplay();
         printf("b15.txt=\"A2:%3.0f\"",sensores(2));
         SendDataDisplay();
         printf("b18.txt=\"A3:%3.0f\"",sensores(3));
         SendDataDisplay();
         printf("b19.txt=\"T2:%3.1f\"",Temperatura);
         SendDataDisplay();
         printf("b20.txt=\"T1:%3.1f\"",TemperaturaEvaporador);
         SendDataDisplay();
         
         if(RX_Buffer[4]==0x21)//Oprimio salir
         {
            printf("page Principal");
            SendDataDisplay();
            PruebaEstados=OFF;
            ApagaSalidas();
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==7){ //Menu de Configuraci?n de Fecha y Hora
         if(RX_Buffer[4]==0x0a){//Selecciono Hora
            Opcion=4;
         }
         
         if(RX_Buffer[4]==0x0b){//Selecciono Minuto
            Opcion=5;
         }
         
         if(RX_Buffer[4]==0x0c){//Selecciono Dia
            Opcion=1;
         }
         
         if(RX_Buffer[4]==0x0d){//Selecciono Hora
            Opcion=2;
         }
         
         if(RX_Buffer[4]==0x0e){//Selecciono Year
            Opcion=3;
         }
         
         if(RIGHT){
            delay_ms(200);
            if(RIGHT){
               Opcion++;
            }
         }
         if(Opcion>5)
            Opcion=1;
         if(Opcion<1)
            Opcion=5;
            
         if(RX_Buffer[4]==0x11 || LEFT){//Selecciono Regresar
            printf("page Principal");
            SendDataDisplay();  
            
            
            if(esBisiesto(YearTx))
               Modulo=Bisiesto[MesTx];
            else
               Modulo=Regular[MesTx];
         
            dowTx=((YearTx-1)%7+((YearTx-1)/4-3*((YearTx-1)/100+1)/4)%7+Modulo+DiaTx%7)%7;
            rtc_set_datetime(DiaTx,MesTx,YearTx,dowTx,HoraTx,MinutoTx);
         }
         
         if((RX_Buffer[4]==0x2a)|| UP){//Tecla Arriba Oprimida
            delay_ms(200);
            if((RX_Buffer[4]==0x2a)|| UP){   
               if(Opcion==1){
                  if(MesTx==2){
                     if(esBisiesto(YearTx)){
                        if(DiaTx<29)
                           DiaTx++;
                        else
                           DiaTx=1;   
                     }else{
                        if(DiaTx<28)
                           DiaTx++;
                        else
                           DiaTx=1;   
                     }
                  }else{
                     if(MesTx<=7){
                        if(MesTx % 2 ==0){
                           if(DiaTx<30)
                              DiaTx++;    
                           else
                              DiaTx=1;   
                        }else{
                           if(DiaTx<31)
                              DiaTx++;    
                           else
                              DiaTx=1;   
                        }    
                     }else{
                        if(MesTx % 2 ==0){
                           if(DiaTx<31)
                              DiaTx++;  
                           else
                              DiaTx=1;
                        }else{
                           if(DiaTx<30)
                              DiaTx++;    
                           else
                              DiaTx=1;
                        }    
                     }
                  }
               }else if(Opcion==2){
                  if(MesTx<12)
                     MesTx++;
                  else
                     MesTx=1;
               }else if(Opcion==3){
                  if(YearTx<99)
                     YearTx++;
                  else 
                     YearTx=0;
               }else if(Opcion==4){
                  if(HoraTx<24)
                     HoraTx++;
                  else
                     HoraTx=0;
               }else if(Opcion==5){
                  if(MinutoTx<59)
                     MinutoTx++;
                  else
                     MinutoTx=0;
               }
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;
            }
         }
         
         if(RX_Buffer[4]==0x2b || DOWN){//Tecla Abajo Oprimida
            delay_ms(200);
            if(RX_Buffer[4]==0x2b || DOWN){   
               if(Opcion==1){
                  if(DiaTx>0)
                     DiaTx--;
               }else if(Opcion==2){
                  if(MesTx>1)
                     MesTx--;
                  else
                     MesTx=12;
               }else if(Opcion==3){
                  if(YearTx>0)
                     YearTx--;
                  else
                     YearTx=99;
               }else if(Opcion==4){
                  if(HoraTx>0)
                     HoraTx--;
                  else
                     HoraTx=23;
               }else if(Opcion==5){
                  if(MinutoTx>0)
                     MinutoTx--;
                  else
                     MinutoTx=59;
               }
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;
            }
         }
      
      printf("thora.txt=\"%02u\"",HoraTx);
      SendDataDisplay();
      printf("tminutos.txt=\"%02u\"",MinutoTx);
      SendDataDisplay();
      printf("tdia.txt=\"%02u\"",DiaTx);
      SendDataDisplay();
      printf("tmes.txt=\"%02u\"",MesTx);
      SendDataDisplay();
      printf("tyear.txt=\"%02u\"",YearTx);
      SendDataDisplay();
      LimitaDia();  
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
   if(Menu==8){ //Menu de Rampas
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();
      
      for(i=0;i<20;i++){
         if(rampas[i].Horas==0 && rampas[i].Minutos==0){
            Segmentos=i;  
            break;
         }else{
            Segmentos=i+1;  
         }
      }
      
      printf("t3.txt=\"Pasos Programados %02u\"",Segmentos);
      SendDataDisplay();
      GuardaEEPROM=OFF;
      
      if(RX_Buffer[4]>0x00 && RX_Buffer[4]<21){//Personalizados
         printf("ciclo.txt=\"Paso %02u\"",RX_Buffer[4]);
         SendDataDisplay();          
         printf("temp.txt=\"Temperatura: %2.1f?C\"",rampas[RX_Buffer[4]-1].Temperatura);
         SendDataDisplay();
         printf("tiempo.txt=\"Tiempo: %02.0f:%02.0f \"",rampas[RX_Buffer[4]-1].Horas,rampas[RX_Buffer[4]-1].Minutos);
         SendDataDisplay();
         CicloLibre=RX_Buffer[4];
      }else if(CicloLibre==0x00){
         printf("ciclo.txt=\"Paso --\"");
         SendDataDisplay();          
         printf("temp.txt=\"Temperatura: --\"");
         SendDataDisplay();
         printf("tiempo.txt=\"Tiempo: --:--\"");
         SendDataDisplay();
      }else if(CicloLibre>0 && CicloLibre<21){
         printf("ciclo.txt=\"Paso %02u\"",CicloLibre);
         SendDataDisplay();          
         printf("temp.txt=\"Temperatura: %2.1f?C\"",rampas[CicloLibre-1].Temperatura);
         SendDataDisplay();
         printf("tiempo.txt=\"Tiempo: %02.0f:%02.0f \"",rampas[CicloLibre-1].Horas,rampas[CicloLibre-1].Minutos);
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0x20 && CicloLibre>0){//20, Temperatura
         printf("page Temperatura");
         SendDataDisplay();     
         rampa=ON;
      }
      
      if(RX_Buffer[4]==0x40 && CicloLibre>0){//40, Tiempo
         printf("page Tiempo");
         SendDataDisplay();     
         rampa=ON;
      }
      
      if(RX_Buffer[4]==0x70){//70, Regresar
         printf("page Principal");
         SendDataDisplay();     
         rampa=OFF;
      }
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==15){//Menu de Clave Correcta
      if(!entro){
         delay_ms(2000);
         entro=ON;
      }
      printf("page Principal");
      SendDataDisplay();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==16){//Menu de Clave InCorrecta
      if(!entro){
         delay_ms(2000);
         entro=ON;
      }
      printf("page Clave");
      SendDataDisplay();
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------      

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==20){ //Menu de Funcionamiento
      GuardaEEPROM=OFF;
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u:%02u\"",Dia,Mes,Year,Hora,Minuto,Segundo);
      SendDataDisplay();

      if(RX_Buffer[4]==0x01){//Temperatura
         printf("page Temperatura");
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0x03){//Tiempo
         printf("page Tiempo");
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0x04){//Test de componentes
         printf("page Clave");
         SendDataDisplay();
         TipoClave=4;
      }
      
      if(RX_Buffer[4]==0x05){//Cambio Clave
         printf("page Clave");
         SendDataDisplay();
         TipoClave=2;
      }
      
      if(RX_Buffer[4]==0x06){//Fecha y Hora
         printf("page Fecha");
         SendDataDisplay();
         DiaTx=Dia;
         MesTx=Mes;
         YearTx=Year;
         HoraTx=Hora;
         MinutoTx=Minuto;
      }
      
      if(RX_Buffer[4]==0x07){//Configuracion Parametros
         printf("page Clave");
         SendDataDisplay();
         TipoClave=1;
      }
      
      if(RX_Buffer[4]==0x08){//Inicia Control Temperatura
         flagControlT=!flagControlT;
         RX_Buffer[4]=0x00;
         RX_Buffer2[4]=0x00;
      }
      
      if(RX_Buffer[4]==0x09){//Inicia Control Humedad
         flagControlH=!flagControlH;
         RX_Buffer[4]=0x00;
         RX_Buffer2[4]=0x00;
      }
      
      if(RX_Buffer[4]==0x0a){//Configuracion Parametros
         printf("page Rampas");
         SendDataDisplay();
      }
      
      if(RX_Buffer[4]==0x0b){//Inicia Control Rampa
         flagRampa=!flagRampa;
         if(flagRampa){
            // Configura parametros para inicio de rampa  
            SetpointAnt=Setpoint;            
            hEstpAnt=hEstp;
            mEstpAnt=mEstp;
            TempeInicial=Temperatura;
            Tiempo=(rampas[Segmento].Horas*3600)+(rampas[Segmento].Minutos*60);         
            Setpoint=rampas[Segmento].Temperatura;            
            Pendiente=((Setpoint-TempeInicial)/Tiempo);
            hEstp=rampas[Segmento].Horas;
            mEstp=rampas[Segmento].Minutos;
            flagControlT=ON;
         }else{
            Setpoint=SetpointAnt;
            //SetpointH=SetpointHAnt;
            hEstp=hEstpAnt;
            mEstp=mEstpAnt;
            mEst=0;
            hEst=0;
            //flagControlH=OFF;
            flagControlT=OFF;
         }
         RX_Buffer[4]=0x00;
         RX_Buffer2[4]=0x00;
      }
      
      if(Finalizo==0){
         if(TERM){
            Ventilador_off;
            Compresor_off;
            Controlando=0;
            flag_tiempocalentamiento=0;
            if(flag_clear3==0){
               tiempo_esterilizacion=0;
               SSR_off;
               SSR2_off;
               Tpulso=0.0;
               flag_clear3=1;
               flag_ganancia=0;
            }
            printf("status.txt=\"Estado:Alarma 01\"");
            SendDataDisplay();
            printf("statusT.txt=\"-----\"");
            SendDataDisplay();
            Ganancia=GananciaR*(Setpoint/44.5);
            Buzzer_on;
            delay_ms(1000);
            Buzzer_off;
            delay_ms(1000);
         }else{
            if(DOOR){
               Ventilador_off;
               Compresor_off;
               Controlando=0;
               flag_tiempocalentamiento=0;
               Ganancia=GananciaR*(Setpoint/44.5);
               tiempo_esterilizacion=0;
               SSR_off;
               SSR2_off;              
               Tpulso=0.0;
               flag_clear2=1;
               flag_ganancia=0;
               printf("status.txt=\"Estado:Puerta Abierta\"");
               SendDataDisplay();
               printf("statusT.txt=\"-----\"");
               SendDataDisplay();
            }else{
               if(flagDefrost){
                  printf("status.txt=\"Estado:Descongelando\"");
                  SendDataDisplay();
               }else{
                  printf("status.txt=\"Estado: ----\"");
                  SendDataDisplay();
               }
               if(flagRampa){
                  iniciaConteo(); 
                  printf("b4.pic=85");
                  SendDataDisplay();
                  printf("b6.pic=85");
                  SendDataDisplay();
                  controlaRampa();
                  printf("t8.txt=\"Paso %02u\"",Segmento+1);
                  //printf("t8.txt=\"%2.0f\"",Tiempo);
                  SendDataDisplay();
               }else{
                  printf("t8.txt=\"Info\"");
                  SendDataDisplay();
                  printf("b6.pic=84");
                  SendDataDisplay();
                  iniciaConteo();
                  
                  if(flagControlT){
                     printf("b4.pic=85");
                     SendDataDisplay();
                     controlaTemperatura();
                  }else{
                     printf("statusT.txt=\"Detenido\"");
                     SendDataDisplay();
                     printf("b4.pic=84");
                     SendDataDisplay();
                     SSR_off;
                     SSR2_off;               
                     Controlando=0;
                     Ventilador_off;
                     Compresor_off;
                     Tpulso=0.0;
                  }
               }
            }
         }
      }else{
         printf("t8.txt=\"Paso --\"");
         SendDataDisplay();
         printf("status.txt=\"Reinicie el equipo\"");
         SendDataDisplay();
         Compresor_off;
         Ventilador_off;
         SSR_off;         
         SSR2_off;
         Controlando=0;
         Tpulso=0.0;
         tiempo_esterilizacion=0;
         flagRampa=OFF;
         flagControlT=OFF;         
         ApagaSalidas();
         printf("b4.pic=84");
         SendDataDisplay();
         printf("b6.pic=84");
         SendDataDisplay();
         printf("statusT.txt=\"Finalizado\"");
         SendDataDisplay();
      }
      
      if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
      {                   
         Lectura=0;
         Potencia=(100.0/Tciclo)*Tpulso; 
 //----------------Muestra Temperatura--------------------------
         if(Pt>146.4){
            printf("temperatura.txt=\"T:----\"");
            SendDataDisplay();
            Desconectado=1;
         }else{
            if(Desconectado==1){
               Desconectado=0;
               delay_ms(2000);
               for(i=0;i<80;i++)
                  Temperatura=LeerPT100(0,80);
            }
            printf("temperatura.txt=\"T:%2.1f?C\"",Temperatura);
            SendDataDisplay();
         }
         //printf("statusH.txt=\"%2.1f?C %2.1f%% %2.1f\"",Tam,RH2,sensores(3));
         //SendDataDisplay();
         printf("stemperatura.txt=\"SET:%2.1f ?C\"",Setpoint);
         SendDataDisplay();
         printf("stiempo.txt=\"SET:%02.0f:%02.0f (HH:MM)\"",hEstp,mEstp);
         SendDataDisplay();
         printf("tiempo.txt=\"%02.0f:%02.0f\"",hEst,mEst);
         SendDataDisplay();
         printf("potT.txt=\"Pot. T:%2.0f %%\"",Potencia);
         //printf("potT.txt=\"Pot:%2.0f%% %2.1f %2.4f\"",Potencia,TemperaturaIdeal,Pendiente);
         SendDataDisplay(); 
      } 
      
      // Si cumplio el tiempo de esterilizaci?n, Muestre ciclo finalizado y detenga control de temperatura.
      if((mEstp>0)||(hEstp>0)){
         CuentaTiempo();  
         if(flagRampa){
            if((mEst>=mEstp)&&(hEst>=hEstp)&& Finalizo==0){
               flag_calcula=0;
               if(Segmento<Segmentos){
                  Segmento++;
                  hEst=0;
                  mEst=0;
               }else{
                  Finalizo=1;
               }
               for(i=0;i<2;i++){
                  Buzzer_on;
                  delay_ms(1000);
                  Buzzer_off;
                  delay_ms(1000);
               }
            }
         }else{
            if((mEst>=mEstp)&&(hEst>=hEstp)&& tiempo_esterilizacion==1){
               if(flag_clear==0){
                  tiempo_esterilizacion=0;
                  Pausa=2;
                  flag_clear=1;
                  SSR_off;
                  SSR2_off;
                  Controlando=0;
                  Tpulso=0.0;
                  flag_ganancia=0;
                  Finalizo=1;
               }
               printf("status.txt=\"Finalizado\"");
               SendDataDisplay();
               ApagaSalidas();
               for(i=0;i<2;i++){
                  Buzzer_on;
                  delay_ms(1000);
                  Buzzer_off;
                  delay_ms(1000);
               }
            }
         }
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuraci?n de Parametros de Control
     
      if(Opcion2>10)
         Opcion2=1;
      if(Opcion2<1)
         Opcion2=10;
      
      if(Opcion2==1){
         printf("Config.t2c.txt=\"Proporcional\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.0f\"",Ganancia);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               Ganancia+=1.0;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Ganancia-=1.0;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
         
         if(Ganancia>40)
            Ganancia=1;
         if(Ganancia<1)
            Ganancia=40;
      }else if(Opcion2==2){
         printf("Config.t2c.txt=\"Derivativo\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Derivativo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               Derivativo++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;                           
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Derivativo--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;                           
            }
         }
         
         if(Derivativo>40)
            Derivativo=1;
         if(Derivativo<1)
            Derivativo=40;
      }else if(Opcion2==3){
         printf("Config.t2c.txt=\"Integral\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Integral);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               Integral++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Integral--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;                           
            }
         }
         
         if(Integral>4)
            Integral=1;
         if(Integral<1)
            Integral=4;
      }else if(Opcion2==4){
         printf("Config.t2c.txt=\"Tciclo\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%2.0f\"",Tciclo);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               Tciclo+=1.0;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               Tciclo-=1.0;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
         
         if(Tciclo>20)
            Tciclo=5;
         if(Tciclo<5)
            Tciclo=20;
      }else if(Opcion2==5){
         printf("Config.t2c.txt=\"Ajuste +\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[0].Ajuste);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Sensor[0].Ajuste<40)
                  Sensor[0].Ajuste++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Sensor[0].Ajuste>0)
                  Sensor[0].Ajuste--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
      }else if(Opcion2==6){
         printf("Config.t2c.txt=\"Ajuste -\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[0].Ajusten);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Sensor[0].Ajusten<40)
                  Sensor[0].Ajusten++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }         
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Sensor[0].Ajusten>0)
                  Sensor[0].Ajusten--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
      }else if(Opcion2==7){
         printf("Config.t2c.txt=\"Ajuste2 +\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[1].Ajuste);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Sensor[1].Ajuste<40)
                  Sensor[1].Ajuste++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Sensor[1].Ajuste>0)
                  Sensor[1].Ajuste--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }           
         }
      }else if(Opcion2==8){
         printf("Config.t2c.txt=\"Ajuste2 -\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",Sensor[1].Ajusten);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(Sensor[1].Ajusten<40)
                  Sensor[1].Ajusten++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(Sensor[1].Ajusten>0)
                  Sensor[1].Ajusten--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }
         }
      }else if(Opcion2==9){
         printf("Config.t2c.txt=\"Media\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%u\"",MediaMovil);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a || UP){//Tecla Arriba Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0a || UP){
               if(MediaMovil<80)
                  MediaMovil++;
               RX_Buffer[4]=0x00;  
               RX_Buffer2[4]=0x00;            
            }
         }
            
         if(RX_Buffer[4]==0x0b || DOWN){//Tecla Abajo Oprimida
            delay_ms(20);
            if(RX_Buffer[4]==0x0b || DOWN){
               if(MediaMovil>10)
                  MediaMovil--;
               RX_Buffer[4]=0x00;
               RX_Buffer2[4]=0x00;            
            }            
         }
      }else if(Opcion2==10){
         printf("Config.t2c.txt=\"Histeresis\"");
         SendDataDisplay();      
         printf("t3c.txt=\"%1.1f\"",Histeresis);
         SendDataDisplay();
         if(RX_Buffer[4]==0x0a){//Tecla Arriba Oprimida
            if(Histeresis<5.0)
               Histeresis+=0.1;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;                        
         }
            
         if(RX_Buffer[4]==0x0b){//Tecla Arriba Oprimida
            if(Histeresis>0.0)
               Histeresis-=0.1;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;                        
         }
      }
      
      if(RX_Buffer[4]==0x0c || LEFT){//Tecla Izquierda Oprimida
         delay_ms(20);
         if(RX_Buffer[4]==0x0c || LEFT){
            Opcion2++;
            RX_Buffer[4]=0x00;  
            RX_Buffer2[4]=0x00;            
         }
      }
         
      if(RX_Buffer[4]==0x0d || RIGHT){//Tecla Derecha Oprimida
         delay_ms(20);
         if(RX_Buffer[4]==0x0d || RIGHT){
            Opcion2--;
            RX_Buffer[4]=0x00;
            RX_Buffer2[4]=0x00;            
         }
      }
      
      if(RX_Buffer[4]==0x11 || (RIGHT && LEFT)){//Tecla Regresar
         delay_ms(20);
         if(RX_Buffer[4]==0x11 || (RIGHT && LEFT)){
            if(!GuardaEEPROM){
               write_eeprom(21,Sensor[0].Ajuste);
               delay_ms(10);
               write_eeprom(22,Sensor[0].Ajusten);
               delay_ms(10);
               write_eeprom(23,Sensor[1].Ajuste);
               delay_ms(10);
               write_eeprom(24,Sensor[1].Ajusten);
               delay_ms(10);
               write_eeprom(32,(int8)Integral);
               delay_ms(10);
               write_eeprom(31,(int8)Derivativo);
               delay_ms(10);
               write_eeprom(30,(int8)Ganancia);
               delay_ms(10);
               write_eeprom(34,(int8)Histeresis*10);
               delay_ms(10);
               write_eeprom(36,(int8)MediaMovil);
               delay_ms(10);
               write_eeprom(33,(int8)Tciclo);
               delay_ms(10);
               GuardaEEPROM=ON;
               Buzzer_on;
               delay_ms(1000);
               Buzzer_off;
            }
            printf("page Principal");
            SendDataDisplay();
         }
      }
      printf("fecha.txt=\"Fecha:%02u/%02u/20%02u    Hora: %02u:%02u\"",Dia,Mes,Year,Hora,Minuto);
      SendDataDisplay(); 
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       
      
   }
}
