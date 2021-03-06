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
#fuses XTPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN,NOMCLR
#use delay(clock=48000000)
#include "HDM64GS12.c"
#include "GRAPHICS.c"
#include "imagen.h"

// Definici?n de teclado - NO
#define   UP            input(PIN_A5)
#define   DOWN          input(PIN_E0)
#define   RIGHT         input(PIN_E1)
#define   LEFT          input(PIN_E2)

//#define   DOOR          input(PIN_A3)
#define   TERM          input(PIN_A4)

// Definici?n de Salidas
#define   Resistencia_on     output_bit(PIN_C0,1)
#define   Resistencia_off    output_bit(PIN_C0,0)
#define   Resistencia2_on    output_bit(PIN_C2,1)
#define   Resistencia2_off   output_bit(PIN_C2,0)
#define   Ventilador_on      output_bit(PIN_C1,1)
#define   Ventilador_off     output_bit(PIN_C1,0)
#define   Libre1_on          output_bit(PIN_C6,1)
#define   Libre1_off         output_bit(PIN_C6,0)
#define   Libre2_on          output_bit(PIN_C7,1)
#define   Libre2_off         output_bit(PIN_C7,0)
#define   Libre3_on          output_bit(PIN_B0,1)
#define   Libre3_off         output_bit(PIN_B0,0)
#define   Libre4_on          output_bit(PIN_B1,1)
#define   Libre4_off         output_bit(PIN_B1,0)

float promedio[80]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float X[2]={0.0,0.0};
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=60,tiemporecta=0,tiempoventilador=0;
short Flanco=0,Flanco1=0,Flanco2=0,visual=1,Lectura=0,estado_ventilador=0;
short flag_clear=0,flag_clear2=0,flag_clear3=0,flag_clear4=0,flag_tiempocalentamiento=0,flag_display=0,flag_setpoint=0,flag_control=0;
int8 y=0,tiemporeset=0,flag_ventilador=0;
int8 Menu=0, unidad=1,i=0,Pausa=0,Pausa_ant=0,TiempoControl=0;
int8 Setpoint=0,Opcion=1,tiempo_esterilizacion=0,tiempo_secado=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,tinicio=0,tiempocalentamiento=0,tiemposetpoint=0,Puerta=0;
float Potencia=0.0,Tpulsomin=0.0,Gananciamax=0.0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Contrasena[4]={0,0,0,0};      // Contrasena de 4 digitos
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
float V0=0.0,R1=10000.0,Pt,Tciclo=5.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=1.0,desvio=0.0,Coeficiente=35.0;
float Aumento=0.0,Temperatura=0.0;

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

// Funcion para conversor analogo-digital
float sensores(int x){
float y;set_adc_channel(x);delay_ms(1);y=read_adc();return (y);
}
float Leer_Sensor(int media)
{
   Prom=0.0;
   V0 = sensores(0);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
   V0 = V0/49.9; // Se elimina la ganancia
   Pt=(R1)/((1023.0/V0)-1.0); // Se calcula el valor de la PT100
   //Pt=R1/((1023.0/V0)-1.0); // Se calcula el valor de la PT100
   //Pt=Pt-0.5;
   
            
   if(l>(media-1))
   {l=0;}
   //promedio[l]=Temperature;l++;
   promedio[l]=Pt;l++;
         
   for(h=0;h<media;h++)
   {
      Prom+=promedio[h];
   } 
   Prom=Prom/media;   
   Temperature=(Prom-100.0)/(0.385);  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.  
   //Temperature=(Pt-100.0)/(0.385);  // Ecuaci?n determinada por linealizaci?n en Matlab. Revisar este polinomio.
   
   Temperature+=((float)Ajuste-(float)Ajusten);
   
   //return Prom;
   return Temperature;
}

