void BorraBuffer(void){
RX_Buffer[0]=0;
RX_Buffer[1]=0;
RX_Buffer[2]=0;
RX_Buffer[3]=0;
RX_Buffer[4]=0;
RX_Buffer[5]=0;
RX_Buffer[6]=0;
RX_Buffer[7]=0;
RX_Buffer[8]=0;
RX_Buffer[9]=0;

RX_Buffer2[0]=0;
RX_Buffer2[1]=0;
RX_Buffer2[2]=0;
RX_Buffer2[3]=0;
RX_Buffer2[4]=0;
RX_Buffer2[5]=0;
RX_Buffer2[6]=0;
RX_Buffer2[7]=0;
RX_Buffer2[8]=0;
RX_Buffer2[9]=0;
}

void SendDataDisplay(void){
delay_us(10);
putc(0xFF);
delay_us(10);
putc(0xFF);
delay_us(10);
putc(0xFF);
delay_us(10);
}

short esBisiesto(int8 year) {
     return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

void LimitaDia(){
   if(MesTx>12)
      MesTx=10;
   if(DiaTx>31)
      DiaTx=16;
   if(YearTx>99)
      YearTx=18;   
   if(HoraTx>24)
      HoraTx=11;      
   if(MinutoTx>59)
      MinutoTx=30;      
   
      if(MesTx==2){
         if(esBisiesto(YearTx)){
            if(DiaTx>29){
               DiaTx=29;
               //glcd_fillScreen(OFF);
            }
         }else{
            if(DiaTx>28){
               DiaTx=28;
               //glcd_fillScreen(OFF);
            }
         }
      }else{
         if(MesTx<=7){
            if(MesTx % 2 ==0){
               if(DiaTx>30){
                  DiaTx=30; 
                  //glcd_fillScreen(OFF);
               }
            }
         }else{
            if(MesTx % 2 ==1){
               if(DiaTx>30){
                  DiaTx=30; 
                  //glcd_fillScreen(OFF);
               }
            }
         }
      }
}


void ActualizaRecta(){
   X[y]=Temperatura;
   y++;
   if(y>=2){
      y=0;
      Aumento=X[1]-X[0];
      HorasActual=hEst;
      MinutosActual=mEst;
      TiempoActual=Tiempo-((HorasActual*3600)+(MinutosActual*60)+tinicio);
      PendienteActual=((Setpoint-Temperatura)/TiempoActual);
      DifPendiente=(PendienteActual*20)-Aumento;
      DifPendiente=DifPendiente*5;
   }
}

// Funcion para conversor analogo-digital
float sensores(int x){
float y;set_adc_channel(x);delay_ms(1);y=read_adc();return (y);
}

float LeerPT100(int8 canal,int media)
{
   Prom=0.0; 
   Sensor[canal].V0 = sensores(canal);   //Lectura de Divisor de Voltaje de PT100 con resistencia de 6.18k (+-2%)
   Sensor[canal].V0 = Sensor[canal].V0/Sensor[canal].Gain; // Se elimina la ganancia
   Sensor[canal].V0 = Sensor[canal].V0+Vfijo+ Vref; // Se suma voltaje de divisor fijo + Voltaje de referencia
   //Sensor[canal].BN=(Sensor[canal].V0*(R8+R9))+(1023*R9);
   Sensor[canal].Pt = 6800/((Vdiv/Sensor[canal].V0)-1);
   //Sensor[canal].Pt=Sensor[canal].Pt-Sensor[canal].RPT100; //Ajuste de impedancia de cableado de PT100   
            
   if(Sensor[canal].l>(media-1))
   {Sensor[canal].l=0;}
   
   Sensor[canal].promedio[Sensor[canal].l]=Sensor[canal].Pt;Sensor[canal].l++;
         
   for(Sensor[canal].h=0;Sensor[canal].h<media;Sensor[canal].h++)
   {
      Prom+=Sensor[canal].promedio[Sensor[canal].h];
   } 
   Prom=Prom/media;   
   //Temperature=(Prom-100.0)/(0.385);  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.  
   Temperature=((Prom*Prom)*a)+(Prom*b)+c;  // Ecuación determinada por linealización en Matlab. Revisar este polinomio.
   Temperature+=((float)(Sensor[canal].Ajuste/10.0)-(float)(Sensor[canal].Ajusten/10.0));
   
   return Temperature;
}


void Controla(){
//-------------------------------Control de Calentamiento------------------------------------------------------------//         
   if(!flagRampa){
      if(tiemporecta>=derivativo_ant/2){
         ActualizaRecta();
         tiemporecta=0;
      } 
   }else{
      if(tiemporecta>=10){
         ActualizaRecta();
         tiemporecta=0;
      }
   }
   
   if(flagRampa){
      if(Segmento==0){
         if(flag_calcula==0){
            Tiempo=(rampas[0].Horas*3600)+(rampas[0].Minutos*60);         
            Setpoint=rampas[0].Temperatura;
            Pendiente=((Setpoint-TempeInicial)/Tiempo);
            hEstp=rampas[0].Horas;
            mEstp=rampas[0].Minutos;
            flag_calcula=1;
            delay_ms(10);
            if(Setpoint<Temperatura){
               Enfria=1;
               Ganancia=GananciaR*(Setpoint/150);
               //Ganancia=4;
            }else{
               Enfria=0;
               Ganancia=GananciaR*(Setpoint/150);
               Gananciamax=(0.21)*Setpoint;
            }
         }
      }else{
         if(flag_calcula==0){
            Tiempo=(rampas[Segmento].Horas*3600)+(rampas[Segmento].Minutos*60);         
            Setpoint=rampas[Segmento].Temperatura;
            Pendiente=((rampas[Segmento].Temperatura-rampas[Segmento-1].Temperatura)/Tiempo);
            hEstp=rampas[Segmento].Horas;
            mEstp=rampas[Segmento].Minutos;
            flag_calcula=1;
            delay_ms(10);
            if(Setpoint<Temperatura){
               Enfria=1;
               Ganancia=GananciaR*(Setpoint/150);
               //Ganancia=4;
            }else{
               Enfria=0;
               Ganancia=GananciaR*(Setpoint/150);
               Gananciamax=(0.21)*Setpoint;
            }
         }
      }
   }
  
   if(Pendiente!=0 && flagRampa){// Si Esta realizando una Rampa debe realizar el control de acuerdo a la pendiente de Temperatura. Aprox 10°C * min Maximo Valor de Pendiente.
      TiempoRampa=((hEst*3600)+(mEst*60)+tinicio);
      if(Segmento==0){
         TemperaturaIdeal=(Pendiente*TiempoRampa)+TempeInicial;
      }else{
         TemperaturaIdeal=(Pendiente*TiempoRampa)+rampas[Segmento-1].Temperatura;
      }
      PotenciaEntregada=((TemperaturaIdeal-Temperatura)*100)/penCamara;
// Pendiente determinar si el comportamiento de la mufla es lineal 10°C/min o si tiene un comportamiento diferente.         
      
      if(Pendiente<0){
         // Pendiente revisar rampa para enfriamiento
         if(PotenciaEntregada<0 && PotenciaEntregada>-100.0){
            PotenciaEntregada=abs(PotenciaEntregada);     
         }else if(PotenciaEntregada<-100.0){
            PotenciaEntregada=100.0;
         }else if(PotenciaEntregada>0){
            PotenciaEntregada=0; 
         }
         
         SSR2_off;
         Controlando=0;
         
         if(tCongelamiento>=10800){ // Si el tiempo de congelamiento es igual o mayor a 10800 segundos (3 Horas) Activa el modo Defrost
            flagDefrost=ON;
            tCongelamiento=0;
         }
         
         if(flagDefrost){ // Si debe hacer descongelamiento
            Ventilador_off;
            if(tDefrost<180){
               Compresor_off;
               if(TemperaturaEvaporador<35.0){
                  SSR_on; //Resistencias de evaporador prenden
               }else{
                  SSR_off; //Resistencias de evaporador apagan
               }
            }else if(tDefrost>=180 && tDefrost<=240){
               SSR_off; //Resistencias de evaporador apagan
               Compresor_off;
            }else if(tDefrost>240 && tDefrost<=300){
               if(tApagadoCompresor==0){//4 minutos de espera para arranque de compresor
                  Compresor_on;
                  tApagadoCompresor=240;
               }
            }else{
               flagDefrost=OFF;
               tDefrost=0;
               SSR_off; //Resistencias de evaporador apagan
            }
         }else{ // Si esta en modo Refrigeracion
            Ventilador_on;
            SSR_off; //Resistencias de evaporador apagan
            if(error>0){
               //Compresor_off;
               Valvula1_off;
               Valvula2_on;
            }else{
               if(PotenciaEntregada>=50){
                  if(tApagadoCompresor==0){//4 minutos de espera para arranque de compresor
                     Compresor_on;
                     tApagadoCompresor=240;
                  }
                  Valvula1_on;
                  Valvula2_off;
               }else if(PotenciaEntregada<=30){
                  //Compresor_off;
                  Valvula1_off;
                  Valvula2_on;
               }
            }
         }  
         //-----------------------------------------//
      }else{
         if(PotenciaEntregada>100.0)
            PotenciaEntregada=100.0;
     
         if(PotenciaEntregada<0)
            PotenciaEntregada=0; 
            
         Tpulso=(PotenciaEntregada*Tciclo)/100;
         Tp=(Tpulso/0.005);
         Tc=(Tciclo/0.005);
         Controlando=1;   
      }
   }else{
      if(Enfria==1){ //Debe ejecutar control de refrigeración
         SSR2_off;
         Controlando=0;
         error=Setpoint-Temperatura;      // Cálculo del error ******
         
         if(tCongelamiento>=10800){ // Si el tiempo de congelamiento es igual o mayor a 10800 segundos (3 Horas) Activa el modo Defrost
            flagDefrost=ON;
            tCongelamiento=0;
         }
         
         if(flagDefrost){ // Si debe hacer descongelamiento
            Ventilador_off;
            if(tDefrost<180){
               Compresor_off;
               if(TemperaturaEvaporador<35.0){
                  SSR_on; //Resistencias de evaporador prenden
               }else{
                  SSR_off; //Resistencias de evaporador apagan
               }
            }else if(tDefrost>=180 && tDefrost<=240){
               SSR_off; //Resistencias de evaporador apagan
               Compresor_off;
            }else if(tDefrost>240 && tDefrost<=300){
               if(tApagadoCompresor==0){//4 minutos de espera para arranque de compresor
                  Compresor_on;
                  tApagadoCompresor=240;
               }
            }else{
               flagDefrost=OFF;
               tDefrost=0;
               SSR_off; //Resistencias de evaporador apagan
            }
         }else{ // Si esta en modo Refrigeracion
            Ventilador_on;
            SSR_off; //Resistencias de evaporador apagan
            if(error>0){
               //Compresor_off;
               Valvula1_off;
               Valvula2_on;
            }else{
               if(abs(error)>=Histeresis){
                  if(tApagadoCompresor==0){//4 minutos de espera para arranque de compresor
                     Compresor_on;
                     tApagadoCompresor=240;
                  }
                  Valvula1_on;
                  Valvula2_off;
               }else if(error==0){
                  //Compresor_off;
                  Valvula1_off;
                  Valvula2_on;
               }
            }
         }  
      }else{ //Debe ejecutar control de aumento de temperatura
         Ventilador_on;
         Compresor_off;
         SSR_off;
         Valvula1_on;
         Valvula2_off;
            
         desvio=0.3;
         error=desvio+Setpoint-Temperatura;      // Cálculo del error ******
     
         if(error>=15.0 && error<60 && Aumento<=0.5){
            derivativo_ant=Derivativo*4;
            flag_ajuste=0;
            flag_ajuste2=0;
            Ajusto=10;
            if(Potencia<50.0){
               if(TiempoControl>=derivativo_ant){//80
                  TiempoControl=0;
                  if(Aumento<-0.5){
                     Ganancia+=Integral*1.2;//2
                  }else{
                     if(Aumento<-0.1){
                        Ganancia+=Integral*1.1;//1
                     }else{
                        Ganancia+=Integral;//0.5
                     }
                  }
               }
            }
         }else if(error>=6.0 && error<15 && Aumento<=0.3){
            derivativo_ant=Derivativo*3;
            flag_ajuste=0;
            flag_ajuste2=0;
            Ajusto=10;
            if(Potencia<40.0){
               if(TiempoControl>=derivativo_ant){
                  TiempoControl=0;
                  if(Aumento<-0.5){
                     Ganancia+=Integral;//2
                  }else{
                     if(Aumento<-0.1){
                        Ganancia+=Integral*0.7;//1
                     }else{
                        Ganancia+=Integral*0.5;//0.5
                     }
                  }
               }
            }
         }else if(error>=0.7 && error<6 && Aumento<=0.2){
            derivativo_ant=Derivativo*2;
            flag_ajuste=0;
            flag_ajuste2=0;
            Ajusto=10;
            if(Potencia<30.0){
               if(TiempoControl>=derivativo_ant){
                  TiempoControl=0;
                  if(Aumento<-0.5){
                     Ganancia+=Integral*0.5;//2
                  }else{
                     if(Aumento<-0.1){
                        Ganancia+=Integral*0.3;//1
                     }else{
                        Ganancia+=Integral*0.1;//0.5
                     }
                  }
               }
            }
         }
         
         if(Ganancia<0.01)
            Ganancia=0.01;
      
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
      
         if(Tpulso<0)Tpulso=0;
         if(Tpulso>Tciclo)Tpulso=0;
      
         Tp=(Tpulso/0.005);
         Tc=(Tciclo/0.005);
         Controlando=1; 
      }  
   }    
//--------------------------------------------------------------------------------------------------------------------//   
  
}

void Envio_I2C(direccion, posicion, dato){

   i2c_start();            // Comienzo comunicación
   i2c_write(direccion);   // Dirección del esclavo en el bus I2C
   i2c_write(posicion);    // Posición donde se guardara el dato transmitido
   i2c_write(dato);        // Dato a transmitir
   i2c_stop();             // Fin comunicación
 }

void Lectura_I2C (byte direccion, byte posicion, byte &dato) {

   i2c_start();            // Comienzo de la comunicación
   i2c_write(direccion);   // Dirección del esclavo en el bus I2C
   i2c_write(posicion);    // Posición de donde se leerá el dato en el esclavo
   i2c_start();            // Reinicio
   i2c_write(direccion+1); // Dirección del esclavo en modo lectura
   dato=i2c_read(0);       // Lectura del dato
   i2c_stop();             // Fin comunicación
}

void Carga_Vector(void){
   SlaveA0Tx[0]=(int8)Temperatura;
   SlaveA0Tx[1]=(Temperatura-((int8)Temperatura))*10;
}

void Lee_Vector(void){
   /*Dia=txbuf[0];
   Mess=txbuf[1];
   Year=txbuf[2];
   Hora=txbuf[3];
   Minuto=txbuf[4];
   Segundo=txbuf[5];
   dow=txbuf[6];*/
}

void Envio_Esclavos(void){
   Carga_Vector();
   for(i=0;i<2;i++){
      direccion=0xB0;
      i2c_start();            // Comienzo comunicación
      i2c_write(direccion);   // Dirección del esclavo en el bus I2C
      i2c_write(i);   // Dirección del esclavo en el bus I2C
      //i2c_write(0);    // Posición donde se guardara el dato transmitido
      i2c_write(SlaveA0Tx[i]); // Dato a transmitir
      i2c_stop(); 
      //Envio_I2C(direccion,i,SlaveB0Tx[i]);
   } 
}

int8 extraeDecimal(float data){
   float entero=0;
   float tmp=0.0;
   
   entero=(int8)data;
   tmp=(data-entero)*10;
   return (int8)tmp;
}

float inttofloat(int8 decimal){
   float tmp;
   tmp=decimal;
   return tmp/10;
}

void ConfiguraInterrupciones(){
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN2);
   setup_adc(ADC_CLOCK_DIV_64);
   port_b_pullups(true);
   
   setup_wdt(WDT_ON);
   enable_interrupts(global);
}  

