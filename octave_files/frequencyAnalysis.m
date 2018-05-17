%First install the control package from https://octave.sourceforge.io/control/
%Then install the signal package from https://octave.sourceforge.io/signal/index.html
%pkg install 'pkg Name' -forge
pkg load signal


figure(1,"name","Specgrams","position",[10 10 950 1000]);
nPlots = 4

[x,Fs] = audioread("sound_files/wail.wav");
step = fix(5*Fs/1000);     # Slice size is 5 ms
window = fix(40*Fs/1000);  # 40 ms data window
fftn = 2^nextpow2(window); # next highest power of 2
x1 = (x(:,1)+x(:,2))/2;
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:)); # magnitude in range 0<f<=4000 Hz.
S = S/max(S(:));           # normalize magnitude so that max is 0 dB.
subplot(nPlots,1,1)
imagesc (t, f(1:rows(S)), S);
title("Whelen wail audio clip");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
set (gca, "ydir", "normal"); # put the 'y' direction in the correct direction

[x,Fs] = audioread("sound_files/yelp.wav");
step = fix(5*Fs/1000);
window = fix(40*Fs/1000);
fftn = 2^nextpow2(window);
x1 = (x(:,1)+x(:,2))/2;
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:));
S = S/max(S(:));
subplot(nPlots,1,2)
imagesc (t, f(1:rows(S)), S);
title("Whelen yelp audio clip");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
set (gca, "ydir", "normal");

[x,Fs] = audioread("sound_files/sample_siren_750_1650_sine_lin.wav");
step = fix(5*Fs/1000);
window = fix(40*Fs/1000);
fftn = 2^nextpow2(window);
x1 = (x(:,1)+x(:,2))/2;
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:));
S = S/max(S(:));
subplot(nPlots,1,3)
imagesc (t, f(1:rows(S)), S);
title("Generated linear chirp audio clip");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
set (gca, "ydir", "normal");

[x,Fs] = audioread("sound_files/sample_siren_750_1650_sine_log.wav");
step = fix(5*Fs/1000);
window = fix(40*Fs/1000);
fftn = 2^nextpow2(window);
x1 = (x(:,1)+x(:,2))/2;
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:));
S = S/max(S(:));
subplot(nPlots,1,4)
imagesc (t, f(1:rows(S)), S);
title("Generated logarithmic chirp audio clip");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
set (gca, "ydir", "normal");



print -dpng "-S950,1000" plot1.png

figure(2,"name","FFTs","position",[10 10 950 1000]);
nPlots = 2

[x,Fs] = audioread("sound_files/Horn_fixed.wav");
x1 = (x(:,1)+x(:,2))/2;
f = (Fs/length(x1))*(1:length(x1));
y=fft(x1);
mask = f<5000;
y=y(mask);
f=f(mask);
subplot(nPlots,1,1)
plot(f,abs(y))
title("Whelen horn audio clip (shortened to remove silence)");
xlabel("Frequency (Hz)");
ylabel("Amplitude");

[x,Fs] = audioread("sound_files/sample_horn_370_sawtooth.wav");
x1 = (x(:,1)+x(:,2))/2;
f = (Fs/length(x1))*(1:length(x1));
y=fft(x1);
mask = f<5000;
y=y(mask);
f=f(mask);
subplot(nPlots,1,2)
plot(f,abs(y))
title("Generated 370Hz Sawtooth Tone");
xlabel("Frequency (Hz)");
ylabel("Amplitude");

print -dpng "-S950,1000" plot2.png
