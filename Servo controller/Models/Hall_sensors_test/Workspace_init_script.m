clear;
%% Machine Parameters
pole_pairs_num    = 4;        % Number of pole pairs

%% Control Parameters
Tpwm_per    = 1/32000;
T_pwm       = 1/16000;  % Sample time for current control loop [s]
Tspeed      = 1/1000;    % Sample time for speed control loop [s]

Ts_simscape = T_pwm/2;

MaxCurrent = 6;

Vdc = 24;
Deg_to_ADC = 1060/90;
Diff_to_Degps = 1/(Deg_to_ADC*T_pwm);
Speed_exp_filtr_fact = 1-1/256; % Найденный экспениментально коэфициент.  