void LeeEEPROM(){
   mEstp=read_eeprom(0);
   delay_ms(10);   
   hEstp=read_eeprom(1);
   delay_ms(10);   
   
   Signo=read_eeprom(79);
   delay_ms(10);   
   entero=read_eeprom(80);
   delay_ms(10);   
   Temporal=read_eeprom(81);
   Temporal=Temporal/10.0;
   Setpoint=entero+Temporal;
   delay_ms(10);   
   
   entero=read_eeprom(82);
   delay_ms(10);   
   Temporal=read_eeprom(83);
   Temporal=Temporal/10.0;
   SetpointH=entero+Temporal;
   delay_ms(10);   
   
   Password[0]=read_eeprom(10);
   delay_ms(10);   
   Password[1]=read_eeprom(11);
   delay_ms(10);   
   Password[2]=read_eeprom(12);
   delay_ms(10);   
   Password[3]=read_eeprom(13);
   delay_ms(10);   
   
   Sensor[0].Ajuste=read_eeprom(21);
   delay_ms(10);
   Sensor[0].Ajusten=read_eeprom(22);
   delay_ms(10);
   Sensor[1].Ajuste=read_eeprom(23);
   delay_ms(10);
   Sensor[1].Ajusten=read_eeprom(24);
   delay_ms(10);
   Ganancia=read_eeprom(30);
   delay_ms(10);   
   Derivativo=read_eeprom(31);
   delay_ms(10);   
   Integral=read_eeprom(32);
   delay_ms(10);   
   Tciclo=read_eeprom(33);
   delay_ms(10);   
   Histeresis=read_eeprom(34);
   delay_ms(10);   
   Histeresis=Histeresis/10.0;
   MediaMovil=read_eeprom(36);
   delay_ms(10);   
   Coeficiente=read_eeprom(37);
   delay_ms(10);   
   Saturacion=read_eeprom(38);
   delay_ms(10);   
   GananciaR=Ganancia;
   
   for(i=0;i<20;i++){//Lee valores almacenados de ciclos personalizados
      rampas[i].TemperaturaInt=read_eeprom(100+(i*5)); 
      rampas[i].TemperaturaDec=read_eeprom(101+(i*5)); 
      rampas[i].Signo=read_eeprom(102+(i*5)); 
      rampas[i].Horas=read_eeprom(103+(i*5)); 
      rampas[i].Minutos=read_eeprom(104+(i*5)); 
     
      if(rampas[i].Signo>20)rampas[i].Signo=10;
      if(rampas[i].Signo==20){
         rampas[i].Temperatura=(rampas[i].TemperaturaInt+inttofloat(rampas[i].TemperaturaDec))*-1;
      }else{
         rampas[i].Temperatura=rampas[i].TemperaturaInt+inttofloat(rampas[i].TemperaturaDec);
      }
      if(rampas[i].Temperatura>60.0 || rampas[i].Temperatura<-12.0)rampas[i].Temperatura=30.0;      
      if(rampas[i].Horas>99.0)rampas[i].Horas=10.0;
      if(rampas[i].Minutos>59.0)rampas[i].Minutos=10.0;
   }
}

