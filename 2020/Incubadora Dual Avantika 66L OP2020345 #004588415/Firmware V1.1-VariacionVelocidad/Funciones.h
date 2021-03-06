void ActualizaRecta(){
   X[y]=Temperatura;
   y++;
   if(y>=2){
      y=0;
      Aumento=X[1]-X[0];
   }
}

// Funcion para conversor analogo-digital
float sensores(int8 x){
float y;set_adc_channel(x);delay_ms(1);y=read_adc();return (y);
}

float Leer_Sensor(int8 media){
   Prom=0.0;
   V0 = sensores(0);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
   V0 = V0/Gain; // Se elimina la ganancia
   V0 += 14.82;
   Pt=6800/((1023/V0)-1);
            
   if(l>(media-1))
   {l=0;}
   //promedio[l]=Temperature;l++;
   promedio[l]=Pt;l++;
         
   for(h=0;h<media;h++)
   {
      Prom+=promedio[h];
   } 
   Prom=Prom/media;   
   //Temperature=(Prom-100.0)/(0.385);  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.  
   Temperature=((Prom*Prom)*a)+(Prom*b)+c;  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.
   Ajustef=Ajuste;
   Ajustenf=Ajusten;
   Ajustef=Ajustef/10;
   Ajustenf=Ajustenf/10;
   Temperature+=(Ajustef-Ajustenf);
   //Temperature-=39.0;
   
   //return Prom;
   return Temperature;
}

void ControlaNatural(){
   //if(tiempoApertura>600){
   //   abrio=0;
   //   tiempoApertura=0;
   //   flag_setpoint=0;
   //}
      
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
      if(tiemporecta>=derivativo_ant){
         ActualizaRecta();
         tiemporecta=0;
      }    
      desvio=1.0;
      error=desvio+Setpoint-Temperatura;      // Cálculo del error ******
  
      if(error>=15.0 && error<60 && Aumento<=0.5){
         Derivativo=derivativo_ant*4;
         flag_ajuste=0;
         flag_ajuste2=0;
         Ajusto=10;
         if(Potencia<60.0){
            if(TiempoControl>=Derivativo)//80
            {
               TiempoControl=0;
               if(Aumento<-0.5)
               {
                  Ganancia+=Integral;//2
               }
               else
               {
                  if(Aumento<-0.1)
                  {
                     Ganancia+=Integral*0.3;//1
                  }   
                  else
                  {
                     Ganancia+=Integral*0.1;//0.5
                  }
               }
            }
         }
      }else if(error>=6.0 && error<15 && Aumento<=0.3){
         Derivativo=derivativo_ant*3;
         flag_ajuste=0;
         flag_ajuste2=0;
         Ajusto=10;
         if(Potencia<40.0){
            if(TiempoControl>=Derivativo)//80
            {
               TiempoControl=0;
               if(Aumento<-0.5)
               {
                  Ganancia+=Integral;//2
               }
               else
               {
                  if(Aumento<-0.1)
                  {
                     Ganancia+=Integral*0.3;//1
                  }   
                  else
                  {
                     Ganancia+=Integral*0.1;//0.5
                  }
               }
            }
         }
      }else if(error>=desvio+0.5 && error<10 && Aumento<=0.3){
         Derivativo=derivativo_ant*3;
         flag_ajuste=0;
         flag_ajuste2=0;
         Ajusto=10;
         if(Potencia<30.0){
            if(TiempoControl>=Derivativo)//80
               {
                  TiempoControl=0;
                  if(Aumento<-0.5)
                  {
                     Ganancia+=Integral;//2
                  }
                  else
                  {
                     if(Aumento<-0.1)
                     {
                        Ganancia+=Integral*0.3;//1
                     }   
                     else
                     {
                        Ganancia+=Integral*0.1;//0.5
                     }
                  }
               }
         }
      }
      
      if(Ganancia<0.1)
         Ganancia=0.1;
      
      if(Ganancia>Gananciamax)
         Ganancia=Gananciamax;
   
      if(error<0.0)     // Anti Wind_DOWN    
         tmp=0;
      else
         tmp=error*Ganancia;  // Control Proporcional.
      
      //tmp=(Setpoint-Prom)*Ganancia;  // Control Proporcional.
            
      if(tmp>Setpoint)   // Anti Wind-UP      
         tmp=Setpoint;
                  
      if(tmp<0.0)     // Anti Wind_DOWN    
         tmp=0.0;
   
      Tpulso=(tmp/Setpoint)*Tciclo;
      
      //if(Tpulso<Tpulsomin && flag_setpoint){
      //   Tpulso=Tpulsomin;
      //}
      
      if(error<desvio+0.5 && error>0){
         if(error<=desvio+0.5 && error>0){   
            if(flag_ajuste==0){
               TpulsoMax=Tpulso;
               flag_ajuste=1;  
               Ajusto=5;
            }
         }
         
         if(error<=desvio-0.3 && error>0){   
            if(flag_ajuste2==0){
               Tpulsomin=Tpulso;
               flag_ajuste2=1;           
               Ajusto=5;
            }
         }
         
         if(flag_ajuste2==1){
            if(Tpulso<Tpulsomin){
               Tpulso=Tpulsomin;
            }
         }
         if(flag_ajuste==1){   
            if(Tpulso>TpulsoMax){
               Tpulso=TpulsoMax;
            } 
         }
      }
      
      if(error<desvio-0.4 && error>0){
         sobrepaso=1;
         Tpulso=Tpulsomin/2;
         if(tSobrepaso>300){
            Tpulsomin-=0.1;
            tSobrepaso=0;
         }
      }else{
         sobrepaso=0;
         tSobrepaso=0;
      }
      
      if(Tpulso<0)Tpulso=0;
      if(Tpulso>Tciclo)Tpulso=0;
      
      Tp=(Tpulso/0.005);
      Tc=(Tciclo/0.005);
      
      Controlando=1;     
   
//--------------------------------------------------------------------------------------------------------------------//     
}