void Controla(){
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   if(tiemporecta>=20)
   {
      ActualizaRecta();
      tiemporecta=0;
   }
   
   if(tiemposetpoint>600)
   {
      flag_control=1;
   }
   
   if(flag_ventilador==0)
   {
      Ventilador_on;
   }
   else
   {
      if(estado_ventilador==1)
      {
         Ventilador_on;
      }
      else
      {
         Ventilador_off;
      }
   }
  
   error=desvio+Setpoint-Temperatura;      // C?lculo del error ******
   
   /*if(error>1.0 && error<50 && Aumento<0.3 && flag_setpoint==0)
   {
      if(Potencia<90.0)
      {
         if(TiempoControl>=120)
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
   
   if(error<-1.0)
   {
      if(TiempoControl>=120)
      {
         TiempoControl=0;
         Ganancia-=0.2;
      }
   }*/
   
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
   
   // Tpulso(t)= Tciclo*(y(t)-ymin)/(ymax - ymin); calculo de ciclo util para control de resistencia.ymax=140.ymin=0;
   // Tpulso es float, hay que pasar ese ciclo a una salida de un puerto usando el TMR1.
   Tpulso=(tmp/Setpoint)*Tciclo;  
   
   if(tiempoctrl<=(int16)(Tpulso/0.005)) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
   {
      Resistencia_on;
      Resistencia2_on;
   }
   else         
   {
      Resistencia_off;
      Resistencia2_off;
   }
         
   if(tiempoctrl>=(int16)(Tciclo/0.005)) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
   {
      tiempoctrl=0;
   } // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
                                      
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

void displayTemperatura(int digito, int x, int y, int sombreado, int tamano)
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
      sprintf(voltage, "%03u", digito);
   
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
   Libre1_on;Libre2_on;Libre3_on;Libre4_on;
   Ventilador_off;Resistencia_off;Resistencia2_off;
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN3);
   setup_adc(ADC_CLOCK_DIV_64);
   /*setup_ccp1 (CCP_PWM);
   setup_timer_2(T2_DIV_BY_4, 255, 1);   
   set_pwm1_duty(0);*/
   enable_interrupts(global);
   Testerilizacionp[0]=read_eeprom(10);Testerilizacionp[1]=read_eeprom(20);Testerilizacionp[2]=read_eeprom(30);Testerilizacionp[3]=read_eeprom(40);
   Setpoint=read_eeprom(50);
   Password[0]=read_eeprom(60);Password[1]=read_eeprom(70);Password[2]=read_eeprom(80);Password[3]=read_eeprom(90);
   //Ganancia=read_eeprom(20);
   //Tciclo=read_eeprom(30);
   //Ajuste=read_eeprom(40);
   //Ajusten=read_eeprom(50);
   //MediaMovil=read_eeprom(60);
   //Coeficiente=read_eeprom(70);
   
   glcd_init(ON);             //Inicializa la glcd
   glcd_fillScreen(OFF);      //Limpia la pantalla
   strcpy(Menus.Letra,"JP Inglobal");
   displayMenu(Menus.Letra,0,0,0,2);     
   strcpy(Menus.Letra,"     Incubadora");
   displayMenu(Menus.Letra,0,20,0,1);       
   strcpy(Menus.Letra,"   Digital" );
   displayMenu(Menus.Letra,0,30,0,1);
   strcpy(Menus.Letra,"  400 Lts");
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
   /*Menu=20;
   glcd_fillScreen(OFF);
   Ajuste=0;
   Ajusten=0;*/
   while(true)
   {
   
      if(UP && DOWN && RIGHT && LEFT)
      {
         delay_ms(500);
         strcpy(Menus.Letra,"Cargando..");
         displayMenu(Menus.Letra,0,30,1,1);
         delay_ms(2000);
         Menu=100;
         glcd_fillScreen(OFF);
      }
   
      glcd_update();            
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==0){ //Menu de Contrase?a.
      
      strcpy(Menus.Letra,"Clave");
      displayMenu(Menus.Letra,30,0,1,2);     
      
      
      
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
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
      }
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {Contrasena[i-1]--; 
                     if(Contrasena[i-1]<0)
                        {Contrasena[i-1]=9;}
                   displayContrasena(Contrasena[i-1],i*20,30,1);}
                  else
                  {displayContrasena(Contrasena[i-1],i*20,30,0);}
               }      
            }     
      }
         else
            {Flanco2 = 0;}  
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(30);unidad++;
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {displayContrasena(Contrasena[i-1],i*20,30,1);}
                  else
                  {displayContrasena(Contrasena[i-1],i*20,30,0);}
               }
            }
      }
         else
            {Flanco1 = 0;}  
    
      if(unidad>4)
      {glcd_fillScreen(OFF);unidad=4;
         if(Contrasena[0]==3&&Contrasena[1]==8&&Contrasena[2]==9&&Contrasena[3]==2) // Si Ingresa clave para reset general del sistema.
            {write_eeprom(60,0);delay_ms(20);write_eeprom(70,0);delay_ms(20);// Reestablece a contrase?a de Fabrica y reinicia Programa.
             write_eeprom(80,0);delay_ms(20);write_eeprom(90,0);delay_ms(20);
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
            {strcpy(Menus.Letra,"Temperatura");
            displayMenu(Menus.Letra,0,30,1,1);}
         else
            {strcpy(Menus.Letra,"Temperatura");
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
         {  if(Flanco == 0) 
               {Flanco = 1;delay_ms(30);Opcion--;visual=1;
               }
         }
            else
               {Flanco = 0;}
                  
         if(DOWN)//Si oprime hacia abajo
         {  if(Flanco2 == 0) 
               {Flanco2 = 1;delay_ms(30);Opcion++;visual=1;
               }     
         }
            else
               {Flanco2 = 0;}  
               
       if(RIGHT)//Si oprime SET
         {  if(Flanco1 == 0) 
               {Flanco1 = 1;delay_ms(500);Menu=Opcion+3;Opcion=1;glcd_fillscreen(OFF);visual=1;
               }
         }
            else
               {Flanco1 = 0;}  
         /*      
         if(LEFT)//Si oprime boton de Toma.
         {delay_ms(700);Menu=2;glcd_fillscreen(OFF);}*/
   }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       
//-Sub-Menus de selecci?n de proceso.

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==4){ //Menu de Continuar
      
      if(Opcion>2)
         Opcion=1;
         
      if(Opcion<1)
         Opcion=2;
      
         strcpy(Menus.Letra,"Continuar");
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
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         if(Opcion>1)
         {
            Opcion--;
         }  
      }
            
      if(RIGHT)//Si oprime SET
      {  
         if(Opcion==2)
         {
            Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;glcd_fillScreen(OFF);pausa=0;
            Ganancia=Setpoint*(Ganancia/Coeficiente);
         }
         else
         {
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
         }
      } 
                  
      if(LEFT)//Si oprime boton de Toma.
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
      }
    
    
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==5){ // Menu de Temperatura
      glcd_circle(85,30,3,NO,ON);
      strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,92,30,2,3);
      
         //glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Temperatura");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Setpoint,30,30,0,3);
      
         if(UP)//Si oprime hacia arriba
         {  
            //glcd_fillScreen(OFF);
            glcd_rect(30, 30, 80, 55, YES, OFF);
               if(Setpoint>60)
               {
                  Setpoint=60;delay_ms(100);visual=1;
               }
               else
               {
                  delay_ms(30);Setpoint++;visual=1;
               }
            
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            //glcd_fillScreen(OFF);
            glcd_rect(30, 30, 80, 55, YES, OFF);
            if(Setpoint>20)
            {
               delay_ms(30);Setpoint--;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(50,Setpoint);
         }
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
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Testerilizacionp[unidad-1]>0)
            {
               delay_ms(30);Testerilizacionp[unidad-1]--;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);unidad--;visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(10,Testerilizacionp[0]);write_eeprom(20,Testerilizacionp[1]);
            write_eeprom(30,Testerilizacionp[2]);write_eeprom(40,Testerilizacionp[3]);
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
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
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
      }
      else
      {
         Flanco = 0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
               for(i=1;i<=4;i++)
               {
                  if(unidad==i)
                  {Contrasena[i-1]--; 
                     if(Contrasena[i-1]<0)
                        {Contrasena[i-1]=9;}
                  }
               }      
            }     
      }
      else
      {
         Flanco2 = 0;
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(300);unidad++;}
      }
      else
      {
         Flanco1 = 0;
      }
      
      if(LEFT)//Si oprime Izquierda
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);
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
         if(Flanco == 0)
         {
            Flanco = 1;delay_ms(30);
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
      }
      else
      {
         Flanco = 0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
            {Flanco2 = 1;delay_ms(30);
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
      }
      else
      {
         Flanco2 = 0;
      }
            
      if(RIGHT)//Si oprime RIGHT
      {
         if(Flanco1 == 0)
            {Flanco1 = 1;delay_ms(300);unidad++;}
      }
      else
      {
         Flanco1 = 0;
      }
      
      if(LEFT)//Si oprime Izquierda
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);
      }
      
      if(unidad>4)
      {
         Opcion=1;unidad=1;
         write_eeprom(60,Password[0]);write_eeprom(70,Password[1]);write_eeprom(80,Password[2]);write_eeprom(90,Password[3]);
         delay_ms(1000);glcd_fillScreen(OFF);Menu=1;
      }
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    