void LimitaValores(){
   if(mEstp>59){
      mEstp=10;write_eeprom(0,10);delay_ms(20);
   }
   if(hEstp>99){
      hEstp=10;write_eeprom(1,10);delay_ms(20);
   }
   if(Password[0]>9 || Password[0]<0){
      Password[0]=0;write_eeprom(10,0);delay_ms(20);
   }
   if(Password[1]>9 || Password[1]<0){
      Password[1]=0;write_eeprom(11,0);delay_ms(20);
   }
   if(Password[2]>9 || Password[2]<0){
      Password[2]=0;write_eeprom(12,0);delay_ms(20);
   }
   if(Password[3]>9 || Password[3]<0){
      Password[3]=0;write_eeprom(13,0);delay_ms(20);
   }
   if(Setpoint>60.0 || Setpoint<-12.0){
      Setpoint=30.0;write_eeprom(80,30);delay_ms(20);write_eeprom(81,0);delay_ms(20);
   }
   //if(SetpointH>90.0 || SetpointH<10.0){
   //   SetpointH=50.0;write_eeprom(82,50);delay_ms(20);write_eeprom(83,0);delay_ms(20);
   //}
   if(Ganancia>4 || Ganancia<1){
      Ganancia=2;write_eeprom(30,2);delay_ms(20);
   }
   if(Derivativo>80 || Derivativo<5){
      Derivativo=80;write_eeprom(31,80);delay_ms(20);
   }
   if(Integral>4 || Integral<1){
      Integral=1;write_eeprom(32,1);delay_ms(20);
   }
   if(Tciclo>40 || Tciclo<5){
      Tciclo=5;write_eeprom(33,5);delay_ms(20);
   }
   if(Sensor[0].Ajuste>200){
      Sensor[0].Ajuste=0;
      write_eeprom(21,0);
      delay_ms(10);
   }
   if(Sensor[0].Ajusten>200){
      Sensor[0].Ajusten=0;
      write_eeprom(22,0);
      delay_ms(10);
   }
   if(Sensor[1].Ajuste>200){
      Sensor[1].Ajuste=0;
      write_eeprom(23,0);
      delay_ms(10);
   }
   if(Sensor[1].Ajusten>200){
      Sensor[1].Ajusten=0;
      write_eeprom(24,0);
      delay_ms(10);
   }
   if(MediaMovil>80 || MediaMovil<10){
      MediaMovil=80;write_eeprom(36,80);delay_ms(20);
   }
   if(Histeresis>5.0 || Histeresis==0.0){
      Histeresis=0.2;write_eeprom(34,2);delay_ms(20);
   }
   if(Signo>20){
      Signo=10;write_eeprom(79,10);delay_ms(20);
   }
   if(Signo==20){
      Setpoint=Setpoint*-1;
   }
}

