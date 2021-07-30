/// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programación para Autoclave 80Lts.
// Tiene Menú:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Piñeros.
// Ing. Faiver Humberto Trujillo.
// JP Inglobal. 2014

#include <18F4685.h>
#device adc=10
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,NOPUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR
#use delay(clock=20000000)
#include "HDM64GS12.c"
#include "GRAPHICS.c"
#include "imagen.h"

// Definición de teclado - NO
#define   UP            !input(PIN_A4)
#define   DOWN          !input(PIN_A5)
#define   RIGHT         !input(PIN_E0)
#define   LEFT          !input(PIN_E1)

#define   DOOR          input(PIN_E2)
#define   TERM          input(PIN_B7)


// Definición de Salidas
#define   SSR_on            output_bit(PIN_C2,0)
#define   SSR_off           output_bit(PIN_C2,1)
#define   Motor1_on         output_bit(PIN_C5,0)
#define   Motor1_off        output_bit(PIN_C5,1)
#define   Motor2_on         output_bit(PIN_C6,0)
#define   Motor2_off        output_bit(PIN_C6,1)
#define   Display_on        output_bit(PIN_C7,1)
#define   Display_off       output_bit(PIN_C7,0)

float promedio[90]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float X[2]={0.0,0.0};
short Desconectado=0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=10,tiemporecta=0,flag_ganancia=0,Lectura2=0,Controlando=0,Ajusto=10,n=0;
short Flanco=0,Flanco1=0,Flanco2=0,visual=1,Lectura=0,flag_ajuste=0,flag_ajuste2=0,flag_clear=0,Infinito=0;
short flag_clear2=0,flag_clear3=0,flag_tiempocalentamiento=0,flag_display=0,flag_setpoint=0,Finalizo=0;
int8 y=0,tiemporeset=0,Source=1;
int8 Menu=0, unidad=1,i=0,Pausa=0,TiempoControl=0;
int8 Opcion=1,tiempo_esterilizacion=0,tiempo_secado=0,decimal=0,entero=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,tinicio=0,tiempocalentamiento=0,tiemposetpoint=0,Tp=0,Tc=0;
float Potencia=0.0,Tpulsomin=0.0,Gananciamax=0.0,Gain=0.0,RPT100=0.0,Integral=0,Derivativo=0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Contrasena[4]={0,0,0,0};      // Contrasena de 4 digitos
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
float V0=0.0,R1=10000.0,Pt,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=2.0,desvio=0.5,Coeficiente=44.5;
float Aumento=0.0,Temperatura=0.0,TpulsoMax=0.0,Setpoint=0,Temperatura1=0,Temperatura2=0,Temperatura3=0,Temporal=0.0;
float a=0.0011,b=2.3302,c=-244.0723;

typedef struct{
   char Letra[30];
}MENUU;

MENUU Menus;

#include "Funciones.h"

