%%
%%  Datos de Horno de Conveccion Forzada
clc,clear,clf
Temperatura=[25.2 26.4 31.3 38.5 45.8 52.7 59.2 66.7 73.2 80.4 87 94.2 100.3 106.7 113.2 119.3 125.6 ...
    131.7 137.8 142.7 147.2 149.6 150.4 148.6 147.8 147.4 147.5 147.7 148.2 148.7 149 149 149.7 149.4 149.4 149.9 ...
    149.7 149.1 148.4 148.7 150.5 150.1 148.6 150.3 150.4 148.8 149.8 150.4 150.3 148.4 150.3 149.2 149.3 152.1 151.1 148.2 ...
    150.9 152.6 152.7 149.1 148.7 152.7 152.2] ;
Ganancia=[7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 10 13 14 15 16 17 18 18 19 21 22 23 24 27 28 28 29 31 32 34 34 34 ...
    34 36 36 36 40 40 40 44 45 45 45 47 49 49];
plot(Temperatura)
hold on
plot(Ganancia,'r')

%%
clc,clear,clf

Temperatura=[132.3 134 138 142 144.6 145.9 146.6 146.8 146.9 147.1 147.9 148.5 148.8 149 148.9 148.4 148.7 148.9 149.4 ...
    149.5 149.3 149 148.4 148.8 149.2 146.5 149.4 148.9 149 149.7 149.9 149.4 149.1 148.9 149.4 149.6 149.6 149.7 ...
    149.7 149.6 149.2 148.9 149.6 150.2 150.2 149.6 149.3 149.5 149.4 149.3 149.4 149.8 149.6 149.7 149.6 148.9 149.5  149.8...
    150.1 149.6 148.6 149.3] ;
Ganancia=[9 9 9 9 9 9 11 12 13 14 16 16 17 18 20 21 22 22 22 22 22 22 24 24 24 24 25 26 26 26 26 29 29 29 29 29 29 ...
    29 29 30 30 30 30 30 30 30 30 30 30 30 31 31 31 31 31 32 33] ; %%3152022846
plot(Temperatura)
hold on
plot(Ganancia,'r')

%%
clc,clear,clf

Temperatura=[83.5 88.4 93.8 98.9 104.3 109.2 114.8 119.9 125.5 131 136.7 141.6 144.5 146.9 147.9 148 147.9 148.2 148 ...
    148.5 148.7 149.2 149.5 149.6 149.4 149.4 149.1 149.4 149.6 149.7 149.5 149.2 149.6 149.8 149.5 149.1 149.5 150.1 ...
    150.5 149.8 149.1 149.7 150.1 150 149.7 149.8 150.3 150.1 149.7 149.6 149.7 150.1 150 149.7 150 150.1 149.8 150.1 ...
    150.5 149.8 149.3 149.9 150.9 150.2 149.6 149 149.8 150.7 150.5 149.7 150.3 150.7 149.5 149.1 150.2 150.8 150.5 149.5 ...
    149.8 150.2 150 149.1] ;
Ganancia=[7 7 7 7 7 7 7 7 7 7 7 7 7 8 7 9 10 12 13 16 18 19 20 22 23 24 25 27 29 29 29 30 32 32 32 32 33 35 35 35 35 37 38 ...
    38 38 38 38 38 38 38 38 38 38 38 38 38 38 39 39 39 41 42 42 42 42 44 45 45 45 45 45 45 47 48 48 48 48 49 50 50 50];  
plot(Temperatura) 
hold on
plot(Ganancia,'r')

%%


%%
clc,clear,clf

Temperatura=[93.5 94.4 96.5 97.6 97.9 99.2 99.7 99.4 99.2 99.6 99.9 99.9 99.7 99.5 99.7 99.6 99.7 99.7 99.8 99.7 99.5 99.7...
    100 99.9 99.5 99.5 100.1 99.6 99.5 99.6 99.9 100.1 99.8] ;
plot(Temperatura) 
Ganancia=[5 6 7 7 9 10 11 12 14 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15] ;
hold on
plot(Ganancia,'r')


%%  Datos de Horno de Conveccion Forzada
clc,clear,clf
Temperatura=[82.7 88.8 95.5 101.5 107.9 113.5 120.4 125.8 131.3 137.3 143.3 148.4 153.3 158.8 163.5 168.2 171.4 173 173.9 174.7 175.1 ...
    175.4 176.3 177.1 176.1 177.5 178.5 178.4 177.7 177.6 178 178.8 178.9 179.1 178.6 178.9 179 178.9 179.1 179.9 179.6...
    179.6 179. 179.30 179 179.8 180 179.6 179.7 179 179.2 180.2 180.3 179.7 179 179.2 179.8 180.1 180.3 179.2 180.1 180.2 179.7 ...
    179.5 179.9 180.2 179.9 180.4 179.7 179.5 179.8 180.3 179.9 179.5 179 180 180.3 179.7 179.5 180.3 179.2 180 179.4 180 ...
    179.4 179.7 180 180.1 179.8 180.1 180 179.5 180.2 179.2 179.5 180.2 180.3 179.8 179.9 180 180 179. 179.5 179.3 180.4 ...
    179.8 179.3 179.9 179.8 180.2 180.3 179.6 180 180.1 179.6 179.9 179.6 179.9 180.1 ] ;
Ganancia=[7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 9 10 11 13 14 16 19 21 22 24 26 29 31 32 34 35 37 38 40 42 42 42 42 43 44 ...
    45 45 45 45 46 46 47 47 47 47 49 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 48 49 49 ...
    49 49  50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50];  
plot(Temperatura) 
hold on
plot(Ganancia,'r')