void DelayPuls(){
   delay_ms(2);
}

void CuentaTiempo(){
      if(tinicio>=60){
         tinicio=0;
         mEst+=1.0;
         if(mEst>59){
            mEst=0;
            hEst+=1.0;
         }
      }
}

void LeeDisplay(void){
   if(Dato_Exitoso==5){
         RX_Buffer[0]=RX_Buffer2[0];
         RX_Buffer[1]=RX_Buffer2[1];
         RX_Buffer[2]=RX_Buffer2[2];
         RX_Buffer[3]=RX_Buffer2[3];
         RX_Buffer[4]=RX_Buffer2[4];
         RX_Buffer[5]=RX_Buffer2[5];
         RX_Buffer[6]=RX_Buffer2[6];
         RX_Buffer[7]=RX_Buffer2[7];
         RX_Buffer[8]=RX_Buffer2[8];
         RX_Buffer[9]=RX_Buffer2[9];
         
      if(RX_Buffer[3]==0x01){// Pregunta por la pagina en la que esta el display,01 es Contraseña de Acceso
         Menu=0;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x02){//02 es Menu Principal con PantallaPriincipal=0, y con PantallaPrincipal=1 es Ciclo Libre
         Menu=1;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x04){//04 es Tiempo 
         Menu=2;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x05){//05 es Temperatura
         Menu=3;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x06){//06 es Humedad
         Menu=4;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x07){//07 es Test de Componentes
         Menu=6;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x08){//08 es Fecha y hora
         Menu=7;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x09){//09 es Ciclos Personalizados
         Menu=8;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x0f){//0f es Recibe caracteres de contraseña desde display
         
      }else if(RX_Buffer[3]==0x1a){//1a es Menu de Funcionamiento
         Menu=20;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x1b){//1b es Menu de clave correcta
         Menu=15;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x1c){//1c es Menu de clave incorrecta
         Menu=16;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0x2c){//2c es Menu de Configuracion de Parametros
         Menu=100;
         if(MenuAntt!=Menu)
            MenuAntt=Menu;
      }else if(RX_Buffer[3]==0xcc){//cc es Menu de Bienvenida
         Menu=240;
         if(MenuAntt!=240)
            reset_cpu();
      }
   }else{
      for(z=0;z<RX_BUFFER_SIZE;z++){
            //Rx_Buffer[z]=0;
            //Rx_Buffer2[z]=0;
         }
   }
}