#int_TIMER1
void temp1s(void){
   //set_timer1(45612);  // 5 ms 5536 , Posiblemente cargar con 22144 debido a F=12MHz
   set_timer1(40536);
   tiempos++;
   tiempos3++;
   tiempoctrl++;
   
   if(tiempos3>=200)
      Lectura2=1;
   
   if(Controlando==1){
      if(tiempoctrl<Tp) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
         SSR_on;
      else         
         SSR_off;
      
      if(tiempoctrl>=Tc) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
         tiempoctrl=0; // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
   }
    
   if(tiempos>=200){  // 12000 para que incremente cada minuto, 200para que incremente cada segundo.
      tiempos=0;Lectura=1;tiemporeset++;TiempoControl++;tiemporecta++;
   
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
   Motor1_off;
   Motor2_off;
   SSR_off;
   Display_on;
   ConfiguraInterrupciones();
   LeeEEPROM();
   LimitaValores();
   MensajeBienvenida();
   
   if(UP && DOWN)
   {
      delay_ms(500);
      strcpy(Menus.Letra,"Cargando..");
      displayMenu(Menus.Letra,0,30,1,1);
      delay_ms(2000);
      Menu=100;
      glcd_fillScreen(OFF);
   }
   
while(true)
{

      if(Lectura2==1)
      {
         Temperatura=Leer_Sensor(MediaMovil);
         Lectura2=0;
      }
   
      glcd_update();            
//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==0){ //Menu de Contraseña.
         PidePassword(0,0,1,Password[0],Password[1],Password[2],Password[3],0);
      }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==1){ //Menu Principal.      
      
         if(Opcion>4)
            Opcion=1;
            
         if(Opcion<1)
            Opcion=4;
         
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
            
         if(UP){
            DelayPuls();Opcion--;visual=1;
         }
                  
         if(DOWN){
            DelayPuls();Opcion++;visual=1;   
         }
               
         if(RIGHT){ 
            delay_ms(200);Menu=Opcion+3;Opcion=1;glcd_fillscreen(OFF);visual=1;
         }
      }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       
//-Sub-Menus de selección de proceso.

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
            displayTemperatura(Temperatura1,50,38,0,2);
         else if(Opcion==2)
            displayTemperatura(Temperatura2,50,38,0,2); 
         else if(Opcion==3)
            displayTemperatura(Temperatura3,50,38,0,2); 
        
      if(UP){
         Opcion++;
         glcd_rect(40, 35, 85, 60, YES, OFF);
      }
            
      if(DOWN){  
         Opcion--;  
         glcd_rect(40, 35, 85, 60, YES, OFF);
      }
            
      if(RIGHT){  
         if(Opcion==1)
            Setpoint=Temperatura1;          
         else if(Opcion==2)
            Setpoint=Temperatura2; 
         else if(Opcion==3)
            Setpoint=Temperatura3;
         
         Ganancia=Ganancia*(Setpoint/44.5);
         Gananciamax=(0.21)*Setpoint; 
         Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;glcd_fillScreen(OFF);pausa=0;        
         
         if((Testerilizacionp[0]==0)&&(Testerilizacionp[1]==0)&&(Testerilizacionp[2]==0)&&(Testerilizacionp[3]==0))
            Infinito=1;
         else
            Infinito=0;
      } 
                  
      if(LEFT){
         delay_ms(300);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
      } 
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==5){ // Menu de Temperatura
     
         if(Opcion>3)
            Opcion=1;
         if(Opcion<1)
            Opcion=3;
         
         if(Opcion==1)   {
            strcpy(Menus.Letra,"Temperatura 1");
            displayTemps(Menus.Letra,Temperatura1,0,20,1,1);
         }else{
            strcpy(Menus.Letra,"Temperatura 1");
            displayTemps(Menus.Letra,Temperatura1,0,20,0,1);
         }
         
         if(Opcion==2)   {
            strcpy(Menus.Letra,"Temperatura 2");
            displayTemps(Menus.Letra,Temperatura2,0,35,1,1);
         }else{
            strcpy(Menus.Letra,"Temperatura 2");
            displayTemps(Menus.Letra,Temperatura2,0,35,0,1);
         }
            
         if(Opcion==3)   {
            strcpy(Menus.Letra,"Temperatura 3");
            displayTemps(Menus.Letra,Temperatura3,0,50,1,1);
         }else{
            strcpy(Menus.Letra,"Temperatura 3");
            displayTemps(Menus.Letra,Temperatura3,0,50,0,1);
         }
              
               
         if(RIGHT)//Si oprime SET         
            Opcion++;         
      
         strcpy(Menus.Letra,"Temperatura");
         displayMenu(Menus.Letra,0,0,1,2);
         
      
         if(UP){  
            if(Opcion==1)
               Temperatura1+=0.1;
            
            if(Opcion==2)
               Temperatura2+=0.1;
               
            if(Opcion==3)
               Temperatura3+=0.1;   
         }
               
         if(DOWN){  
            if(Opcion==1)
               Temperatura1-=0.1;
            
            if(Opcion==2)
               Temperatura2-=0.1;
               
            if(Opcion==3)
               Temperatura3-=0.1;   
         }
                          
         if(LEFT){
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
            
            entero=(int8)Temperatura1;
            decimal=(Temperatura1-entero)*10;
            write_eeprom(8,entero);
            delay_ms(50);   
            write_eeprom(9,decimal);//Guardar valor de Setpoint en eeprom
            delay_ms(50);  
            
            entero=(int8)Temperatura2;
            decimal=(Temperatura2-entero)*10;
            write_eeprom(10,entero);
            delay_ms(50);   
            write_eeprom(11,decimal);//Guardar valor de Setpoint en eeprom
            delay_ms(50);   
            
            entero=(int8)Temperatura3;
            decimal=(Temperatura3-entero)*10;
            write_eeprom(12,entero);
            delay_ms(50);   
            write_eeprom(13,decimal);//Guardar valor de Setpoint en eeprom
            delay_ms(50); 
            MensajeAlmacenado();
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
            unidad=4;
         
         strcpy(Menus.Letra,"Tiempo");
         displayMenu(Menus.Letra,10,0,1,2);
         
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
      
         if(UP){  
            if(unidad==2){
               if(Testerilizacionp[unidad-1]<5){
                  DelayPuls();Testerilizacionp[unidad-1]++;visual=1;
               }
            }else{
               if(Testerilizacionp[unidad-1]<9){
                  DelayPuls();Testerilizacionp[unidad-1]++;visual=1;
               }
            }
         }
               
         if(DOWN){  
            if(Testerilizacionp[unidad-1]>0){
               DelayPuls();Testerilizacionp[unidad-1]--;visual=1;     
            }
         }
               
         if(RIGHT){  
            DelayPuls();unidad--;visual=1;
         } 
                  
         if(LEFT){
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(0,Testerilizacionp[0]);
            delay_ms(50);   
            write_eeprom(1,Testerilizacionp[1]);
            delay_ms(50);   
            write_eeprom(2,Testerilizacionp[2]);
            delay_ms(50);   
            write_eeprom(3,Testerilizacionp[3]);
            delay_ms(50);   
            MensajeAlmacenado();
            if((Testerilizacionp[0]==0)&&(Testerilizacionp[1]==0)&&(Testerilizacionp[2]==0)&&(Testerilizacionp[3]==0))
               Infinito=1;
            else
               Infinito=0;
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==7){ // Menu de Cambio de Contraseña
         PidePassword(1,7,91,Password[0],Password[1],Password[2],Password[3],0);
      }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==91){ // Menu de Cambio de Contraseña
       PidePassword(1,91,91,Password[0],Password[1],Password[2],Password[3],1);
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==20){ //Menu de Funcionamiento
      
      // Muestra imagen de fondo en Display GLCD
      glcd_imagen(5);
      
      if(UP && DOWN){
         glcd_init(ON);flag_display=!flag_display;
      }
 
      if(!TERM && !DOOR && !Finalizo){
         Motor1_on;
         Motor2_on;
         if(Temperatura>=Setpoint-1.0){
            glcd_rect(0, 55, 127, 63, YES, OFF);
            strcpy(Menus.Letra,"Status:Controlando");
            displayMenu(Menus.Letra,0,55,2,1);
            if(!Infinito)
               tiempo_esterilizacion=1;
               
            Controla();
            flag_tiempocalentamiento=0;
         }else{
            flag_tiempocalentamiento=1;
            glcd_rect(0, 55, 127, 63, YES, OFF);
            strcpy(Menus.Letra,"Status:Calentando");
            displayMenu(Menus.Letra,0,55,2,1);
            tiempo_esterilizacion=0;
            Controla();
         }
      }
         
      if(DOOR){
         Controlando=0;
         Motor1_off;
         Motor2_off;
         flag_tiempocalentamiento=0;
         if(flag_clear2==0){
            Ganancia=read_eeprom(14);
            delay_ms(10);
            Ganancia=Ganancia*(Setpoint/44.5);
         }
         glcd_rect(0, 55, 127, 63, YES, OFF);
         tiempo_esterilizacion=0;
         SSR_off;
         Tpulso=0.0;
         flag_clear2=1;
         flag_ganancia=0;
         strcpy(Menus.Letra,"Status:Puerta Abierta");
         displayMenu(Menus.Letra,0,55,2,1);
      }else{
         if(flag_clear2==1)
         {
            glcd_rect(0, 55, 127, 63, YES, OFF);
            flag_clear2=0;
         }
      }
      
      if(TERM){
         Controlando=0;
         Motor1_off;
         Motor2_off;         
         flag_tiempocalentamiento=0;
         if(flag_clear3==0){
            glcd_rect(0, 55, 127, 63, YES, OFF);
            tiempo_esterilizacion=0;
            SSR_off;
            Tpulso=0.0;
            flag_clear3=1;
            flag_ganancia=0;
         }
         strcpy(Menus.Letra,"Status:Alarma 01");
         displayMenu(Menus.Letra,0,55,2,1);         
      }else{
         if(flag_clear3==1){
            glcd_rect(0, 55, 127, 63, YES, OFF);
            flag_clear3=0;
         }
      }   
        
      if(Lectura==1){                  
         Lectura=0;
         Potencia=(100.0/Tciclo)*Tpulso;
         
 //----------------Muestra Temperatura--------------------------
         if(Source==1){
            if((V0<180 || V0>1000)){
               glcd_rect(15, 29, 69, 45, YES, OFF);
               strcpy(Menus.Letra,"-----");displayMenu(Menus.Letra,15,29,2,2);
               Desconectado=1;
            }else{
               if(Desconectado==1){
                  Desconectado=0;
                  delay_ms(2000);
                  for(i=0;i<80;i++){
                     Temperatura=Leer_Sensor(80);
                  }
               }
            }
         }
         displayT(Temperatura,15,29,2);
         glcd_circle(74,30,2,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,75,30,2,2);
         
         displayT(Setpoint,30,46,1);
         glcd_circle(55,47,1,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,58,45,2,1);
         strcpy(Menus.Letra,"Temp:");displayMenu(Menus.Letra,0,45,2,1); 
         
         displayTiempo(Potencia,97,42,3,1);strcpy(Menus.Letra,"%");displayMenu(Menus.Letra,115,42,2,1);
        
         if(flag_display==1)
            displayTiempo(Ganancia,66,45,3,1);
            //displayTemperatura(Prom,97,13,0,1);
            //displayT(Tpulsomin,60,46,1);
         else
            //displayT(V0,66,45,1);
            glcd_rect(66, 45, 88, 52, YES, OFF);
         
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
      
      // Si cumplio el tiempo de esterilización, Muestre ciclo finalizado y detenga control de temperatura.
      if((Testerilizacionp[0]>0)||(Testerilizacionp[1]>0)||(Testerilizacionp[2]>0)||(Testerilizacionp[3]>0)){
         if((Testerilizacion[0]>=Testerilizacionp[0])&&(Testerilizacion[1]>=Testerilizacionp[1])&&(Testerilizacion[2]>=Testerilizacionp[2])&&(Testerilizacion[3]>=Testerilizacionp[3]) && tiempo_esterilizacion==1){
            if(flag_clear==0){
               glcd_fillScreen(OFF);
               //glcd_rect(0, 55, 127, 63, YES, OFF);
               tiempo_esterilizacion=0;
               Pausa=2;
               flag_clear=1;
               SSR_off;
               Motor1_off;
               Motor2_off;
               Controlando=0;
               Tpulso=0.0;
               flag_ganancia=0;
               Finalizo=1;
            }
            strcpy(Menus.Letra,"Status:Finalizado");
            displayMenu(Menus.Letra,0,55,2,1);
         }
      }
       
      if(Finalizo){
         strcpy(Menus.Letra,"Status:Finalizado");
         displayMenu(Menus.Letra,0,55,2,1);
         SSR_off;
         Motor1_off;
         Motor2_off;
         Controlando=0;
         Tpulso=0.0;
         tiempo_esterilizacion=0;
      }
      
      CuentaTiempo();
      /*if(LEFT)//Si oprime boton de Toma.
      {
         if(Pausa!=2)
         {
            delay_ms(500);Menu=30;glcd_fillscreen(OFF);Opcion=1;visual=1;Pausa=1;
         }
         LEFT=0;
      }*/        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuración de Parametros de Control
      
      if(Opcion>11)
      {Opcion=1;}
      if(Opcion<1)
      {Opcion=11;}
      
      if(Opcion<=6){
         if(Opcion==1)   
            {strcpy(Menus.Letra,"Proporcional");
            displayMenu(Menus.Letra,0,0,1,1);}
         else
            {strcpy(Menus.Letra,"Proporcional");
            displayMenu(Menus.Letra,0,0,0,1);}
         
         if(Opcion==2)   
            {strcpy(Menus.Letra,"Integral");
            displayMenu(Menus.Letra,0,9,1,1);}
         else
            {strcpy(Menus.Letra,"Integral");
            displayMenu(Menus.Letra,0,9,0,1);}
            
         if(Opcion==3)   
            {strcpy(Menus.Letra,"Derivativo");
            displayMenu(Menus.Letra,0,18,1,1);}
         else
            {strcpy(Menus.Letra,"Derivativo");
            displayMenu(Menus.Letra,0,18,0,1);}   
         
         if(Opcion==4)   
            {strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,27,1,1);}
         else
            {strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,27,0,1);}
            
         if(Opcion==5)   
            {strcpy(Menus.Letra,"Ajuste +");
            displayMenu(Menus.Letra,0,36,1,1);}
         else
            {strcpy(Menus.Letra,"Ajuste +");
            displayMenu(Menus.Letra,0,36,0,1);}   
                        
         if(Opcion==6)   
            {strcpy(Menus.Letra,"Ajuste -");
            displayMenu(Menus.Letra,0,45,1,1);}
         else
            {strcpy(Menus.Letra,"Ajuste -");
            displayMenu(Menus.Letra,0,45,0,1);}     
      }
      else{
         if(Opcion==7)   
            {strcpy(Menus.Letra,"Media Movil");
            displayMenu(Menus.Letra,0,0,1,1);}
         else
            {strcpy(Menus.Letra,"Media Movil");
            displayMenu(Menus.Letra,0,0,0,1);}     
            
         if(Opcion==8)   
            {strcpy(Menus.Letra,"Ganancia    ");
            displayMenu(Menus.Letra,0,9,1,1);}
         else
            {strcpy(Menus.Letra,"Ganancia    ");
            displayMenu(Menus.Letra,0,9,0,1);}  
            
         if(Opcion==9)   
            {strcpy(Menus.Letra,"R51         ");
            displayMenu(Menus.Letra,0,18,1,1);}
         else
            {strcpy(Menus.Letra,"R51         ");
            displayMenu(Menus.Letra,0,18,0,1);}     
         
         if(Opcion==10)   
            {strcpy(Menus.Letra,"RPT100 B     ");
            displayMenu(Menus.Letra,0,27,1,1);}
         else
            {strcpy(Menus.Letra,"RPT100 B     ");
            displayMenu(Menus.Letra,0,27,0,1);}     
         
         if(Opcion==11)   
            {strcpy(Menus.Letra,"In Temperatura");
            displayMenu(Menus.Letra,0,36,1,1);}
         else
            {strcpy(Menus.Letra,"In Temperatura");
            displayMenu(Menus.Letra,0,36,0,1);} 
            
            strcpy(Menus.Letra,"            ");
            displayMenu(Menus.Letra,0,45,0,1);
      }   
      
         if(UP)//Si oprime hacia arriba
         {
            delay_ms(10);
            if(Flanco == 0)
               {Flanco = 1;delay_ms(30);Opcion--;visual=1;
               }
         }
            else
               {Flanco = 0;}
                  
         if(DOWN)//Si oprime hacia abajo
         {
            delay_ms(10);
            if(Flanco2 == 0)
               {Flanco2 = 1;delay_ms(30);Opcion++;visual=1;
               }     
         }
            else
               {Flanco2 = 0;}  
               
         if(RIGHT)//Si oprime SET
         {  
            delay_ms(10);
            if(Flanco1 == 0)
               {Flanco1 = 1;delay_ms(30);Menu=Opcion+100;Opcion=1;glcd_fillscreen(OFF);visual=1;
               }
         }
            else
               {Flanco1 = 0;} 
               
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(10);
            delay_ms(700);reset_cpu();         
         }      
        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==101){ // Menu de Ajuste de Ganancia
         if(Ganancia>40)      
            Ganancia=1;
         else if(Ganancia<1)
            Ganancia=40;
         
         if(visual==1){
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"P");
            displayMenu(Menus.Letra,0,0,1,2); 
            displayTemperatura(Ganancia,30,30,0,3);         
            visual=0;
         }
      
         if(UP){  
            DelayPuls();Ganancia+=1.0;visual=1;            
         }
               
         if(DOWN){  
            DelayPuls();Ganancia-=1.0;visual=1;     
         }
               
         if(RIGHT)//Si oprime SET
            visual=1;          
                  
         if(LEFT){
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(14,(int8)Ganancia);delay_ms(20);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==102){ // Menu de Ajuste de Integral   
         if(Integral>40)      
            Integral=1;
         else if(Integral<1)
            Integral=40;
      
         if(visual==1){
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"I");
            displayMenu(Menus.Letra,0,0,1,2);
            displayTemperatura(Integral,30,30,0,3); 
            visual=0;
         }
      
         if(UP){  
            DelayPuls();Integral+=1.0;visual=1; 
         }
               
         if(DOWN){              
            DelayPuls();Integral-=1.0;visual=1;     
         }
               
         if(RIGHT)//Si oprime SET
            visual=1; 
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(15,(int8)Integral);delay_ms(20);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==103){ // Menu de Ajuste de Derivativo
      
         if(Derivativo>200)
            Derivativo=2;
         else if(Derivativo<2)
            Derivativo=200;
      
         if(visual==1){
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"D");
            displayMenu(Menus.Letra,0,0,1,2);         
            displayTemperatura(Derivativo,30,30,0,3);
            visual=0;
         }
      
         if(UP){  
            delay_ms(10);
            DelayPuls();Derivativo+=1.0;visual=1;
         }
               
         if(DOWN){  
            DelayPuls();Derivativo-=1.0;visual=1;     
         }
               
         if(RIGHT)//Si oprime SET
            visual=1; 
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(16,(int8)Derivativo);delay_ms(20);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==104){ // Menu de Tiempo de Ciclo
      
         if(Tciclo>40)
            Tciclo=40;
         else if(Tciclo<1)
            Tciclo=40;
      
         if(visual==1){
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Tciclo");
            displayMenu(Menus.Letra,0,0,1,2);         
            displayTemperatura(Tciclo,30,30,0,3);
            visual=0;
         }
      
         if(UP){  
            DelayPuls();Tciclo+=1.0;visual=1;
         }
               
         if(DOWN){  
            DelayPuls();Tciclo-=1.0;visual=1;     
         }
               
         if(RIGHT)
            visual=1; 
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(17,(int8)Tciclo);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==105){ // Menu de Ajuste de Ajuste+
      
         if(Ajuste>100)
            Ajuste=0;
      
         if(visual==1)
         {
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Ajuste");
            displayMenu(Menus.Letra,0,0,1,2);      
            displayTemperatura(Ajuste,30,30,0,3);
            visual=0;
         }
      
         if(UP){  
            DelayPuls();Ajuste++;visual=1;
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Ajuste>0){
               DelayPuls();Ajuste--;visual=1;     
            }
         }
               
         if(RIGHT)
            visual=1;
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(18,Ajuste);delay_ms(20);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==106){ // Menu de Ajuste de Ajuste -
      
         if(Ajusten>100)
            Ajusten=0;
       
         if(visual==1){
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Ajusten");
            displayMenu(Menus.Letra,0,0,1,2);
            displayTemperatura(Ajusten,30,30,0,3);
            visual=0;
         }
      
         if(UP){  
            DelayPuls();Ajusten++;visual=1;
         }
               
         if(DOWN){  
            if(Ajusten>0){
               DelayPuls();Ajusten--;visual=1;     
            }
         }
               
         if(RIGHT)
            visual=1;
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(19,Ajusten);delay_ms(20);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==107){ // Menu de Ajuste de Media Movil
      
         if(MediaMovil>80)
            MediaMovil=10;
         else if(MediaMovil<10)
            MediaMovil=80;
      
         if(visual==1){
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"Media");
            displayMenu(Menus.Letra,0,0,1,2);
            displayTemperatura(MediaMovil,30,30,0,3);
            visual=0;
         }
      
         if(UP){  
            DelayPuls();MediaMovil++;visual=1;
         }
               
         if(DOWN){  
            DelayPuls();MediaMovil--;visual=1;     
         }
               
         if(RIGHT)
            visual=1;
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(20,(int8)MediaMovil);delay_ms(20);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==108){ // Menu de Ganancia Instrumentacion
      
         if(Gain>50 || Gain<30)
            Gain=40;
      
         strcpy(Menus.Letra,"Ganancia");
         displayMenu(Menus.Letra,0,0,1,2);
         displayTT(Gain,30,30,2);

         if(UP){  
            DelayPuls();
            Gain+=0.1;
         }
               
         if(DOWN){  
            DelayPuls();
            Gain-=0.1;
         }
               
         if(RIGHT)
            visual=1;
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(21,(int8)Gain);
            delay_ms(10);
            Temporal=Gain-((int8)Gain);
            Temporal=Temporal*10;
            write_eeprom(22,(int8)Temporal);
            delay_ms(10);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==109){ // Menu de Ajuste R1
      
         if(R1>6400 || R1<6000)
            R1=6200;

         strcpy(Menus.Letra,"R51");
         displayMenu(Menus.Letra,0,0,1,2);   
         displayTemperatura(R1,30,30,0,3);

         if(UP){  
            DelayPuls();
            R1+=1.0;
            glcd_fillscreen(OFF);
         }
               
         if(DOWN){  
            DelayPuls();
            R1-=1.0;
            glcd_fillscreen(OFF);
         }
               
         if(RIGHT){  
            visual=1;
            glcd_fillscreen(OFF);
         } 
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(23,make8((int16)R1,0));
            delay_ms(10);
            write_eeprom(24,make8((int16)R1,1));
            delay_ms(10);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==110){ // Menu de Ajuste RPT100
      
         if(RPT100>2.0)
            RPT100=0.5;

         strcpy(Menus.Letra,"RPT100");
         displayMenu(Menus.Letra,0,0,1,2);   
         displayTT(RPT100,30,30,2);

         if(UP){  
            DelayPuls();
            RPT100+=0.1;
            glcd_fillscreen(OFF);
         }
               
         if(DOWN){  
            DelayPuls();
            RPT100-=0.1;
            glcd_fillscreen(OFF);
         }
               
         if(RIGHT){  
            visual=1;
            glcd_fillscreen(OFF);
         } 
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(25,(int8)RPT100);
            delay_ms(10);
            Temporal=RPT100-((int8)RPT100);
            Temporal=Temporal*10;
            write_eeprom(26,(int8)Temporal);
            delay_ms(10);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==111){ // Menu de Ajuste de Entrada Analoga de Temperatura
      
         if(visual==1){
            glcd_fillScreen(OFF);
            strcpy(Menus.Letra,"In Tem");
            displayMenu(Menus.Letra,0,0,1,2);
            displayTemperatura(Source,30,30,0,3);
            visual=0;
         }
      
         if(UP){  
            DelayPuls();Source=1;visual=1;
         }
               
         if(DOWN){  
            DelayPuls();Source=0;visual=1;     
         }
               
         if(RIGHT)
            visual=1;
                  
         if(LEFT){
            delay_ms(10);
            delay_ms(700);Menu=100;glcd_fillscreen(OFF);Opcion=1;visual=1;
            write_eeprom(27,Source);delay_ms(20);
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------


      if(tiemporeset>=60 && Menu!=0)
      {
         glcd_init(ON);
         tiemporeset=0;
      } 

   }
}