void ControlaForzada(){
      Derivativo=derivativo_ant;
      
//-------------------------------Control de Calentamiento------------------------------------------------------------//
      if(tiemporecta>=(Derivativo)){
         ActualizaRecta();
         tiemporecta=0;
      }    

      desvio=0.5;
      error=desvio+Setpoint-Temperatura;      // Cálculo del error ******
  
      if(error>=15.0 && error<60 && Aumento<=0.5){
         Derivativo=derivativo_ant*2;
         flag_ajuste=0;
         flag_ajuste2=0;
         Ajusto=10;
         if(Potencia<50.0){
            if(TiempoControl>=Derivativo)//80
            {
               TiempoControl=0;
               if(Aumento<-0.5)
               {
                  Ganancia+=Integral+1;//2
               }
               else
               {
                  if(Aumento<-0.1)
                  {
                     Ganancia+=Integral;//1
                  }   
                  else
                  {
                     Ganancia+=Integral-0.5;//0.5
                  }
               }
            }
         }
      }else if(error>=6.0 && error<15 && Aumento<=0.3){
         Derivativo=derivativo_ant*2;
         flag_ajuste=0;
         flag_ajuste2=0;
         Ajusto=10;
         if(Potencia<40.0){
            if(TiempoControl>=Derivativo)//80
            {
               TiempoControl=0;
               if(Aumento<-0.5)
               {
                  Ganancia+=Integral+1;//2
               }
               else
               {
                  if(Aumento<-0.1)
                  {
                     Ganancia+=Integral;//1
                  }   
                  else
                  {
                     Ganancia+=Integral-0.5;//0.5
                  }
               }
            }
         }
      }else if(error>=0.7 && error<6 && Aumento<=0.2){
         Derivativo=derivativo_ant*2;
         flag_ajuste=0;
         flag_ajuste2=0;
         Ajusto=10;
         if(Potencia<30.0){
            if(TiempoControl>=Derivativo)//80
               {
                  TiempoControl=0;
                  if(Aumento<-0.5)
                  {
                     Ganancia+=Integral+1;//2
                  }
                  else
                  {
                     if(Aumento<-0.1)
                     {
                        Ganancia+=Integral;//1
                     }   
                     else
                     {
                        Ganancia+=Integral*0.5;//0.5
                     }
                  }
               }
         }
      }
      
      if(Ganancia<0.1)
         Ganancia=0.1;
      
      if(Ganancia>Gananciamax)
         Ganancia=Gananciamax;
   
      if(error<0.0)     // Anti Wind_DOWN    
         tmp=0;
      else
         tmp=error*Ganancia;  // Control Proporcional.
      
      //tmp=(Setpoint-Prom)*Ganancia;  // Control Proporcional.
            
      if(tmp>Setpoint)   // Anti Wind-UP      
         tmp=Setpoint;
                  
      if(tmp<0.0)     // Anti Wind_DOWN    
         tmp=0.0;
   
      Tpulso=(tmp/Setpoint)*Tciclo;
      
      //if(Tpulso<Tpulsomin && flag_setpoint){
      //   Tpulso=Tpulsomin;
      //}
      
      /*
      if(error<1.5){
         if(error<=2.3){   
            if(flag_ajuste==0){
               TpulsoMax=Tpulso;
               flag_ajuste=1;  
               Ajusto=5;
            }
         }
         
         if(error<=0.8){   
            if(flag_ajuste2==0){
               Tpulsomin=Tpulso;
               flag_ajuste2=1;           
               Ajusto=5;
            }
         }
         
         if(flag_ajuste2==1){
            if(Tpulso<Tpulsomin){
               Tpulso=Tpulsomin;
            }
         }
         //if(flag_ajuste==1){   
         //   if(Tpulso>TpulsoMax){
         //      Tpulso=TpulsoMax;
         //   } 
         //}
      }
      
      if(error<0.5){
         sobrepaso=1;
         Tpulso=Tpulsomin/2;
         if(tSobrepaso>300){
            Tpulsomin-=0.1;
            tSobrepaso=0;
            if(Tpulsomin<0)Tpulsomin=0;
         }
      }else{
         sobrepaso=0;
         tSobrepaso=0;
      }
      */
      if(Tpulso<0)Tpulso=0;
      if(Tpulso>Tciclo)Tpulso=0;
      
      Tp=(Tpulso/0.005);
      Tc=(Tciclo/0.005);
      
      Controlando=1;     
   
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

void displayTemps(char palabra[30], float digito, int x, int y, int sombreado, int tamano)
{
   char voltage[9];
   
   if(sombreado==1)
   {
      sprintf(voltage, "%03.1f", digito);
      glcd_rect(0, y-2, 127, y+(tamano*8), YES, ON);
      strcpy(Menus.Letra,palabra);
      glcd_text57(x+84, y+1, voltage, tamano, OFF);
      glcd_text57(x+1, y+1, Menus.Letra, tamano, OFF);
      strcpy(Menus.Letra,"C");
      glcd_text57(x+120, y+1, Menus.Letra, tamano, OFF);
      glcd_circle(x+115,y-1,1,NO,OFF);
   }
   if(sombreado==0)
   {
      sprintf(voltage, "%03.1f", digito);
      glcd_rect(0, y-2, 127, y+(tamano*8), YES, OFF);
      strcpy(Menus.Letra,palabra);
      glcd_text57(x+84, y+1, voltage, tamano, ON);
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

void displayTT(float adc,int x,int y,int w) {
   char voltage[9];    
   sprintf(voltage, "%03.1f",adc); // Converts adc to text
   glcd_rect(x, y, x+(w*27), y+(w*8), YES, OFF);
   /*glcd_rect(x, y, x+53, y+15, YES, ON);*/glcd_text57(x, y, voltage, w, ON);
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

   
void Limpia(int x1,int x2,int y){
   glcd_rect(x1, y, x2, y+7, YES, OFF);
}

void displayT(float adc,int x,int y,int w) {
   char voltage[9];    
   sprintf(voltage, "%03.1f",adc); // Converts adc to text
   glcd_rect(x, y, x+(w*27), y+(w*7), YES, OFF);
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

void MensajeAlmacenado(){
   glcd_fillScreen(OFF);
   strcpy(Menus.Letra,"Valor");
   displayMenu(Menus.Letra,30,0,0,2);
   strcpy(Menus.Letra," Almacenado");
   displayMenu(Menus.Letra,0,30,0,2);
   glcd_update();
   delay_ms(2000);
   glcd_fillScreen(OFF);
}

void ConfiguraInterrupciones(){
   delay_ms(1000);     
   enable_interrupts(INT_EXT_L2H);
   setup_timer_0(RTCC_8_BIT|RTCC_DIV_256);
   enable_interrupts(INT_TIMER0);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0);
   setup_adc(ADC_CLOCK_DIV_64 );
   set_timer0(0);
   enable_interrupts(global);
}

void MensajeBienvenida(){
   glcd_init(ON);             //Inicializa la glcd
   glcd_fillScreen(OFF);      //Limpia la pantalla
   strcpy(Menus.Letra,"JP Inglobal");
   displayMenu(Menus.Letra,0,0,0,2);     
   strcpy(Menus.Letra,"   Incubadora");
   displayMenu(Menus.Letra,0,20,0,1);       
   strcpy(Menus.Letra,"     Digital  ");
   displayMenu(Menus.Letra,0,30,0,1);
   strcpy(Menus.Letra,"   66 Lts");
   displayMenu(Menus.Letra,0,45,0,2);
   delay_ms(1000);
   glcd_fillScreen(OFF);      //Limpia la pantalla
   
   for(i=0;i<80;i++){
      Temperatura=Leer_Sensor(MediaMovil);
   }
}

void LeeEEPROM(){
   Testerilizacionp[0]=read_eeprom(0);
   delay_ms(20);   
   Testerilizacionp[1]=read_eeprom(1);
   delay_ms(20);   
   Testerilizacionp[2]=read_eeprom(2);
   delay_ms(20);
   Testerilizacionp[3]=read_eeprom(3);
   delay_ms(20);   
   Password[0]=read_eeprom(4);
   delay_ms(20);   
   Password[1]=read_eeprom(5);
   delay_ms(20);   
   Password[2]=read_eeprom(6);
   delay_ms(20);   
   Password[3]=read_eeprom(7);
   delay_ms(20);   

   entero=read_eeprom(8);
   delay_ms(20);   
   Temporal=read_eeprom(9);
   Temporal=Temporal/10.0;
   Temperatura1=entero+Temporal;
   delay_ms(20);   
   
   entero=read_eeprom(10);
   delay_ms(20);   
   Temporal=read_eeprom(11);
   Temporal=Temporal/10.0;
   Temperatura2=entero+Temporal;
   delay_ms(20);   
   
   entero=read_eeprom(12);
   delay_ms(20);   
   Temporal=read_eeprom(13);
   Temporal=Temporal/10.0;
   Temperatura3=entero+Temporal;
   delay_ms(20);   
   
   
   Ganancia=read_eeprom(14);
   delay_ms(10);
   Integral=read_eeprom(15);
   delay_ms(10);
   Derivativo=read_eeprom(16);
   delay_ms(10);
   Tciclo=read_eeprom(17);
   delay_ms(10);
   Ajuste=read_eeprom(18);
   delay_ms(10);
   Ajusten=read_eeprom(19);
   delay_ms(10);
   //Desvio=read_eeprom(23);
   MediaMovil=read_eeprom(20);
   delay_ms(10); 
   
   Temporal=read_eeprom(22);
   Temporal=Temporal/10.0;
   delay_ms(10);
   Gain=read_eeprom(21)+Temporal;
   delay_ms(10);
   R1=make16(read_eeprom(24),read_eeprom(23));
   delay_ms(10);
   Temporal=read_eeprom(26);
   Temporal=Temporal/10.0;
   delay_ms(10);
   RPT100=read_eeprom(25)+Temporal;
   delay_ms(10);
   Source=read_eeprom(27);
   delay_ms(10);
   Vel1=read_eeprom(28);
   delay_ms(10);
}

void LimitaValores(){
   if(Testerilizacionp[0]>9 || Testerilizacionp[0]<0){
      Testerilizacionp[0]=0;write_eeprom(0,0);delay_ms(20);
   }
   if(Testerilizacionp[1]>9 || Testerilizacionp[1]<0){
      Testerilizacionp[1]=0;write_eeprom(1,0);delay_ms(20);
   }
   if(Testerilizacionp[2]>9 || Testerilizacionp[2]<0){
      Testerilizacionp[2]=0;write_eeprom(2,0);delay_ms(20);
   }
   if(Testerilizacionp[3]>9 || Testerilizacionp[3]<0){
      Testerilizacionp[3]=0;write_eeprom(3,0);delay_ms(20);
   }
   if(Password[0]>9 || Password[0]<0){
      Password[0]=0;write_eeprom(4,0);delay_ms(20);
   }
   if(Password[1]>9 || Password[1]<0){
      Password[1]=0;write_eeprom(5,0);delay_ms(20);
   }
   if(Password[2]>9 || Password[2]<0){
      Password[2]=0;write_eeprom(6,0);delay_ms(20);
   }
   if(Password[3]>9 || Password[3]<0){
      Password[3]=0;write_eeprom(7,0);delay_ms(20);
   }
   if(Temperatura1>105.0){
      Temperatura1=30.0;write_eeprom(8,30);delay_ms(20);write_eeprom(9,0);delay_ms(20);
   }
   if(Temperatura2>105.0){
      Temperatura2=30.0;write_eeprom(10,30);delay_ms(20);write_eeprom(11,0);delay_ms(20);
   }
   if(Temperatura3>105.0){
      Temperatura3=30.0;write_eeprom(12,30);delay_ms(20);write_eeprom(13,0);delay_ms(20);
   }
   if(Ganancia>4){
      Ganancia=2;write_eeprom(14,2);delay_ms(20);
   }
   if(Integral>4){
      Integral=1;write_eeprom(15,1);delay_ms(20);
   }
   if(Derivativo>40){
      Derivativo=20;write_eeprom(16,20);delay_ms(20);
   }
   if(Tciclo>10){
      Tciclo=5;write_eeprom(17,5);delay_ms(20);
   }
   if(Ajuste>100){
      Ajuste=0;write_eeprom(18,0);delay_ms(20);
   }
   if(Ajusten>100){
      Ajusten=0;write_eeprom(19,0);delay_ms(20);
   }
   if(MediaMovil>80){
      MediaMovil=80;write_eeprom(20,80);delay_ms(20);
   }
   if(Gain>110 || Gain<100){
      Gain=105.9;write_eeprom(21,105);delay_ms(20);write_eeprom(22,9);delay_ms(20);
   }
   if(R1>6400){
      R1=6200;write_eeprom(23,0x18);delay_ms(20);write_eeprom(24,0x38);delay_ms(20);
   }
   if(RPT100>2.0){
      RPT100=0.5;write_eeprom(25,0);delay_ms(20);write_eeprom(26,5);delay_ms(20);
   }
   if(Source>1){
      Source=1;write_eeprom(27,1);delay_ms(20);
   }
   if(Vel1>10){
      Vel1=10;write_eeprom(28,10);delay_ms(20);
   }
   gananciaAnt=Ganancia;
}

void DelayPuls(){
   delay_ms(2);
}

short PidePassword(int8 MenuAnt, int8 MenuActual, int8 MenuPost, int8 clave0,int8 clave1, int8 clave2,int8 clave3,short cambia){
      if(n==0)
      {
         displayContrasena(Contrasena[0],20,30,1);displayContrasena(Contrasena[1],40,30,0);
         displayContrasena(Contrasena[2],60,30,0);displayContrasena(Contrasena[0],80,30,0);
         if(!cambia){
            strcpy(Menus.Letra,"Clave");
            displayMenu(Menus.Letra,20,0,1,3);
         }else{
            strcpy(Menus.Letra,"Clave Nueva");
            displayMenu(Menus.Letra,0,0,1,2);  
         }
         n=1;
      }
      
      if(UP){
         if(Flanco == 0){
            Flanco = 1;DelayPuls();
            Buzzer_on;
            delay_ms(50);
            Buzzer_off;
            for(i=1;i<=4;i++){
               if(unidad==i){
                  Contrasena[i-1]++;
                  if(Contrasena[i-1]>9){
                     Contrasena[i-1]=0;
                  }
                  displayContrasena(Contrasena[i-1],i*20,30,1);
               }else{
                  displayContrasena(Contrasena[i-1],i*20,30,0);
               }
            }
         }
      }else{
         Flanco = 0;
      }
            
      if(DOWN){
         if(Flanco2 == 0){
            Flanco2 = 1;DelayPuls();
            Buzzer_on;
            delay_ms(50);
            Buzzer_off;
            for(i=1;i<=4;i++){
               if(unidad==i){
                  Contrasena[i-1]--;
                  if(Contrasena[i-1]<0){
                     Contrasena[i-1]=9;
                  }
                  displayContrasena(Contrasena[i-1],i*20,30,1);
               }else{
                  displayContrasena(Contrasena[i-1],i*20,30,0);
               }
            }      
         }     
      }else{
         Flanco2 = 0;
      }
            
      if(RIGHT){
         if(Flanco1 == 0){
            Flanco1 = 1;DelayPuls();unidad++;
            Buzzer_on;
            delay_ms(50);
            Buzzer_off;
            for(i=1;i<=4;i++){
               if(unidad==i){
                  displayContrasena(Contrasena[i-1],i*20,30,1);
               }else{
                  displayContrasena(Contrasena[i-1],i*20,30,0);
               }
            }
         }
      }else{
         Flanco1 = 0;
      }
      
      if(LEFT){
         delay_ms(100);
         if(LEFT){
            Buzzer_on;
            delay_ms(50);
            Buzzer_off;
            Menu=MenuAnt;glcd_fillScreen(OFF);n=0;Opcion=1;unidad=1;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
            glcd_fillScreen(OFF);glcd_update();
            return 1;
         }
      }
    
      if(unidad>4){
         glcd_fillScreen(OFF);unidad=1;
         
         if(!cambia){
            if(Contrasena[0]==3&&Contrasena[1]==8&&Contrasena[2]==9&&Contrasena[3]==2 && MenuActual==0){ // Si Ingresa clave para reset general del sistema.
               write_eeprom(4,0);delay_ms(20);write_eeprom(5,0);delay_ms(20);// Reestablece a contraseña de Fabrica y reinicia Programa.
               write_eeprom(6,0);delay_ms(20);write_eeprom(7,0);delay_ms(20);
               reset_cpu();
            }
         
            if((Contrasena[0]==clave0)&&(Contrasena[1]==clave1)&&(Contrasena[2]==clave2)&&(Contrasena[3]==clave3)){
               glcd_fillScreen(OFF);
               strcpy(Menus.Letra,"Clave");
               displayMenu(Menus.Letra,30,0,0,2);
               strcpy(Menus.Letra,"Correcta");
               displayMenu(Menus.Letra,15,30,0,2);
               Menu=MenuPost;glcd_update();delay_ms(1000);glcd_fillScreen(OFF);
               Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
               n=0;
               return 1;
            }else{
               glcd_fillScreen(OFF);
               strcpy(Menus.Letra,"Clave");
               displayMenu(Menus.Letra,30,0,0,2);
               strcpy(Menus.Letra,"Incorrecta");
               displayMenu(Menus.Letra,0,30,0,2);
               unidad=1;glcd_update();delay_ms(1000);Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;glcd_fillScreen(OFF);
               glcd_rect(0, 0, 127, 25, YES, ON);strcpy(Menus.Letra,"Clave");glcd_text57(25, 1, Menus.Letra, 3, OFF);displayContrasena(Contrasena[0],20,30,1);
               displayContrasena(Contrasena[1],40,30,0);displayContrasena(Contrasena[2],60,30,0);displayContrasena(Contrasena[0],80,30,0);
               Menu=MenuAnt;glcd_fillScreen(OFF);n=0;
               return 0;
            }
         }else{
            Opcion=1;unidad=1;
            write_eeprom(4,Contrasena[0]);
            delay_ms(50);   
            write_eeprom(5,Contrasena[1]);
            delay_ms(50);   
            write_eeprom(6,Contrasena[2]);
            delay_ms(50);   
            write_eeprom(7,Contrasena[3]);
            delay_ms(1000);glcd_fillScreen(OFF);Menu=MenuAnt;
            n=0;
            Contrasena[0]=0;Contrasena[1]=0;Contrasena[2]=0;Contrasena[3]=0;
            MensajeAlmacenado();
            return 0;
         }
      }
}

void CuentaTiempo(){
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
}
