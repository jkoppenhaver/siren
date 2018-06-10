

start_freq  = 700;
end_freq    = 1650;
n_steps     = 100;
clock       = 40000000;
prescaler   = 1;
fileName    = "lookup_table";

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
fdisp(file,"const unsigned int LOOKUP_VALUE[]={");
fprintf(file,"\t%.0f,\n",cycles(1:end-1));
fprintf(file,"\t%.0f\n",cycles(end));
fdisp(file,"};\n");
fdisp(file,"#endif");
fclose(file);