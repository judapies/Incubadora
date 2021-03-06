/// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci?n para Autoclave 80Lts.
// Tiene Men?:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Pi?eros.
// Ing. Faiver Humberto Trujillo.
// JP Inglobal. 2014

#include <18F4550.h>
#device adc=10
#fuses XTPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,NOVREGEN,NOMCLR
#use delay(clock=48000000)
#include "HDM64GS12.c"
#include "GRAPHICS.c"
#include "imagen.h"

// Definici?n de teclado - NO
#define   ARRIBA        input(PIN_A5)
#define   ABAJO         input(PIN_E0)
#define   DERECHA       input(PIN_E1)
#define   IZQUIERDA     input(PIN_E2)

#define   DOOR          input(PIN_A4)
#define   TERM          input(PIN_A3)


// Definici?n de Salidas
#define   Rele1_on          output_bit(PIN_C0,1)
#define   Rele1_off         output_bit(PIN_C0,0)
//#define   Vent_on           output_bit(PIN_C1,1)
//#define   Vent_off          output_bit(PIN_C1,0)
#define   SSR_on            output_bit(PIN_C2,1)
#define   SSR_off           output_bit(PIN_C2,0)
#define   Buzzer_on         output_bit(PIN_B7,1)
#define   Buzzer_off        output_bit(PIN_B7,0)
#define   Rele2_on          output_bit(PIN_C6,1)
#define   Rele2_off         output_bit(PIN_C6,0)
#define   Rele3_on          output_bit(PIN_C7,1)
#define   Rele3_off         output_bit(PIN_C7,0)

float promedio[90]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float X[2]={0.0,0.0};
short UP=0,DOWN=0,RIGHT=0,LEFT=0,flagup=0,flagdn=0,flagrt=0,flaglt=0,Desconectado=0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=10,tiemporecta=0,flag_ganancia=0,saturado=0,Velocidad=3,Lectura2=0,Enfria=0;
short Flanco=0,Flanco1=0,Flanco2=0,visual=1,Lectura=0;
short flag_clear=0,flag_clear2=0,flag_clear3=0,flag_clear4=0,flag_tiempocalentamiento=0,flag_display=0,flag_setpoint=0,flag_control=0;
int8 y=0,tiemporeset=0;
int8 Menu=0, unidad=1,i=0,Pausa=0,Pausa_ant=0,TiempoControl=0;
int8 Opcion=1,tiempo_esterilizacion=0,tiempo_secado=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,tinicio=0,tiempocalentamiento=0,tiemposetpoint=0,Setpoint=0,Temperatura1=0,Temperatura2=0,Temperatura3=0;
float Potencia=0.0,Tpulsomin=0.0,Gananciamax=0.0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Contrasena[4]={0,0,0,0};      // Contrasena de 4 digitos
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
float V0=0.0,R1=10000.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=25.0,Ganancia2=20.0,desvio=1.0,Coeficiente=38.0;
float Aumento=0.0,Temperatura=0.0,Saturacion=10.0,Tpulsomax=0.0;
float a=0.0011,b=2.3302,c=-244.0723;

typedef struct{
   char Letra[30];
}MENUU;

MENUU Menus;

void ActualizaRecta(){
   X[y]=Temperatura;
   y++;
   if(y>=2)
   {
      y=0;
      Aumento=X[1]-X[0];
   }
}

void Velocidades(int x){

if(x==0)
{
   set_pwm2_duty(0);
}

if(x==1)
{
   set_pwm2_duty(128);
}

if(x==2)
{
   set_pwm2_duty(180);
}

if(x==3)
{
   set_pwm2_duty(255);
}

}

void Teclado(void){

   if(ARRIBA)
   {
      if(!flagup)
      {
         UP=1;
         flagup=1;
      }
   }
   
   if(!ARRIBA)
   {
      if(flagup)
      {
         UP=1;
         flagup=0;
      }
   }
   
   if(ABAJO)
   {
      if(!flagdn)
      {
         DOWN=1;
         flagdn=1;
      }
   }
   
   if(!ABAJO)
   {
      if(flagdn)
      {
         DOWN=1;
         flagdn=0;
      }
   }
   
   if(DERECHA)
   {
      if(!flagrt)
      {
         RIGHT=1;
         flagrt=1;
      }
   }
   
   if(!DERECHA)
   {
      if(flagrt)
      {
         RIGHT=1;
         flagrt=0;
      }
   }
   
   if(IZQUIERDA)
   {
      if(!flaglt)
      {
         LEFT=1;
         flaglt=1;
      }
   }
   
   if(!IZQUIERDA)
   {
      if(flaglt)
      {
         LEFT=1;
         flaglt=0;
      }
   }
}

// Funcion para conversor analogo-digital
float sensores(int x){
float y;set_adc_channel(x);delay_ms(1);y=read_adc();return (y);
}
float Leer_Sensor(int media)
{
   Prom=0.0;
   V0 = sensores(0);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
   V0 = V0/40.6; // Se elimina la ganancia
   Pt=(R1)/((1023.0/V0)-1.0); // Se calcula el valor de la PT100
   //Pt=R1/((1023.0/V0)-1.0); // Se calcula el valor de la PT100
   Pt=Pt-0.3;
   
            
   if(l>(media-1))
   {l=0;}
   //promedio[l]=Temperature;l++;
   promedio[l]=Pt;l++;
         
   for(h=0;h<media;h++)
   {
      Prom+=promedio[h];
   } 
   Prom=Prom/media;   
   //Temperature=(Prom-100.0)/(0.385);  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.  
   Temperature=((Prom*Prom)*a)+(Prom*b)+c;  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.
   
   Temperature+=((float)Ajuste-(float)Ajusten);
   
   //return Prom;
   return Temperature;
}

