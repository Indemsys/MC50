%% Machine Parameters
p    = 4;        % Number of pole pairs

%% Control Parameters
Ts  = 1/3200000;     % Fundamental sample time              [s]
Tpwm_per = 1/32000;
Tcurrent = 1/16000;  % Sample time for current control loop [s]
Tspeed   = 1/100;    % Sample time for speed control loop [s]
Curr_exp_filtr_fact = 1-1/64;
Speed_exp_filtr_fact = 1-1/2048;

Deg_to_ADC = 1060/90;
Diff_to_Degps = 1/(Deg_to_ADC*Tcurrent);

Vdc = 24;          % Maximum DC link voltage            [V]

Kpw = 0.1;         % Proportional gain speed controller
Kiw = 0.1;         % Integrator gain speed controller

MaxCurrent = 6;
Kpc = 0.1;         % Proportional gain current controller
Kic = 1;           % Integrator gain current controller