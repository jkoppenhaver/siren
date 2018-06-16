#Constants
#Use these to change the parameters of the generated arrays
start_freq    = 700;
end_freq      = 1650;
n_steps       = 300;
clock         = 40000000;
prescaler     = 1;
fileName      = "lookup_table";
riseFallTimes = [0 2 3 0.18 0.18 0.04 0.04]
riseFallTimes = riseFallTimes .*(clock/n_steps);
horn_frequency= 220;

#First calculate the coefficients
#General Form is y =  a*ln(x+e)+b
#y_2 = end_freq
#y_1 = start_freq
#x_2 = n_steps-1
#x_1 = 0
#a = (y_2-y_1)/ln((x_2+e)/(x_1+e))
a = (end_freq-start_freq)/log((n_steps-1+exp(1))/exp(1));
#b = y_1-a*ln(x_1+e)
b = start_freq-a;
x = 0:n_steps-1;
y = a*log(x+exp(1))+b;

cycles = clock./(y.*prescaler);

fileName = [fileName ".h"];
file = fopen(fileName,"w");
fileName = upper(fileName(1:end-2));
temp = ["#ifndef " fileName "_H_\n#define " fileName "_H_\n"]
fdisp(file,temp);
fprintf(file,"const unsigned int LOOKUP_LENGTH = %.0f;\n",length(cycles));
fprintf(file,"const unsigned int HORN_VALUE = %.0f;\n",clock/horn_frequency);
fdisp(file,"const unsigned long RISE_FALL_TIMES[]={");
fprintf(file,"\t%.0f,\n",riseFallTimes(1:end-1));
fprintf(file,"\t%.0f\n",riseFallTimes(end));
fdisp(file,"};\n");
fdisp(file,"const unsigned int LOOKUP_VALUE[]={");
fprintf(file,"\t%.0f,\n",cycles(1:end-1));
fprintf(file,"\t%.0f\n",cycles(end));
fdisp(file,"};\n");
fdisp(file,"#endif");
fclose(file);