void Controla(){
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   if(tiemporecta>=40)
   {
      ActualizaRecta();
      tiemporecta=0;
   } 
  
   if(Enfria==1)
   {
      error=Setpoint-desvio-Temperatura;      // C?lculo del error ******
      
      if(error>-1.0 && Aumento>-0.2)
      {
         if(Potencia<90.0)
         {
            if(TiempoControl>=80)
            {
               TiempoControl=0;
               Ganancia+=0.5;
            }
         }
      }
   
      if(error>0.0)     // Anti Wind_DOWN
      {
         error=0.0;
      }
      error=error*-1.0;
      
      if(Ganancia<0.5)
      {
         Ganancia=0.5;
      }
      if(Ganancia>Gananciamax)
      {
         Ganancia=Gananciamax;
      }
   
      tmp=error*Ganancia;  // Control Proporcional.
            
      if(tmp>Setpoint)   // Anti Wind-UP      
      {
         tmp=Setpoint;
      }
                  
      if(tmp<0.0)     // Anti Wind_DOWN    
      {
         tmp=0.0;
      }
      
      // Tpulso(t)= Tciclo*(y(t)-ymin)/(ymax - ymin); calculo de ciclo util para control de resistencia.ymax=140.ymin=0;
      // Tpulso es float, hay que pasar ese ciclo a una salida de un puerto usando el TMR1.
      Tpulso=(tmp/Setpoint)*Tciclo;  
      
      if(tiempoctrl>=(int16)(Tciclo/0.005)) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
      {
         tiempoctrl=0;
      } // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
      
      if(tiempoctrl<(int16)(Tpulso/0.005)) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
      {
         SSR_on;
         //Peltiern_off;
         //Peltierp_on;
      }
      else         
      {
         SSR_off;
         //Peltiern_off;
         //Peltierp_off;
      }
            
   }
   else
   {
      error=desvio+Setpoint-Temperatura;      // C?lculo del error ******
   
      if(error>1.2 && error<30 && Aumento<0.3 && flag_setpoint==0)
      {
         if(Potencia<90.0)
         {
            if(TiempoControl>=30)
            {
               TiempoControl=0;
               if(Aumento<-0.5)
               {
                  Ganancia+=2;
               }
               else
               {
                  if(Aumento<-0.1)
                  {
                     Ganancia+=1;
                  }   
                  else
                  {
                     Ganancia+=0.5;
                  }
               }
            }
         }
      }
      
      if(error<-0.7)
      {
         if(TiempoControl>=40)
         {
            TiempoControl=0;
            Ganancia-=1;
         }
      }
      
      if(Ganancia<0.5)
      {
         Ganancia=0.5;
      }
      if(Ganancia>Gananciamax)
      {
         Ganancia=Gananciamax;
      }
   
      if(error<0.0)     // Anti Wind_DOWN    
      {
         error=0.0;
      }
      //tmp=(Setpoint-Prom)*Ganancia;  // Control Proporcional.
      tmp=error*Ganancia;  // Control Proporcional.
            
      if(tmp>Setpoint)   // Anti Wind-UP      
      {
         tmp=Setpoint;
      }
                  
      if(tmp<0.0)     // Anti Wind_DOWN    
      {
         tmp=0.0;
      }
   
      Tpulso=(tmp/Setpoint)*Tciclo;
      
                                         
      if(tiempoctrl<(int16)(Tpulso/0.005)) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
      {
         SSR_on;
      }
      else         
      {
         SSR_off;
      }
      
      if(tiempoctrl>=(int16)(Tciclo/0.005)) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
      {
         tiempoctrl=0;
      } // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
            
   }
   
//--------------------------------------------------------------------------------------------------------------------//   
  
}

void displayMenu(char palabra[30], int x, int y, int sombreado, int tamano)
{
   strcpy(Menus.Letra,palabra);
   if(sombreado==2)
      {glcd_text57(x+1, y+1, Menus.Letra, tamano, ON);}
   if(sombreado==1)
      {glcd_rect(0, y, 127, y+(tamano*8), YES, ON);glcd_text57(x+1, y+1, Menus.Letra, tamano, OFF);}
   if(sombreado==0)
      {glcd_rect(0, y, 127, y+(tamano*8), YES, OFF);glcd_text57(x+1, y+1, Menus.Letra, tamano, ON);}
   glcd_update();
}

void displayTemps(char palabra[30], int16 digito, int x, int y, int sombreado, int tamano)
{
   char voltage[9];
   
   if(sombreado==1)
   {
      sprintf(voltage, "%03Lu", digito);
      glcd_rect(0, y-2, 127, y+(tamano*8), YES, ON);
      strcpy(Menus.Letra,palabra);
      glcd_text57(x+90, y+1, voltage, tamano, OFF);
      glcd_text57(x+1, y+1, Menus.Letra, tamano, OFF);
      strcpy(Menus.Letra,"C");
      glcd_text57(x+120, y+1, Menus.Letra, tamano, OFF);
      glcd_circle(x+115,y-1,1,NO,OFF);
   }
   if(sombreado==0)
   {
      sprintf(voltage, "%03Lu", digito);
      glcd_rect(0, y-2, 127, y+(tamano*8), YES, OFF);
      strcpy(Menus.Letra,palabra);
      glcd_text57(x+90, y+1, voltage, tamano, ON);
      glcd_text57(x+1, y+1, Menus.Letra, tamano, ON);
      strcpy(Menus.Letra,"C");
      glcd_text57(x+120, y+1, Menus.Letra, tamano, ON);
      glcd_circle(x+115,y-1,1,NO,ON);
   }
   glcd_update();
}

void displayTiempo(int digito, int x, int y, int sombreado, int tamano)
{
   char voltage[9];
   
   if(sombreado==2)
   {
      sprintf(voltage, "%03u", digito);
      glcd_rect(x, y, x+(tamano*18), y+(tamano*8), YES, ON);
      glcd_text57(x+1, y+1, voltage, tamano, OFF);
   }
   else
   {
      if(sombreado==3)
      {
         sprintf(voltage, "%03u", digito);
         glcd_rect(x, y, x+(tamano*18), y+(tamano*8), YES, OFF);
         glcd_text57(x+1, y+1, voltage, tamano, ON);
      }
      sprintf(voltage, "%u", digito);
      if(sombreado==1)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, ON);glcd_text57(x+1, y+1, voltage, tamano, OFF);}
      if(sombreado==0)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, OFF);glcd_text57(x+1, y+1, voltage, tamano, ON);}
   }
   glcd_update();
}

void displayTemperatura(int16 digito, int x, int y, int sombreado, int tamano)
{
   char voltage[9];
   
   if(sombreado==2)
   {
      sprintf(voltage, "%03Lu", digito);
      glcd_rect(x, y, x+(tamano*18), y+(tamano*8), YES, ON);
      glcd_text57(x+1, y+1, voltage, tamano, OFF);
   }
   else
   {
      sprintf(voltage, "%03Lu", digito);
   
      if(sombreado==4)
      {
         glcd_rect(x, y, x+(tamano*16), y+(tamano*8), YES, OFF);
         glcd_text57(x+1, y+1, voltage, tamano, ON);
      }
      if(sombreado==1)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, ON);glcd_text57(x+1, y+1, voltage, tamano, OFF);}
      if(sombreado==0)
         {glcd_rect(x, y, x+(tamano*5), y+(tamano*8), YES, OFF);glcd_text57(x+1, y+1, voltage, tamano, ON);}
   }
   glcd_update();
}

