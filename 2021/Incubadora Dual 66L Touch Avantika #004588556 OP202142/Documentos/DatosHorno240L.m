%% Datos Horno Homogenización
%% JP Bioingenieria
clc,clear
 
%Prueba con Set Point 100 grados 
%Ubicacion Sensores: Tres en cada Bandeja
% Con ambos Damper tapados con un trapo
%Se quito trapo de damper de entrada de aire a las 9:19am
% Se abrio Puerta a 9:59am durante 30 segundos
% A las 10:03 alcanzo nuevamente los 100°C del setpoint
    
TemperaturaControl=[17.3 100.7 101.6 99.9 101.1 99.9 100.9 100.3 100.0 100.8 100.1 100.9 99.9 99.5 99.1 100.5 99.8 ...
    99.8 100.6 100.4 96.7 99.3 97.7 99.3 99.7 100.0 99.6 99.4];

TemperaturaHorno=[17.0 98.2 100.1 98.8 100.5 99.3 100.0 100.0 99.6 100.5 99.7 100.7 99.6 99.7 99.4 100.3 100.0 100.1...
    100.6 100.8 95.6 99.6 98.0 99.3 99.9 100.3 99.9 99.7];

tiempo=[812 834 837 839 841 844 846 849 850 852 858 901 908 916 921 928 934 943 947 956 1001 1004 1007 1009 1015 1034 1158 1320];

PtA=[107.0 133.7 136.2 137.0 137.6 137.9 138.4 138.5 138.3 138.6 138.5 138.6 138.8 138.4 138.5 138.5 138.7 138.4 ...
    138.4 138.5 136.9 137.5 137.5 137.9 138.3 138.1 138.3 138.0];  
PtB=[107.1 137.8 138.0 138.1 138.4 138.2 138.7 138.2 138.4 138.3 138.2 138.6 138.6 138.3 138.3 138.5 138.4 138.3 ...
    138.6 138.3 137.5 137.8 137.5 138.1 138.3 138.1 138.3 138.2];
PtC=[107.0 136.1 137.5 138.2 138.7 138.8 139.0 138.8 139.0 139.0 138.9 139.0 139.0 138.8 138.7 138.8 138.9 138.7 ...
    138.8 138.7 137.8 138.1 138.0 138.3 138.7 138.5 138.7 138.7];
PtD=[107.0 133.2 134.6 135.5 136.2 136.7 137.1 137.4 137.3 137.5 137.6 137.8 137.7 137.7 137.6 137.7 137.7 137.6 ...
    138.0 137.8 137.1 137.3 137.4 137.5 137.6 137.8 137.9 137.7];
PtE=[107.1 130.3 132.0 133.6 134.4 135.4 135.8 136.2 136.5 136.7 137.3 137.4 137.4 137.3 137.4 137.5 137.5 137.4 ...
    137.6 137.6 137.0 137.0 137.1 137.4 137.5 137.6 137.7 137.6];

AjusteTemp = TemperaturaControl+0;

Ganancia=[2 5 8 14 18 20 14 18, 18 28 18 18 18 18];
Potencia=[51 60 39 31 26 28 31 23, 21 22 18 22 21 22];

i=0;
for i=1:length(PtA)
TemperaturaA(i)=((PtA(i)-100)/0.385)-0.5;
TemperaturaB(i)=((PtB(i)-100)/0.385)-0.5;
TemperaturaC(i)=((PtC(i)-100)/0.385)-0.5;
TemperaturaD(i)=((PtD(i)-100)/0.385)-0.5;
TemperaturaE(i)=((PtE(i)-100)/0.385)-0.5;
end
figure(1)

plot(tiempo,TemperaturaA,'k',tiempo,TemperaturaB,'g', ...
     tiempo,TemperaturaC,'b',tiempo,TemperaturaD,'--k', ...
     tiempo,TemperaturaE,'r',tiempo,TemperaturaControl,'--r',...
     tiempo,TemperaturaHorno,'--b');
 
legend('TempA','TempB','TempC','TempD',...
       'TempE','TempControl','TempHorno' );
grid on