void ApagaSalidas(){
   Buzzer_off;
   SSR_off;
   SSR2_off;
   Ventilador_off;
   Compresor_off;  
   Valvula1_off;
   Valvula2_off;
}

void iniciaConteo(){
   if(flagRampa && Pendiente!=0){
      tiempo_esterilizacion=1;
   }else if(flagRampa && Pendiente==0){
      if(flagTemperatura)
         tiempo_esterilizacion=1;
      else
         tiempo_esterilizacion=0;
   }else{
      if(!flagControlT){
         tiempo_esterilizacion=0;
       }else if(flagControlT){
          if(flagTemperatura)
             tiempo_esterilizacion=1;
          else
             tiempo_esterilizacion=0;
       }
   }
}

void controlaTemperatura(){
   if(Enfria==1){
      if(Temperatura<=Setpoint+1.1 && Temperatura>=Setpoint-1.1){
         printf("statusT.txt=\"Controlando\"");
         SendDataDisplay();
         if(!Infinito)
            flagTemperatura=ON;
            //tiempo_esterilizacion=1;
         else
            flagTemperatura=OFF;
            //tiempo_esterilizacion=0;
            Controla();
            flag_tiempocalentamiento=0;
      }else if(Temperatura<Setpoint-1.1){
         Enfria=0;
         Ganancia=GananciaR*(Setpoint/150);
         Gananciamax=(0.21)*Setpoint;
         SSR_off;
         SSR2_off;         
         Controlando=0;
      }else if(Temperatura>Setpoint+1.1){
         flag_tiempocalentamiento=0;
         printf("statusT.txt=\"Enfriando\"");
         SendDataDisplay();
         //tiempo_esterilizacion=0;
         flagTemperatura=OFF;
         Controla();
      }
   }else{  
      if(Temperatura>=Setpoint-1.1 && Temperatura<=Setpoint+1.1){
         printf("statusT.txt=\"Controlando\"");
         SendDataDisplay();
         if(!Infinito)
            //tiempo_esterilizacion=1;
            flagTemperatura=ON;
         else
            //tiempo_esterilizacion=0;   
            flagTemperatura=OFF;
            Controla();
            flag_tiempocalentamiento=0;
      }else if(Temperatura>Setpoint+1.1){
         Enfria=1;
         Ganancia=GananciaR*(Setpoint/150);
         SSR_off;
         SSR2_off;
         Controlando=0;
      }else if(Temperatura<Setpoint-1.1){
         flag_tiempocalentamiento=1;
         printf("statusT.txt=\"Calentando\"");
         SendDataDisplay();
         //tiempo_esterilizacion=0;
         flagTemperatura=OFF;
         Controla();
      }
   }
}

