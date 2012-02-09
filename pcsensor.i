%module pypcsensor
%{
extern float get_temperature_in_c();
extern float get_temperature_in_f();
extern Temperature *get_temperature_obj();
%}
extern float get_temperature_in_c();
extern float get_temperature_in_f();
extern Temperature *get_temperature_obj();