void glcd_imagen(int8 x)
{
   char i,j;
   signed char k; 
   
   if(x==5){
   for( i = 0 ; i < 64 ; i ++ )
   {  
      for( j = 0 ; j < 16 ; j ++)
      {    
         for(k=7;k>-1;k--)
         {      
            if( bit_test(Horno[i][j] ,7-k )) 
               glcd_pixel( j*8+k,i, ON );
         }  
      }
   }
   }
   
}

#int_TIMER1
void temp1s(void){
   //set_timer1(45612);  // 5 ms 5536 , Posiblemente cargar con 22144 debido a F=12MHz
   set_timer1(5536);
   tiempos++;
   tiempos3++;
   tiempoctrl++;
   
   if(tiempos3>=200)
   {
      Lectura2=1;
   }
    
   if(tiempos>=200)  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
   {tiempos=0;Lectura=1;tiemporeset++;TiempoControl++;tiemporecta++;
   
      if(flag_setpoint==1)
         tiemposetpoint++;
         
      if(tiempo_esterilizacion==1)
      {               
         tinicio++;
      }
      
      if(flag_tiempocalentamiento==1)
      {
         tiempocalentamiento++;
      }
   }        
}
   
void Limpia(int x1,int x2,int y){
   glcd_rect(x1, y, x2, y+7, YES, OFF);
}

void displayT(float adc,int x,int y,int w) {
   char voltage[9];    
   sprintf(voltage, "%03.1f",adc); // Converts adc to text
   glcd_rect(x, y, x+(w*27), y+(w*8), YES, OFF);
   glcd_text57(x, y, voltage, w, ON);
}

void displayContrasena(int digito, int x, int y, int sombreado)
{
   char voltage[9];
   sprintf(voltage, "%i", digito);
   
   if(sombreado==1)
      {glcd_rect(x, y, x+18, y+24, YES, ON);glcd_text57(x+2, y+1, voltage, 2, OFF);}
   else   
      {glcd_rect(x, y, x+18, y+24, NO, OFF);glcd_text57(x+2, y+1, voltage, 2, ON);}

}


void main()
{
   delay_ms(1000);   
   Buzzer_off;
   Velocidades(0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN1);
   setup_adc(ADC_CLOCK_DIV_64);
   setup_ccp2 (CCP_PWM);
   setup_timer_2(T2_DIV_BY_16, 255, 1);   
   set_pwm2_duty(0);
   enable_interrupts(global);
   Testerilizacionp[0]=read_eeprom(0);
   delay_ms(50);   
   Testerilizacionp[1]=read_eeprom(1);
   delay_ms(50);   
   Testerilizacionp[2]=read_eeprom(2);
   delay_ms(50);
   Testerilizacionp[3]=read_eeprom(3);
   delay_ms(50);   
   Temperatura1=make16(read_eeprom(81),read_eeprom(80));
   delay_ms(50);   
   Temperatura2=make16(read_eeprom(84),read_eeprom(83));
   delay_ms(50);   
   Temperatura3=make16(read_eeprom(87),read_eeprom(86));
   delay_ms(50);   
   Password[0]=read_eeprom(10);
   delay_ms(50);   
   Password[1]=read_eeprom(11);
   delay_ms(50);   
   Password[2]=read_eeprom(12);
   delay_ms(50);   
   Password[3]=read_eeprom(13);
   delay_ms(50);   
   Ganancia=read_eeprom(90);
   delay_ms(50);   
   Tciclo=read_eeprom(30);
   delay_ms(50);   
   Ajuste=read_eeprom(40);
   delay_ms(50);   
   Ajusten=read_eeprom(50);
   delay_ms(50);   
   MediaMovil=read_eeprom(60);
   delay_ms(50);   
   Coeficiente=read_eeprom(70);
   delay_ms(50);   
   Saturacion=read_eeprom(100);
   
   if(Ganancia>10.0)
   {
      Ganancia=2.0;
   }
   
   glcd_init(ON);             //Inicializa la glcd
   glcd_fillScreen(OFF);      //Limpia la pantalla
   strcpy(Menus.Letra,"JP Inglobal");
   displayMenu(Menus.Letra,0,0,0,2);     
   strcpy(Menus.Letra,"    Incubadora");
   displayMenu(Menus.Letra,0,20,0,1);       
   strcpy(Menus.Letra,"    Refrigerada");
   displayMenu(Menus.Letra,0,30,0,1);
   strcpy(Menus.Letra,"   115 Lts");
   displayMenu(Menus.Letra,0,45,0,2);
   delay_ms(1000);

   glcd_fillScreen(OFF);      //Limpia la pantalla
   displayContrasena(Contrasena[0],20,30,1);
   displayContrasena(Contrasena[1],40,30,0);
   displayContrasena(Contrasena[2],60,30,0);
   displayContrasena(Contrasena[3],80,30,0);
   
   for(i=0;i<80;i++)
   {
      Temperatura=Leer_Sensor(MediaMovil);
   }
   Teclado();
   if(UP && DOWN)// && RIGHT && LEFT)
   {
      delay_ms(500);
      strcpy(Menus.Letra,"Cargando..");
      displayMenu(Menus.Letra,0,30,1,1);
      delay_ms(2000);
      Menu=100;
      glcd_fillScreen(OFF);
      UP=0;DOWN=0;RIGHT=0;LEFT=0;
   }
   while(true)
   {

      if(Lectura2==1)
      {
         Temperatura=Leer_Sensor(MediaMovil);
         Lectura2=0;
      }

      Teclado();
   
      glcd_update();            
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==0){ //Menu de Contrase?a.
      
      strcpy(Menus.Letra,"Clave");
      displayMenu(Menus.Letra,30,0,1,2);     
      
      if(UP)//Si oprime hacia arriba
      {
         UP=0;delay_ms(30);
         for(i=1;i<=4;i++)
         {
            if(unidad==i)
            {
               Contrasena[i-1]++;
               if(Contrasena[i-1]>9)   
               {
                  Contrasena[i-1]=0;
               }
               displayContrasena(Contrasena[i-1],i*20,30,1);
            }
            else
            {
               displayContrasena(Contrasena[i-1],i*20,30,0);
            }
         }
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         DOWN=0;delay_ms(30);
         for(i=1;i<=4;i++)
         {
            if(unidad==i)
            {
               Contrasena[i-1]--;
               if(Contrasena[i-1]<0)
                  {Contrasena[i-1]=9;}
               displayContrasena(Contrasena[i-1],i*20,30,1);}
            else
            {displayContrasena(Contrasena[i-1],i*20,30,0);}
         }        
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         RIGHT=0;delay_ms(30);unidad++;
         for(i=1;i<=4;i++)
         {
            if(unidad==i)
            {displayContrasena(Contrasena[i-1],i*20,30,1);}
            else
            {displayContrasena(Contrasena[i-1],i*20,30,0);}
         }
      }
    
      if(unidad>4)
      {glcd_fillScreen(OFF);unidad=4;
         if(Contrasena[0]==3&&Contrasena[1]==8&&Contrasena[2]==9&&Contrasena[3]==2) // Si Ingresa clave para reset general del sistema.
            {write_eeprom(10,0);delay_ms(20);write_eeprom(11,0);delay_ms(20);// Reestablece a contrase?a de Fabrica y reinicia Programa.
             write_eeprom(12,0);delay_ms(20);write_eeprom(13,0);delay_ms(20);
             reset_cpu();}        
         
         if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3]))
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Correcta");
            displayMenu(Menus.Letra,15,30,0,2);
            Menu=1;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
         }// Esta parte se puede agregar en el Menu 1 dependiendo de la ubicaci?n del vidrio.
         else
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Incorrecta");
            displayMenu(Menus.Letra,0,30,0,2);
            Menu=0;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
            displayContrasena(Contrasena[0],20,30,1);
            displayContrasena(Contrasena[1],40,30,0);
            displayContrasena(Contrasena[2],60,30,0);
            displayContrasena(Contrasena[3],80,30,0);
         }
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       


