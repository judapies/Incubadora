/// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci?n para Autoclave 80Lts.
// Tiene Men?:Ciclo: Liquidos, Instrumental suave, Textil Instrumental y Caucho.
// Tiene 6 niveles programables para desfogue suave.
// Ing. Juan David Pi?eros.
// Ing. Faiver Humberto Trujillo.
// JP Inglobal. 2014

#include <18F4685.h>
#device adc=10
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOSTVREN,NOPUT,NOCPD,NOWRT,NOIESO,NOFCMEN,NOPBADEN,NOWRTC,NOWRTB,NOEBTR,NOEBTRB,NOCPB,NOXINST,NOLPT1OSC,NOMCLR
#use delay(clock=20000000)
#include "HDM64GS12.c"
#include "GRAPHICS.c"
#include "imagen.h"

// Definici?n de teclado - NO
#define   UP            !input(PIN_A5)
#define   DOWN          !input(PIN_E0)
#define   RIGHT         !input(PIN_E1)
#define   LEFT          !input(PIN_E2)

#define   DOOR          input(PIN_A3)
#define   TERM          input(PIN_A4)


// Definici?n de Salidas
#define   Buzzer_on         output_bit(PIN_B7,1)
#define   Buzzer_off        output_bit(PIN_B7,0)
#define   SSR_on            output_bit(PIN_C2,1)
#define   SSR_off           output_bit(PIN_C2,0)
#define   Rele1_on          output_bit(PIN_C3,1)
#define   Rele1_off         output_bit(PIN_C3,0)
#define   Rele2_on          output_bit(PIN_C4,1)
#define   Rele2_off         output_bit(PIN_C4,0)
#define   Rele3_on          output_bit(PIN_C5,1)
#define   Rele3_off         output_bit(PIN_C5,0)