void controlaRampa(){
   if(Enfria==1){
      if(Temperatura<=Setpoint+1.1 && Temperatura>=Setpoint-1.1){
         if(Pendiente!=0){
            printf("statusT.txt=\"Controlando\"");
            SendDataDisplay();
            flagTemperatura=ON;            
         }
         
         if(Pendiente==0){
            printf("statusT.txt=\"Meseta\"");
            SendDataDisplay();
            flagTemperatura=ON;
         }
         Controla();
         flag_tiempocalentamiento=0;
      }else if(Temperatura<Setpoint-1.1){
         Enfria=0;
         Ganancia=GananciaR*(Setpoint/150);
         Gananciamax=(0.21)*Setpoint;
         SSR_off;
         SSR2_off;
         Controlando=0;
      }else if(Temperatura>Setpoint+1.1){
         if(Pendiente!=0){
            printf("statusT.txt=\"Rampa -\"");
            SendDataDisplay();
            flagTemperatura=ON;            
         }
         if(Pendiente==0){
            printf("statusT.txt=\"Enfriando\"");
            SendDataDisplay();
            flagTemperatura=OFF;            
         }
         Controla();
      }
   }else{  
      if(Temperatura>=Setpoint-1.1 && Temperatura<=Setpoint+1.1){
         if(Pendiente!=0){
            printf("statusT.txt=\"Controlando\"");
            SendDataDisplay();
            flagTemperatura=ON;            
         }
         
         if(Pendiente==0){
            printf("statusT.txt=\"Meseta\"");
            SendDataDisplay();
            flagTemperatura=ON;
         }
         Controla();
         flag_tiempocalentamiento=0;
      }else if(Temperatura>Setpoint+1.1){
         Enfria=1;
         Ganancia=GananciaR*(Setpoint/150);
         SSR_off;
         SSR2_off;
         Controlando=0;
      }else if(Temperatura<Setpoint-1.1){
         if(Pendiente!=0){
            printf("statusT.txt=\"Rampa +\"");
            SendDataDisplay();
            flagTemperatura=ON;            
         }
         if(Pendiente==0){
            printf("statusT.txt=\"Calentando\"");
            SendDataDisplay();
            flagTemperatura=OFF;            
         }
         Controla();
      }
   }
}