//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==1){ //Menu Principal.      
      
         if(Opcion>4)
         {Opcion=1;}
         if(Opcion<1)
         {Opcion=4;}
         
         strcpy(Menus.Letra,"Menu");
         displayMenu(Menus.Letra,30,0,1,2);
      
         if(Opcion==1)   
            {strcpy(Menus.Letra,"Iniciar Proceso");
            displayMenu(Menus.Letra,0,20,1,1);}
         else
            {strcpy(Menus.Letra,"Iniciar Proceso");
            displayMenu(Menus.Letra,0,20,0,1);}
         
         if(Opcion==2)   
            {strcpy(Menus.Letra,"Seleccion Temperatura");
            displayMenu(Menus.Letra,0,30,1,1);}
         else
            {strcpy(Menus.Letra,"Seleccion Temperatura");
            displayMenu(Menus.Letra,0,30,0,1);}
            
         if(Opcion==3)   
            {strcpy(Menus.Letra,"Temporizador");
            displayMenu(Menus.Letra,0,40,1,1);}
         else
            {strcpy(Menus.Letra,"Temporizador");
            displayMenu(Menus.Letra,0,40,0,1);}   
                        
         if(Opcion==4)   
            {strcpy(Menus.Letra,"Cambio Clave");
            displayMenu(Menus.Letra,0,50,1,1);}
         else
            {strcpy(Menus.Letra,"Cambio Clave");
            displayMenu(Menus.Letra,0,50,0,1);}     
            
         if(UP)//Si oprime hacia arriba
         {
            UP=0;delay_ms(30);Opcion--;visual=1;
         }
                  
         if(DOWN)//Si oprime hacia abajo
         {
            DOWN=0;delay_ms(30);Opcion++;visual=1;   
         }
               
         if(RIGHT)//Si oprime SET
         { 
            RIGHT=0;delay_ms(500);Menu=Opcion+3;Opcion=1;glcd_fillscreen(OFF);visual=1;
         }
      

   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       
//-Sub-Menus de selecci?n de proceso.

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==4){ //Menu de Continuar
      
      if(Opcion>3)
         Opcion=1;
         
      if(Opcion<1)
         Opcion=3;
      
         strcpy(Menus.Letra,"Seleccione");
         displayMenu(Menus.Letra,0,0,1,2);
         
         strcpy(Menus.Letra,"Temperatura");
         displayMenu(Menus.Letra,0,16,1,2);
         
         displayContrasena(Opcion,10,38,1);
         glcd_circle(90,38,2,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,97,35,2,2);         
         
         if(Opcion==1)
         {
            displayTemperatura(Temperatura1,50,38,0,2);
         }
         
         if(Opcion==2)
         {
            displayTemperatura(Temperatura2,50,38,0,2);
         }
         
         if(Opcion==3)
         {
            displayTemperatura(Temperatura3,50,38,0,2);
         }
         
        
      if(UP)//Si oprime hacia arriba
      {
         Opcion++;
         glcd_rect(40, 35, 85, 60, YES, OFF);
         UP=0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         Opcion--;  
         glcd_rect(40, 35, 85, 60, YES, OFF);
         DOWN=0;
      }
            
      if(RIGHT)//Si oprime SET
      {  
         if(Opcion==1)
         {
            Setpoint=Temperatura1;
         }
         
         if(Opcion==2)
         {
            Setpoint=Temperatura2;
         }
         
         if(Opcion==3)
         {
            Setpoint=Temperatura3;
         }
         
         if(Setpoint<Temperatura)
         {
            Enfria=1;
            Rele1_on;Rele2_on;Rele3_on;
         }
         else
         {
            Enfria=0;
            Rele1_off;Rele2_off;Rele3_off;
         }
         
         Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;glcd_fillScreen(OFF);pausa=0;
         Ganancia=Setpoint*(Ganancia/Coeficiente);
         //Tpulsomin=(Setpoint/150.0);
         Gananciamax=(0.27)*Setpoint;
         RIGHT=0;LEFT=0;
      } 
                  
      if(LEFT)//Si oprime boton de Toma.
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
         LEFT=0;
      }
    
    
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==5){ // Menu de Temperatura
     
         if(Opcion>3)
         {Opcion=1;}
         if(Opcion<1)
         {Opcion=3;}
         
      
         if(Opcion==1)   
         {
            strcpy(Menus.Letra,"Temperatura 1");
            displayTemps(Menus.Letra,Temperatura1,0,20,1,1);
         }
         else
         {
            strcpy(Menus.Letra,"Temperatura 1");
            displayTemps(Menus.Letra,Temperatura1,0,20,0,1);
         }
         
         if(Opcion==2)   
         {
            strcpy(Menus.Letra,"Temperatura 2");
            displayTemps(Menus.Letra,Temperatura2,0,35,1,1);
         }
         else
         {
            strcpy(Menus.Letra,"Temperatura 2");
            displayTemps(Menus.Letra,Temperatura2,0,35,0,1);
         }
            
         if(Opcion==3)   
         {
            strcpy(Menus.Letra,"Temperatura 3");
            displayTemps(Menus.Letra,Temperatura3,0,50,1,1);
         }
         else
         {
            strcpy(Menus.Letra,"Temperatura 3");
            displayTemps(Menus.Letra,Temperatura3,0,50,0,1);
         }
              
               
         if(RIGHT)//Si oprime SET
         { 
            RIGHT=0;Opcion++;
         }
      
         strcpy(Menus.Letra,"Temperatura");
         displayMenu(Menus.Letra,0,0,1,2);
         
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Opcion==1)
               Temperatura1++;
            
            if(Opcion==2)
               Temperatura2++;
               
            if(Opcion==3)
               Temperatura3++;   
            UP=0;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Opcion==1)
               Temperatura1--;
            
            if(Opcion==2)
               Temperatura2--;
               
            if(Opcion==3)
               Temperatura3--;   
            DOWN=0;
         }
                          
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(80,make8(Temperatura1,0));
            delay_ms(50);   
            write_eeprom(81,make8(Temperatura1,1));//Guardar valor de Setpoint en eeprom
            delay_ms(50);   
            write_eeprom(83,make8(Temperatura2,0));
            delay_ms(50);   
            write_eeprom(84,make8(Temperatura2,1));//Guardar valor de Setpoint en eeprom
            delay_ms(50);   
            write_eeprom(86,make8(Temperatura3,0));
            delay_ms(50);   
            write_eeprom(87,make8(Temperatura3,1));//Guardar valor de Setpoint en eeprom
            delay_ms(50);   
            LEFT=0;
         }
         
         if(Temperatura1<1)
            Temperatura1=60;
            
         if(Temperatura2<1)
            Temperatura2=60;
            
         if(Temperatura3<1)
            Temperatura3=60;
            
         if(Temperatura1>60)
            Temperatura1=1;   
            
         if(Temperatura2>60)
            Temperatura2=1;   
            
         if(Temperatura3>60)
            Temperatura3=1;   
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    