float promedio[90]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float X[2]={0.0,0.0};
short Desconectado=0;
short Flanco=0,Flanco1=0,Flanco2=0,Infinito=0,Finalizo=0,flag_clear=0;
int8 h=0,l=0,Ajuste=0,Ajusten=0,MediaMovil=10,tiemporecta=0,flag_ganancia=0,Velocidad=3,Lectura2=0,Enfria=0,Controlando=0,Ajusto=10;
short visual=1,Lectura=0,flag_ajuste=0,flag_ajuste2=0;
short flag_clear2=0,flag_clear3=0,flag_tiempocalentamiento=0,flag_display=0,flag_setpoint=0;
int8 y=0,tiemporeset=0,n=0;
int8 Menu=0, unidad=1,i=0,Pausa=0,TiempoControl=0;
int8 Opcion=1,tiempo_esterilizacion=0,tiempo_secado=0,decimal=0,entero=0;
int16 tiempos=0,tiempos3=0,tiempoctrl=0,tinicio=0,tiempocalentamiento=0,tiemposetpoint=0,Tp=0,Tc=0;
float Potencia=0.0,Tpulsomin=0.0,Gananciamax=0.0;
signed int  Testerilizacion[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Testerilizacionp[4]={0,0,0,0};   // Tiempo transcurrido
signed int  Contrasena[4]={0,0,0,0};      // Contrasena de 4 digitos
signed int  Password[4]={0,0,0,0};        // Contrasena Almacenada de 4 digitos
float V0=0.0,Tciclo=10.0,Tpulso=0.0,error=0.0,Prom=0.0,Temperature=0.0,tmp=0.0,Ganancia=2.0,GananciaR=2.0,desvio=0.2,Coeficiente=44.5;
float Aumento=0.0,Temperatura=0.0,Saturacion=10.0,TpulsoMax=0.0,Setpoint=0,Temperatura1=0,Temperatura2=0,Temperatura3=0,Temporal=0.0;

typedef struct{
   char Letra[30];
}MENUU;

MENUU Menus;

#include "Funciones.h"

#int_TIMER1
void temp1s(void){
   set_timer1(40536);
   tiempos++;
   tiempos3++;
   tiempoctrl++;
   
   if(tiempos3>=200)
   {
      Lectura2=1;
   }
   
   if(Controlando==1){
      if(tiempoctrl<Tp) // Si el tiempo es menor o igual al tiempo de control se enciende la resistencia de lo contrario se apaga.
      {
         SSR_on;
      }
      else         
      {
         SSR_off;
      }
      
      if(tiempoctrl>=Tc) // Si el tiempo de control es mayor al tiempo de ciclo se reinicia tiempo de control
      {
         tiempoctrl=0;
      } // tiempoctrl incrementa una unidad cada 5ms, por eso se dividen los tiempos por 0.005.
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

void main()
{
   delay_ms(1000);   
   Buzzer_off;
   SSR_off;
   Rele1_off;
   Rele2_off;
   ConfiguraInterrupciones();
   LeeEEPROM();
   LimitaValores();
   MensajeBienvenida();
   
   if(UP && DOWN)// && RIGHT && LEFT)
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
    if(Menu==0){ //Menu de Contrase?a.
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
            
         if(UP)//Si oprime hacia arriba
         {
            delay_ms(30);Opcion--;visual=1;
         }
                  
         if(DOWN)//Si oprime hacia abajo
         {
            delay_ms(30);Opcion++;visual=1;   
         }
               
         if(RIGHT)//Si oprime SET
         { 
            delay_ms(500);Menu=Opcion+3;Opcion=1;glcd_fillscreen(OFF);visual=1;
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
            displayTemperatura(Temperatura1,50,38,0,2);
         
         if(Opcion==2)
            displayTemperatura(Temperatura2,50,38,0,2);
         
         if(Opcion==3)
            displayTemperatura(Temperatura3,50,38,0,2); 
        
      if(UP)//Si oprime hacia arriba
      {
         Opcion++;
         glcd_rect(40, 35, 85, 60, YES, OFF);
         
      }
            
      if(DOWN)//Si oprime hacia abajo
      {  
         Opcion--;  
         glcd_rect(40, 35, 85, 60, YES, OFF);
         
      }
            
      if(RIGHT)//Si oprime SET
      {  
         if(Opcion==1)
            Setpoint=Temperatura1;
         
         if(Opcion==2)
            Setpoint=Temperatura2;
         
         if(Opcion==3)
            Setpoint=Temperatura3;
         
         if(Setpoint<Temperatura){
            Enfria=1;
            Ganancia=GananciaR*(Setpoint/44.5);
            //Ganancia=4;
         }else{
            Enfria=0;
            Ganancia=GananciaR*(Setpoint/44.5);
            Gananciamax=(0.21)*Setpoint;
         } 
         Menu=20;glcd_rect(15, 18, 30, 53, NO, ON);tiempo_secado=0;glcd_fillScreen(OFF);pausa=0;
      } 
                  
      if(LEFT)//Si oprime boton de Toma.
      {
         delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
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
         
      
         if(UP)//Si oprime hacia arriba
         {  
            if(Opcion==1)
               Temperatura1+=0.1;
            
            if(Opcion==2)
               Temperatura2+=0.1;
               
            if(Opcion==3)
               Temperatura3+=0.1;   
            
         }
               
         if(DOWN)//Si oprime hacia abajo
         {  
            if(Opcion==1)
               Temperatura1-=0.1;
            
            if(Opcion==2)
               Temperatura2-=0.1;
               
            if(Opcion==3)
               Temperatura3-=0.1;   
            
         }
                          
         if(LEFT)//Si oprime boton de Toma.
         {
            delay_ms(700);Menu=1;glcd_fillscreen(OFF);Opcion=1;visual=1;
            
            entero=(int8)Temperatura1;
            decimal=(Temperatura1-entero)*10;
            write_eeprom(80,entero);
            delay_ms(50);   
            write_eeprom(81,decimal);//Guardar valor de Setpoint en eeprom
            delay_ms(50);  
            
            entero=(int8)Temperatura2;
            decimal=(Temperatura2-entero)*10;
            write_eeprom(83,entero);
            delay_ms(50);   
            write_eeprom(84,decimal);//Guardar valor de Setpoint en eeprom
            delay_ms(50);   
            
            entero=(int8)Temperatura3;
            decimal=(Temperatura3-entero)*10;
            write_eeprom(86,entero);
            delay_ms(50);   
            write_eeprom(87,decimal);//Guardar valor de Setpoint en eeprom
            delay_ms(50);   
            MensajeAlmacenado();
         }
         
         if(Temperatura1<15)
            Temperatura1=40;
            
         if(Temperatura2<15)
            Temperatura2=40;
            
         if(Temperatura3<15)
            Temperatura3=40;
            
         if(Temperatura1>40)
            Temperatura1=15;   
            
         if(Temperatura2>40)
            Temperatura2=15;   
            
         if(Temperatura3>40)
            Temperatura3=15;   
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==6){ // Menu de Temporizador
           
         if(unidad<1)
            unidad=4;
         
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
    if(Menu==7){ // Menu de Cambio de Contrase?a
       PidePassword(1,7,91,Password[0],Password[1],Password[2],Password[3],0);
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
    if(Menu==91){ // Menu de Cambio de Contrase?a
       PidePassword(1,91,91,Password[0],Password[1],Password[2],Password[3],1);
    }    
//_--------------------------------------------------------------------------------------------------------------------------------------------------------    

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==20){ //Menu de Funcionamiento
      
      // Muestra imagen de fondo en Display GLCD
      glcd_imagen(5);

      if(UP && DOWN)//Si oprime hacia abajo
      {
         glcd_init(ON);flag_display=!flag_display;         
      }

      if(!TERM && !DOOR && !Finalizo){
         if(Enfria==1){      
            if(Temperatura<=Setpoint+0.5 && Temperatura>=Setpoint-0.5){
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Status:Controlando");
               displayMenu(Menus.Letra,0,55,2,1);
               if(!Infinito)
                  tiempo_esterilizacion=1;
               else
                  tiempo_esterilizacion=0;
               Controla();
               flag_tiempocalentamiento=0;
            }else if(Temperatura<Setpoint-0.5){
               Enfria=0;
               Ganancia=GananciaR*(Setpoint/44.5);
               Gananciamax=(0.21)*Setpoint;
               SSR_off;
               Controlando=0;
            }else{
               flag_tiempocalentamiento=0;
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Status:Enfriando");
               displayMenu(Menus.Letra,0,55,2,1);
               tiempo_esterilizacion=0;
               Controla();
            }
         }else{  
            if(Temperatura>=Setpoint-0.5 && Temperatura<=Setpoint+0.5){
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Status:Controlando");
               displayMenu(Menus.Letra,0,55,2,1);
               if(!Infinito)
                  tiempo_esterilizacion=1;
               else
                  tiempo_esterilizacion=0;   
               Controla();
               flag_tiempocalentamiento=0;
            }else if(Temperatura>Setpoint+0.5){
               Enfria=1;
               Ganancia=GananciaR*(Setpoint/44.5);
               SSR_off;
               Controlando=0;
            }else{
               flag_tiempocalentamiento=1;
               glcd_rect(0, 55, 127, 63, YES, OFF);
               strcpy(Menus.Letra,"Status:Calentando");
               displayMenu(Menus.Letra,0,55,2,1);
               tiempo_esterilizacion=0;
               Controla();
            }
         }
      }
      
      if(DOOR){
         Controlando=0;
         flag_tiempocalentamiento=0;
         Ganancia=GananciaR*(Setpoint/44.5);
         glcd_rect(0, 55, 127, 63, YES, OFF);
         tiempo_esterilizacion=0;
         SSR_off;
         Tpulso=0.0;
         flag_clear2=1;
         flag_ganancia=0;
         strcpy(Menus.Letra,"Status:Puerta Abierta");
         displayMenu(Menus.Letra,0,55,2,1);
      }else{
         if(flag_clear2==1){
            glcd_rect(0, 55, 127, 63, YES, OFF);
            flag_clear2=0;
         }
      }
      
      if(TERM){
         Controlando=0;
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
         Ganancia=GananciaR*(Setpoint/44.5);
         Buzzer_on;
         delay_ms(1000);
         Buzzer_off;
         delay_ms(1000);
      }else{
         if(flag_clear3==1){
            glcd_rect(0, 55, 127, 63, YES, OFF);
            flag_clear3=0;
         }
      }   
        
      if(Lectura==1)// Utilizado para visualizar la lectura de la temperatura
      {                   
         Lectura=0;
         Potencia=(100.0/Tciclo)*Tpulso; 
 //----------------Muestra Temperatura--------------------------
         if(V0<180 || V0>1000)
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
         glcd_circle(74,30,2,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,75,30,2,2);
         
         displayT(Setpoint,30,45,1);
         glcd_circle(55,47,1,NO,ON);
         strcpy(Menus.Letra,"C");displayMenu(Menus.Letra,58,45,2,1);
         strcpy(Menus.Letra,"Temp:");displayMenu(Menus.Letra,0,45,2,1); 
         
         displayTiempo(Potencia,97,42,3,1);strcpy(Menus.Letra,"%");displayMenu(Menus.Letra,115,42,2,1);
         
         if(flag_display==1)
            displayTiempo(Ganancia,66,45,3,1);
            //displayTemperatura(Prom,97,13,0,1);
            //displayT(Tpulsomin,60,46,1);
         else
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
      
      // Si cumplio el tiempo de esterilizaci?n, Muestre ciclo finalizado y detenga control de temperatura.
      if((Testerilizacionp[0]>0)||(Testerilizacionp[1]>0)||(Testerilizacionp[2]>0)||(Testerilizacionp[3]>0)){
         CuentaTiempo();  
         if((Testerilizacion[0]>=Testerilizacionp[0])&&(Testerilizacion[1]>=Testerilizacionp[1])&&(Testerilizacion[2]>=Testerilizacionp[2])&&(Testerilizacion[3]>=Testerilizacionp[3]) && tiempo_esterilizacion==1){
            if(flag_clear==0){
               glcd_fillScreen(OFF);
               //glcd_rect(0, 55, 127, 63, YES, OFF);
               tiempo_esterilizacion=0;
               Pausa=2;
               flag_clear=1;
               SSR_off;
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
         Controlando=0;
         Tpulso=0.0;
         tiempo_esterilizacion=0;
      }
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------
      if(Menu==100){ //Menu de Configuraci?n de Parametros de Control
      
      if(Opcion>5)
         Opcion=1;
      if(Opcion<1)
         Opcion=5;
      
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
            delay_ms(30);
            Opcion--;visual=1;
         }
                  
         if(DOWN)//Si oprime hacia abajo
         {
            delay_ms(30);
            Opcion++;visual=1;
         }
               
         if(RIGHT)//Si oprime SET
         { 
            delay_ms(30);
            Menu=Opcion+100;Opcion=1;glcd_fillscreen(OFF);visual=1;
         }
         
         if(LEFT)//Si oprime SET
         { 
            delay_ms(30);
            Menu=Opcion+100;Opcion=1;glcd_fillscreen(OFF);visual=1;
         }
        
    }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------       

//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==101){ // Menu de Ajuste de Ganancia =2
      
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
            write_eeprom(90,(int8)Ganancia);
            
         }
     }
//_--------------------------------------------------------------------------------------------------------------------------------------------------------


//_--------------------------------------------------------------------------------------------------------------------------------------------------------      
      if(Menu==102){ // Menu de Tciclo=5
      
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
      if(Menu==103){ // Menu de Ajuste de Ajuste+=0
      
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
      if(Menu==104){ // Menu de Ajuste de Ajusten=4
      
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
      if(Menu==105){ // Menu de Ajuste de Media Movil=20
      
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

      if(tiemporeset>=60){
         glcd_init(ON);
         tiemporeset=0;
      } 
      
   }
}
