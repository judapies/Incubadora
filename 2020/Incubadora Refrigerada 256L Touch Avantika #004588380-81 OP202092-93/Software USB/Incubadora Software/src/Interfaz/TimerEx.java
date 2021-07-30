/*
 * The MIT License
 *
 * Copyright 2017 root.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package Interfaz;

import Comunicacion.Variables;
import HID.HIDComm;
import static Interfaz.CicloPurga.LabelMinutos;
import static Interfaz.CicloPurga.LabelSegundos;
import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author root
 */
public class TimerEx {
    public static Timer timer = new Timer();
    public static int tiempo=0,tiempoapagado=0,apago=0,corriendo=0,tiempooff=0;
    public static int tiemposuspend=0;
    public static TimerTask task = new TimerTask(){  
            @Override
            public void run() {
            
            if(tiemposuspend>=30)    
            {
                try {
                    Runtime.getRuntime().exec("xdotool key ctrl+alt+F2");
                } catch (IOException ex) {
                    Logger.getLogger(TimerEx.class.getName()).log(Level.SEVERE, null, ex);
                }
                tiemposuspend=0;
            }
            else
            {
                tiemposuspend++;
            }
                
            if(corriendo==1)
            {
                System.out.println("Corriendo");
                if(Comunicacion.Hilo.Clave.isVisible())
                {
                    if(tiempoapagado>=30)
                    {
                        Comunicacion.Hilo.Off.setVisible(true);
                        Comunicacion.Hilo.Clave.dispose();
                        apago=1;
                    }
                    tiempoapagado++;
                }
                else{
                    if(!Comunicacion.Hilo.Purga.isVisible())
                    {
                        if(apago==10)
                        {
                            Comunicacion.Hilo.Off.dispose();
                            tiempo=0;
                            apago=0;
                            tiempoapagado=0;
                        }
                        
                        if(!Comunicacion.Hilo.Off.isVisible())
                        {
                            if(tiempo==5)
                            {
                                Comunicacion.Hilo.Clave.setVisible(true);
                                Comunicacion.Hilo.Splash.dispose();
                            }
                            tiempo++;
                        }
                    }
                }
                
                if(Comunicacion.Hilo.Purga.isVisible())
                {
                    if(Variables.VidrioUP==5 && Variables.VidrioDN==5)
                    {
                        Variables.SegundosPurga++;
                        if(Variables.SegundosPurga>59)
                        {
                            Variables.SegundosPurga=0;
                            Variables.MinutosPurga++;
                        }
                
                        LabelMinutos.setText(String.format("%02d",Variables.MinutosPurga));
                        LabelSegundos.setText(String.format("%02d",Variables.SegundosPurga));
                    }
                }
                
                if(Comunicacion.Hilo.Off.isVisible())
                {
                    tiempooff++;
                    if(tiempooff>=300)
                    {
                        Variables.MotorInFlow=false;
                        Variables.MotorDownFlow=false;
                        Variables.LuzBlanca=false;
                        Variables.Toma=false;
                        Variables.LuzUV=false;
                        Variables.tx_msg[0]=(byte)10;
                        Variables.tx_msg[1]=(byte)10;
                        Variables.tx_msg[2]=(byte)10;
                        Variables.tx_msg[3]=(byte)10;
                        Variables.tx_msg[4]=(byte)10;
                        Variables.tx_msg[5]=(byte)10;
                        Variables.tx_msg[6]=(byte)10;
                        Variables.tx_msg[7]=(byte)10;
                        Variables.tx_msg[8]=(byte)10;
                        Variables.tx_msg[9]=(byte)10;
                        Variables.tx_msg[10]=(byte)255;
                        Variables.tx_msg[11]=(byte)255;
                        Variables.tx_msg[12]=(byte)128;
                        HIDComm.EnvioHID((byte)2, Variables.tx_msg,13);
                        
                        try {
                            Runtime.getRuntime().exec("xdotool key ctrl+alt+F7");
                            Runtime.getRuntime().exec("sudo shutdown -h now");
                        } catch (IOException ex) {
                            Logger.getLogger(TimerEx.class.getName()).log(Level.SEVERE, null, ex);
                        }
                    }
                }
                else
                {
                    tiempooff=0;
                }
                
                if(Comunicacion.Hilo.PPurga.isVisible())
                {
                    Variables.SegundosPPurga++;
                    if(Variables.SegundosPPurga>59)
                    {
                        Variables.SegundosPPurga=0;
                        Variables.MinutosPPurga++;
                    }
                
                    Comunicacion.Hilo.PPurga.LabelMinutos.setText(String.format("%02d",Variables.MinutosPPurga));
                    Comunicacion.Hilo.PPurga.LabelSegundos.setText(String.format("%02d",Variables.SegundosPPurga));
                }
                
                if(Variables.estadoUV)
                {
                    Variables.SegundosUV++;
                    if(Variables.SegundosUV>59)
                    {
                        Variables.SegundosUV=0;
                        Variables.MinutosUV++;
                    }
            
                    PantallaPrincipal.LabelMinUV.setText(String.format("%02d",Variables.MinutosUV));
                    PantallaPrincipal.LabelSegUV.setText(String.format("%02d",Variables.SegundosUV));
                }
                
                if(Variables.AlarmaInflow)
                    Variables.tiempoAlarmaIn++;
                
                if(Variables.AlarmaDownflow)
                    Variables.tiempoAlarmaDn++;
                                        
            }
            }
                
        };
    
    public static void IniciaTimer(long tiempo){
        if(corriendo==0)    
        {
            timer.schedule(task, 0, tiempo);
            corriendo=1;
        }
        
        if(corriendo==2)
        {
            corriendo=1;
        }
    }
    
    public static void DetieneTimer(){
        if(corriendo==1)
        {
            //timer.cancel();
            corriendo=2;
        }
    }
}