//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==6){ // Menu de Temporizador
     
      
         if(unidad<1)
         {unidad=4;}
         
         strcpy(Menus.Letra,"Temporiza");
         displayMenu(Menus.Letra,0,0,1,2);
         
         if(unidad==4)
         {displayTiempo(Testerilizacionp[3],20,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[3],20,40,0,2);}
         
         if(unidad==3)
         {displayTiempo(Testerilizacionp[2],40,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[2],40,40,0,2);}
         
         if(unidad==2)
         {displayTiempo(Testerilizacionp[1],70,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[1],70,40,0,2);}
         
         if(unidad==1)
         {displayTiempo(Testerilizacionp[0],90,40,1,2);}
         else
         {displayTiempo(Testerilizacionp[0],90,40,0,2);}
         
         strcpy(Menus.Letra,":");
         displayMenu(Menus.Letra,60,42,2,2);           
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Testerilizacionp[unidad-1]<9)
            {
               delay_ms(30);Testerilizacionp[unidad-1]++;visual=1;
            }
            UP=0;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Testerilizacionp[unidad-1]>0)
            {
               delay_ms(30);Testerilizacionp[unidad-1]--;visual=1;     
            }
            DOWN=0;
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);unidad--;visual=1;
            RIGHT=0;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(0,Testerilizacionp[0]);
            delay_ms(50);   
            write_eeprom(1,Testerilizacionp[1]);
            delay_ms(50);   
            write_eeprom(2,Testerilizacionp[2]);
            delay_ms(50);   
            write_eeprom(3,Testerilizacionp[3]);
            delay_ms(50);   
            LEFT=0;
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    