//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==20){ //Menu de Funcionamiento
      
      // Muestra imagen de fondo en Display GLCD
      glcd_imagen(5);
      
      
      if(Pausa==0 && Puerta<770 && !TERM)
      {
         if(Temperatura>=Setpoint-0.5)
         {
            flag_setpoint=1;
            flag_ventilador=1;
         }
         
         if(Temperatura<Setpoint-0.8)
         {
            flag_ventilador=0;
         }
         
         if(Temperatura<Setpoint-1.2)
         {
            flag_setpoint=0;
         }
         if(Temperatura>=Setpoint-5.0)
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
      
      if(Pausa==1 && Puerta<770 && !TERM)
      {
         flag_tiempocalentamiento=0;
         strcpy(Menus.Letra,"Status:Pausado");
         displayMenu(Menus.Letra,0,55,2,1);
         tiempo_esterilizacion=0;
         Resistencia_off;
         Resistencia2_off;
         Ventilador_off;
         Tpulso=0.0;
      }
      
      if(Puerta>770)
      {
         flag_tiempocalentamiento=0;
         if(flag_clear2==0)
         {
            Pausa_ant=Pausa;
            Pausa=0;
            //glcd_fillScreen(OFF);
            glcd_rect(0, 55, 127, 63, YES, OFF);
            tiempo_esterilizacion=0;
            Resistencia_off;
            Resistencia2_off;
            Ventilador_off;
            Tpulso=0.0;
            flag_clear2=1;
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
         flag_tiempocalentamiento=0;
         if(flag_clear3==0)
         {
            Pausa_ant=Pausa;
            Pausa=0;
            //glcd_fillScreen(OFF);
            glcd_rect(0, 55, 127, 63, YES, OFF);
            tiempo_esterilizacion=0;
            Resistencia_off;
            Resistencia2_off;
            Ventilador_off;
            Tpulso=0.0;
            flag_clear3=1;
         }
         strcpy(Menus.Letra,"Status:Alarma 01");
         displayMenu(Menus.Letra,0,55,2,1);
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
         if(Pausa!=2)
         {
            delay_ms(500);Pausa=0;
            //glcd_fillScreen(OFF);
            glcd_rect(0, 55, 127, 63, YES, OFF);
         }
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         if(Pausa!=2)
         {
            delay_ms(500);Pausa=1;
            //glcd_fillScreen(OFF);
            glcd_rect(0, 55, 127, 63, YES, OFF);
         }
      }
            
      if(RIGHT)//Si oprime SET
      {  
         glcd_init(ON);flag_display=!flag_display;
      } 
        
      if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
      {                 
         
         Lectura=0;
         Potencia=(100.0/Tciclo)*Tpulso;
         Temperatura=Leer_Sensor(MediaMovil);
         Puerta=sensores(3);
         displayT(Temperatura,15,29,2);
         glcd_circle(74,30,2,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,75,30,2,2);
         
         displayTiempo(Setpoint,30,45,3,1);
         glcd_circle(50,47,1,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,53,45,2,1);
         strcpy(Menus.Letra,"Temp:");displayMenu(Menus.Letra,0,45,2,1); 
         
         displayTiempo(Potencia,97,42,3,1);strcpy(Menus.Letra,"%");displayMenu(Menus.Letra,115,42,2,1);
         //displayTiempo(Tpulsomin*10,97,42,3,1);strcpy(Menus.Letra,"%");displayMenu(Menus.Letra,115,42,2,1);
         
         if(flag_display==1)
            displayTiempo(Ganancia,97,13,3,1);
            //displayTiempo(Gananciamax,97,13,3,1);
         else
            glcd_rect(97, 13, 115, 18, YES, OFF);
         
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
            Resistencia_off;
            Resistencia2_off;
            Ventilador_off;
            Tpulso=0.0;
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
            Resistencia_off;
            Resistencia2_off;
            Ventilador_off;
            Tpulso=0.0;
         }
         strcpy(Menus.Letra,"Status:Alarma 02");
         displayMenu(Menus.Letra,0,55,2,1);
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
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         if(Opcion>1)
         {
            Opcion--;
         }  
         visual=1;
      }
            
      if(RIGHT)//Si oprime SET
      {  
         if(Opcion==2)
         {
            Menu=1;glcd_fillScreen(OFF);Testerilizacion[0]=0;Testerilizacion[1]=0;Testerilizacion[2]=0;Testerilizacion[3]=0;
            Resistencia_off;
            Resistencia2_off;
            Ventilador_off;
            Tpulso=0.0;
            Opcion=1;
         }
         else
         {
            delay_ms(700);Menu=20;glcd_fillscreen(OFF);Opcion=1;visual=1;
         }
      } 
                  
      if(LEFT)//Si oprime boton de Toma.
      {
         delay_ms(700);Menu=20;glcd_fillscreen(OFF);Opcion=1;visual=1;
      }
        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuraci?n de Parametros de Control
      
      if(Opcion>6)
      {Opcion=1;}
      if(Opcion<1)
      {Opcion=6;}
      
         if(Opcion==1)   
            {strcpy(Menus.Letra,"Ganancia");
            displayMenu(Menus.Letra,0,0,1,1);}
         else
            {strcpy(Menus.Letra,"Ganancia");
            displayMenu(Menus.Letra,0,0,0,1);}
         
         if(Opcion==2)   
            {strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,10,1,1);}
         else
            {strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,10,0,1);}
            
         if(Opcion==3)   
            {strcpy(Menus.Letra,"Ajuste +");
            displayMenu(Menus.Letra,0,20,1,1);}
         else
            {strcpy(Menus.Letra,"Ajuste +");
            displayMenu(Menus.Letra,0,20,0,1);}   
                        
         if(Opcion==4)   
            {strcpy(Menus.Letra,"Ajuste -");
            displayMenu(Menus.Letra,0,30,1,1);}
         else
            {strcpy(Menus.Letra,"Ajuste -");
            displayMenu(Menus.Letra,0,30,0,1);}     
            
         if(Opcion==5)   
            {strcpy(Menus.Letra,"Media Movil");
            displayMenu(Menus.Letra,0,40,1,1);}
         else
            {strcpy(Menus.Letra,"Media Movil");
            displayMenu(Menus.Letra,0,40,0,1);}     
            
         if(Opcion==6)   
            {strcpy(Menus.Letra,"Coeficiente");
            displayMenu(Menus.Letra,0,50,1,1);}
         else
            {strcpy(Menus.Letra,"Coeficiente");
            displayMenu(Menus.Letra,0,50,0,1);}        
      
      if(UP)//Si oprime hacia arriba
         {  if(Flanco == 0) 
               {Flanco = 1;delay_ms(30);Opcion--;visual=1;
               }
         }
            else
               {Flanco = 0;}
                  
         if(DOWN)//Si oprime hacia abajo
         {  if(Flanco2 == 0) 
               {Flanco2 = 1;delay_ms(30);Opcion++;visual=1;
               }     
         }
            else
               {Flanco2 = 0;}  
               
       if(RIGHT)//Si oprime SET
         {  if(Flanco1 == 0) 
               {Flanco1 = 1;delay_ms(30);Menu=Opcion+100;Opcion=1;glcd_fillscreen(OFF);visual=1;
               }
         }
            else
               {Flanco1 = 0;}  
        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==101){ // Menu de Ajuste de Ganancia
      
      if(Ganancia>40)
      {
         Ganancia=1;
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
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Ganancia>1)
            {
               delay_ms(30);Ganancia-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(20,(int8)Ganancia);
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
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Tciclo>1)
            {
               delay_ms(30);Tciclo-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(30,(int8)Tciclo);
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
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Ajuste>0)
            {
               delay_ms(30);Ajuste-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(40,Ajuste);
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
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Ajusten>0)
            {
               delay_ms(30);Ajusten-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(50,Ajusten);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==105){ // Menu de Ajuste de Media Movil
      
      if(MediaMovil>80)
      {
         MediaMovil=80;
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
            if(MediaMovil>80)
            {
               MediaMovil=80;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);MediaMovil+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(MediaMovil>1)
            {
               delay_ms(30);MediaMovil-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(60,MediaMovil);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==106){ // Menu de Ajuste de Coeficiente Regulador
      
      if(Coeficiente>250)
      {
         Coeficiente=100;
      }
      
      if(visual==1)
      {
         glcd_fillScreen(OFF);
         strcpy(Menus.Letra,"Coeficiente");
         displayMenu(Menus.Letra,0,0,1,2);
         
         displayTemperatura(Coeficiente,30,30,0,3);
         
         visual=0;
      }
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Coeficiente>250)
            {
               Coeficiente=250;delay_ms(100);visual=1;
            }
            else
            {
               delay_ms(30);Coeficiente+=1.0;visual=1;
            }
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Coeficiente>1)
            {
               delay_ms(30);Coeficiente-=1.0;visual=1;     
            }
         }
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(30);visual=1;
         } 
                  
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(70,(int8)Coeficiente);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

      if(tiemporeset>=60)
      {
         glcd_init(ON);
         tiemporeset=0;
         if(tiempoventilador<2)
         {
            estado_ventilador=0;
            tiempoventilador++;
         }
         else
         {
            if(tiempoventilador>2)
            {
               tiempoventilador=0;
               estado_ventilador=0;
            }
            else
            {
               estado_ventilador=1;
               tiempoventilador++;
            }
         }
         //estado_ventilador=!estado_ventilador;
      } 

   }
}