//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==7){ // Menu de Cambio de Contrase?a
        
      strcpy(Menus.Letra,"Clave");
      displayMenu(Menus.Letra,30,0,1,2);     
      
      if(unidad==1)
      {
         displayContrasena(Contrasena[0],20,30,1);
         displayContrasena(Contrasena[1],40,30,0);
         displayContrasena(Contrasena[2],60,30,0);
         displayContrasena(Contrasena[3],80,30,0);
      }
      else
      {
         if(unidad==2)
         {
            displayContrasena(Contrasena[0],20,30,0);
            displayContrasena(Contrasena[1],40,30,1);
            displayContrasena(Contrasena[2],60,30,0);
            displayContrasena(Contrasena[3],80,30,0);
         }
         else
         {
            if(unidad==3)
            {
               displayContrasena(Contrasena[0],20,30,0);
               displayContrasena(Contrasena[1],40,30,0);
               displayContrasena(Contrasena[2],60,30,1);
               displayContrasena(Contrasena[3],80,30,0);
            }
            else
            {
               if(unidad==4)
               {
                  displayContrasena(Contrasena[0],20,30,0);
                  displayContrasena(Contrasena[1],40,30,0);
                  displayContrasena(Contrasena[2],60,30,0);
                  displayContrasena(Contrasena[3],80,30,1);
               }
            }
         }
      }
      
      if(UP)//Si oprime hacia arriba
      {
         UP=0;delay_ms(30);
         for(i=1;i<=4;i++)
         {
            if(unidad==i)
            {
               Contrasena[i-1]++;
               if(Contrasena[i-1]>9)   
               {
                  Contrasena[i-1]=0;
               }
            }
         }
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         DOWN=0;delay_ms(30);
         for(i=1;i<=4;i++)
         {
            if(unidad==i)
            {Contrasena[i-1]--; 
            if(Contrasena[i-1]<0)
            {Contrasena[i-1]=9;}
            }
         }  
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         RIGHT=0;delay_ms(30);unidad++;
      }
      
      if(LEFT)//Si oprime Izquierda
      {
         LEFT=0;delay_ms(700);Menu=1;glcd_fillscreen(OFF);
      }
      
      if(unidad>4)
      {glcd_fillScreen(OFF);
         
         if((Contrasena[0]==Password[0])&&(Contrasena[1]==Password[1])&&(Contrasena[2]==Password[2])&&(Contrasena[3]==Password[3]))
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Correcta");
            displayMenu(Menus.Letra,15,30,0,2);
            Menu=91;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
         }// Esta parte se puede agregar en el Menu 1 dependiendo de la ubicaci?n del vidrio.
         else
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,30,0,0,2);
            strcpy(Menus.Letra,"Incorrecta");
            displayMenu(Menus.Letra,0,30,0,2);
            Menu=1;glcd_update();delay_ms(1000);
            glcd_fillScreen(OFF);unidad=1;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
         }
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==91){ // Menu de Cambio de Contrase?a
        
      strcpy(Menus.Letra,"Clave Nueva");
      displayMenu(Menus.Letra,0,0,1,2);     
      
      if(unidad==1)
      {
         displayContrasena(Password[0],20,30,1);
         displayContrasena(Password[1],40,30,0);
         displayContrasena(Password[2],60,30,0);
         displayContrasena(Password[3],80,30,0);
      }
      else
      {
         if(unidad==2)
         {
            displayContrasena(Password[0],20,30,0);
            displayContrasena(Password[1],40,30,1);
            displayContrasena(Password[2],60,30,0);
            displayContrasena(Password[3],80,30,0);
         }
         else
         {
            if(unidad==3)
            {
               displayContrasena(Password[0],20,30,0);
               displayContrasena(Password[1],40,30,0);
               displayContrasena(Password[2],60,30,1);
               displayContrasena(Password[3],80,30,0);
            }
            else
            {
               if(unidad==4)
               {
                  displayContrasena(Password[0],20,30,0);
                  displayContrasena(Password[1],40,30,0);
                  displayContrasena(Password[2],60,30,0);
                  displayContrasena(Password[3],80,30,1);
               }
            }
         }
      }
      
      if(UP)//Si oprime hacia arriba
      {
         UP=0;delay_ms(30);
         for(i=1;i<=4;i++)
         {
            if(unidad==i)
            {
               Password[i-1]++;
               if(Password[i-1]>9)   
               {
                  Password[i-1]=0;
               }
            }
         }
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         DOWN=0;delay_ms(30);
         for(i=1;i<=4;i++)
            {
               if(unidad==i)
               {
                  Password[i-1]--;
                  if(Password[i-1]<0)
                  {
                     Password[i-1]=9;
                  }
               }
            }       
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         RIGHT=0;delay_ms(30);unidad++;
      }
      
      if(LEFT)//Si oprime Izquierda
      {
         LEFT=0;delay_ms(700);Menu=1;glcd_fillscreen(OFF);
      }
      
      if(unidad>4)
      {
         Opcion=1;unidad=1;
         write_eeprom(10,Password[0]);
         delay_ms(50);   
         write_eeprom(11,Password[1]);
         delay_ms(50);   
         write_eeprom(12,Password[2]);
         delay_ms(50);   
         write_eeprom(13,Password[3]);
         delay_ms(1000);glcd_fillScreen(OFF);Menu=1;
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    



//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==20){ //Menu de Funcionamiento
      
      // Muestra imagen de fondo en Display GLCD
      glcd_imagen(5);

      if(UP && DOWN)//Si oprime hacia abajo
      {
         glcd_init(ON);flag_display=!flag_display;
         UP=0;DOWN=0;
      }

      if(Pausa==0 && !TERM && !DOOR)
      {
         Velocidades(Velocidad);
         if(Enfria==1)
         {
            if(Temperatura<=Setpoint+1.0)
            {
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Status:Controlando");
               displayMenu(Menus.Letra,0,55,2,1);
               tiempo_esterilizacion=1;
               Controla();
               flag_tiempocalentamiento=0;
            }
            else
            {
               flag_tiempocalentamiento=0;
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Status:Enfriando");
               displayMenu(Menus.Letra,0,55,2,1);
               tiempo_esterilizacion=0;
               Controla();
            }
         }
         else
         {
            if(Temperatura>=Setpoint-1.0)
            {
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Status:Controlando");
               displayMenu(Menus.Letra,0,55,2,1);
               tiempo_esterilizacion=1;
               Controla();
               flag_tiempocalentamiento=0;
            }
            else
            {
               flag_tiempocalentamiento=1;
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Status:Calentando");
               displayMenu(Menus.Letra,0,55,2,1);
               tiempo_esterilizacion=0;
               Controla();
            }
         }
      }
      
      if(Pausa==1 && !TERM && !DOOR)
      {
         Velocidades(0);
         flag_tiempocalentamiento=0;
         strcpy(Menus.Letra,"Status:Pausado");
         displayMenu(Menus.Letra,0,55,2,1);
         tiempo_esterilizacion=0;
         SSR_off;
         //Peltierp_off;
         //Peltiern_off;
         Tpulso=0.0;
      }
      
      if(DOOR)
      {
         Velocidades(0);
         flag_tiempocalentamiento=0;
         if(flag_clear2==0)
         {
            Ganancia=read_eeprom(90);
            Ganancia=Setpoint*(Ganancia/Coeficiente);
            Pausa_ant=Pausa;
            Pausa=0;
            //glcd_fillScreen(OFF);
            glcd_rect(0, 55, 127, 63, YES, OFF);
            tiempo_esterilizacion=0;
            SSR_off;
            //Peltierp_off;
            //Peltiern_off;
            Tpulso=0.0;
            flag_clear2=1;
            flag_ganancia=0;
         }
         strcpy(Menus.Letra,"Status:Puerta Abierta");
         displayMenu(Menus.Letra,0,55,2,1);
      }
      else
      {
         if(flag_clear2==1)
         {
            Pausa=Pausa_ant;
            //glcd_fillScreen(OFF);
            glcd_rect(0, 55, 127, 63, YES, OFF);
            flag_clear2=0;
         }
      }
      
      if(TERM)
      {
         Velocidades(0);
         flag_tiempocalentamiento=0;
         if(flag_clear3==0)
         {
            Pausa_ant=Pausa;
            Pausa=0;
            //glcd_fillScreen(OFF);
            glcd_rect(0, 55, 127, 63, YES, OFF);
            tiempo_esterilizacion=0;
            SSR_off;
            //Peltierp_off;
            //Peltiern_off;
            Tpulso=0.0;
            flag_clear3=1;
            flag_ganancia=0;
         }
         strcpy(Menus.Letra,"Status:Alarma 01");
         displayMenu(Menus.Letra,0,55,2,1);
         
         Buzzer_on;
         delay_ms(1000);
         Buzzer_off;
         delay_ms(1000);
      }
      else
      {
         if(flag_clear3==1)
         {
            Pausa=Pausa_ant;
            //glcd_fillScreen(OFF);
            glcd_rect(0, 55, 127, 63, YES, OFF);
            flag_clear3=0;
         }
      }  
      
      if(UP)//Si oprime hacia arriba
      {
         Velocidad++;
         UP=0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         Velocidad--;
         DOWN=0;
      }
      
      if(Velocidad>3)
         Velocidad=1;
      
      if(Velocidad<1)
         Velocidad=3;      
      
      if(RIGHT)//Si oprime SET
      {  
         if(Pausa!=2)
         {
            if(Pausa==0)
            {
               delay_ms(200);Pausa=1;
            }
            else
            {
               delay_ms(200);Pausa=0;
            }
            glcd_rect(0, 55, 127, 63, YES, OFF);
         }
         RIGHT=0;
      }
      
        
      if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
      {                 
         
         Lectura=0;
         Potencia=(100.0/Tciclo)*Tpulso;
         
 //----------------Muestra Temperatura--------------------------
         if(Pt>=247)
         {
            glcd_rect(15, 29, 69, 45, YES, OFF);
            strcpy(Menus.Letra,"-----");displayMenu(Menus.Letra,15,29,2,2);
            Desconectado=1;
         }
         else
         {
            if(Desconectado==1)
            {
               Desconectado=0;
               delay_ms(2000);
               for(i=0;i<80;i++)
               {
                  Temperatura=Leer_Sensor(80);
               }
            }
            displayT(Temperatura,15,29,2);
         }

         //displayT(Temperatura,15,29,2);
         glcd_circle(74,30,2,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,75,30,2,2);
         
         displayTemperatura(Setpoint,30,45,0,1);
         glcd_circle(50,47,1,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,53,45,2,1);
         strcpy(Menus.Letra,"Temp:");displayMenu(Menus.Letra,0,45,2,1); 
         
         displayTiempo(Potencia,97,42,3,1);strcpy(Menus.Letra,"%");displayMenu(Menus.Letra,115,42,2,1);
         //displayTiempo(Tpulsomin*10,97,42,3,1);strcpy(Menus.Letra,"%");displayMenu(Menus.Letra,115,42,2,1);
         
         strcpy(Menus.Letra,"Vel:");displayMenu(Menus.Letra,97,13,2,1);
         displayTiempo(Velocidad,115,13,0,1);
         
         if(flag_display==1)
            //displayTiempo(Ganancia,97,13,3,1);
            //displayTemperatura(Prom,97,13,0,1);
            displayT(Prom,60,46,1);
         else
            glcd_rect(60, 46, 88, 52, YES, OFF);
         
         // Muestra Valor de esterilizacion Programado.
         displayTiempo(Testerilizacion[3],20,2,0,2);displayTiempo(Testerilizacion[2],33,2,0,2);
         strcpy(Menus.Letra,":");displayMenu(Menus.Letra,46,2,2,2); 
         displayTiempo(Testerilizacion[1],55,2,0,2);displayTiempo(Testerilizacion[0],68,2,0,2);
         
         // Muestra Valor de esterilizacion actual.
         strcpy(Menus.Letra,"Time:");displayMenu(Menus.Letra,0,18,2,1); 
         displayTiempo(Testerilizacionp[3],30,18,0,1);displayTiempo(Testerilizacionp[2],36,18,0,1);
         strcpy(Menus.Letra,":");displayMenu(Menus.Letra,42,18,2,1); 
         displayTiempo(Testerilizacionp[1],48,18,0,1);displayTiempo(Testerilizacionp[0],54,18,0,1);
         strcpy(Menus.Letra,"(H:M)");displayMenu(Menus.Letra,60,18,2,1); 
         
         glcd_rect(0, 0, 89, 27, NO, ON);
         glcd_rect(0, 27, 89, 54, NO, ON);
      } 
      
      // Si cumplio el tiempo de esterilizaci?n, Muestre ciclo finalizado y detenga control de temperatura.
      if((Testerilizacion[0]>=Testerilizacionp[0])&&(Testerilizacion[1]>=Testerilizacionp[1])&&(Testerilizacion[2]>=Testerilizacionp[2])&&(Testerilizacion[3]>=Testerilizacionp[3]) && tiempo_esterilizacion==1)
      {
         if(flag_clear==0)
         {
            glcd_fillScreen(OFF);
            //glcd_rect(0, 55, 127, 63, YES, OFF);
            tiempo_esterilizacion=0;
            Pausa=2;
            flag_clear=1;
            //Resistencia_off;
            Velocidades(0);
            Tpulso=0.0;
            flag_ganancia=0;
            Buzzer_on;
            delay_ms(1000);
            Buzzer_off;
            delay_ms(1000);
         }
         strcpy(Menus.Letra,"Status:Finalizado");
         displayMenu(Menus.Letra,0,55,2,1);
      } 
      
      // Si ha pasado un tiempo x y no ha llegado al setpoint
      if(tiempocalentamiento>7200 && tiempo_esterilizacion==0)
      {
         if(flag_clear4==0)
         {
            glcd_rect(0, 55, 127, 63, YES, OFF);
            tiempo_esterilizacion=0;
            Pausa=2;
            flag_clear4=1;
            //Resistencia_off;
            Velocidades(0);
            Tpulso=0.0;
            flag_ganancia=0;
         }
         strcpy(Menus.Letra,"Status:Alarma 02");
         displayMenu(Menus.Letra,0,55,2,1);
         Buzzer_on;
         delay_ms(1000);
         Buzzer_off;
         delay_ms(1000);
      }
      
      if(tinicio>=60)
      {
         tinicio=0;
         Testerilizacion[0]++;
         if(Testerilizacion[0]>9)
         {
            Testerilizacion[0]=0;
            Testerilizacion[1]++;
         }
         if(Testerilizacion[1]>5)
         {
            Testerilizacion[1]=0;
            Testerilizacion[2]++;
         }
         if(Testerilizacion[2]>9)
         {
            Testerilizacion[2]=0;
            Testerilizacion[3]++;
         }
         if(Testerilizacion[3]>9)
         {
            Testerilizacion[3]=0;
         }
      }
      
      if(LEFT)//Si oprime boton de Toma.
      {
         if(Pausa!=2)
         {
            delay_ms(500);Menu=30;glcd_fillscreen(OFF);Opcion=1;visual=1;Pausa=1;
         }
         LEFT=0;
      }
        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==30){ //Menu de Retroceso
      
      strcpy(Menus.Letra,"Regresar");
      displayMenu(Menus.Letra,0,0,1,2);
         
      if(Opcion==2)
      {strcpy(Menus.Letra,"SI");
      displayMenu(Menus.Letra,50,25,1,2);
      strcpy(Menus.Letra,"NO");
      displayMenu(Menus.Letra,50,45,0,2);
      }
      if(Opcion==1)
      {strcpy(Menus.Letra,"SI");
      displayMenu(Menus.Letra,50,25,0,2);
      strcpy(Menus.Letra,"NO");
      displayMenu(Menus.Letra,50,45,1,2);
      }  
        
      if(UP)//Si oprime hacia arriba
      {
         if(Opcion<2)
         {
            Opcion++;
         }
         visual=1;
         UP=0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         if(Opcion>1)
         {
            Opcion--;
         }  
         visual=1;
         DOWN=0;
      }
            
      if(RIGHT)//Si oprime SET
      {  
         if(Opcion==2)
         {
            Menu=1;glcd_fillScreen(OFF);Testerilizacion[0]=0;Testerilizacion[1]=0;Testerilizacion[2]=0;Testerilizacion[3]=0;
            //Resistencia_off;
            //Ventilador1_off;
            Tpulso=0.0;
            Opcion=1;
         }
         else
         {
            delay_ms(700);Menu=20;glcd_fillscreen(OFF);Opcion=1;visual=1;
         }
         RIGHT=0;
      } 
                  
      if(LEFT)//Si oprime boton de Toma.
      {
         LEFT=0;delay_ms(700);Menu=20;glcd_fillscreen(OFF);Opcion=1;visual=1;
      }
        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuraci?n de Parametros de Control
      
      if(Opcion>5)
      {Opcion=1;}
      if(Opcion<1)
      {Opcion=5;}
      
         if(Opcion==1)   
            {strcpy(Menus.Letra,"Ganancia");
            displayMenu(Menus.Letra,0,0,1,1);}
         else
            {strcpy(Menus.Letra,"Ganancia");
            displayMenu(Menus.Letra,0,0,0,1);}
         
         if(Opcion==2)   
            {strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,9,1,1);}
         else
            {strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,9,0,1);}
            
         if(Opcion==3)   
            {strcpy(Menus.Letra,"Ajuste +");
            displayMenu(Menus.Letra,0,18,1,1);}
         else
            {strcpy(Menus.Letra,"Ajuste +");
            displayMenu(Menus.Letra,0,18,0,1);}   
                        
         if(Opcion==4)   
            {strcpy(Menus.Letra,"Ajuste -");
            displayMenu(Menus.Letra,0,27,1,1);}
         else
            {strcpy(Menus.Letra,"Ajuste -");
            displayMenu(Menus.Letra,0,27,0,1);}     
            
         if(Opcion==5)   
            {strcpy(Menus.Letra,"Media Movil");
            displayMenu(Menus.Letra,0,36,1,1);}
         else
            {strcpy(Menus.Letra,"Media Movil");
            displayMenu(Menus.Letra,0,36,0,1);}     
              
      
         if(UP)//Si oprime hacia arriba
         {
            UP=0;delay_ms(30);
            Opcion--;visual=1;
         }
                  
         if(DOWN)//Si oprime hacia abajo
         {
            DOWN=0;delay_ms(30);
            Opcion++;visual=1;
         }
               
         if(RIGHT)//Si oprime SET
         { 
            RIGHT=0;delay_ms(30);
            Menu=Opcion+100;Opcion=1;glcd_fillscreen(OFF);visual=1;
         }
         
         if(LEFT)//Si oprime SET
         { 
            LEFT=0;delay_ms(30);
            Menu=Opcion+100;Opcion=1;glcd_fillscreen(OFF);visual=1;
         }
        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==101){ // Menu de Ajuste de Ganancia
      
      if(Ganancia>40)
      {
         Ganancia=2;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Ganancia");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Ganancia,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Ganancia>40)
            {
               Ganancia=40;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Ganancia+=1.0;visual=1;
            }
            UP=0;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Ganancia>1)
            {
               delay_ms(30);Ganancia-=1.0;visual=1;     
            }
            DOWN=0;
         }
               
         if(RIGHT)//Si oprime SET
         {  
            RIGHT=0;delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(90,(int8)Ganancia);
            LEFT=0;
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------


//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==102){ // Menu de Ajuste de Ganancia2
      
      if(Tciclo>40)
      {
         Tciclo=40;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Tciclo");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Tciclo,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Tciclo>40)
            {
               Tciclo=40;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Tciclo+=1.0;visual=1;
            }
            UP=0;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Tciclo>1)
            {
               delay_ms(30);Tciclo-=1.0;visual=1;     
            }
            DOWN=0;
         }
               
         if(RIGHT)//Si oprime SET
         {  
            RIGHT=0;delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(30,(int8)Tciclo);
            LEFT=0;
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==103){ // Menu de Ajuste de Ajuste
      
      if(Ajuste>100)
      {
         Ajuste=0;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Ajuste");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Ajuste,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Ajuste>100)
            {
               Ajuste=100;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Ajuste+=1.0;visual=1;
            }
            UP=0;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Ajuste>0)
            {
               delay_ms(30);Ajuste-=1.0;visual=1;     
            }
            DOWN=0;
         }
               
         if(RIGHT)//Si oprime SET
         {  
            RIGHT=0;delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(40,Ajuste);
            LEFT=0;
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==104){ // Menu de Ajuste de Ajusten
      
      if(Ajusten>100)
      {
         Ajusten=0;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Ajusten");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Ajusten,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Ajusten>100)
            {
               Ajusten=100;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Ajusten+=1.0;visual=1;
            }
            UP=0;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Ajusten>0)
            {
               delay_ms(30);Ajusten-=1.0;visual=1;     
            }
            DOWN=0;
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
            RIGHT=0;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(50,Ajusten);
            LEFT=0;
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==105){ // Menu de Ajuste de Media Movil
      
      if(MediaMovil>80)
      {
         MediaMovil=10;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Media");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(MediaMovil,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            if(MediaMovil>79)
            {
               MediaMovil=79;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);MediaMovil+=1.0;visual=1;
            }
            UP=0;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(MediaMovil>1)
            {
               delay_ms(30);MediaMovil-=1.0;visual=1;     
            }
            DOWN=0;
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
            RIGHT=0;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(60,MediaMovil);
            LEFT=0;
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------


      if(tiemporeset>=60)
      {
         glcd_init(ON);
         tiemporeset=0;
      } 

   }
}
