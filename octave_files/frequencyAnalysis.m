%First install the control package from https://octave.sourceforge.io/control/
%Then install the signal package from https://octave.sourceforge.io/signal/index.html
%pkg install 'pkg Name' -forge
pkg load signal
[x,Fs] = audioread("sound_files/wail.wav");
step = fix(5*Fs/1000);     # Slice size is 5 ms
window = fix(40*Fs/1000);  # 40 ms data window
fftn = 2^nextpow2(window); # next highest power of 2
x1 = x(:,1)+x(:,2);
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:)); # magnitude in range 0<f<=4000 Hz.
S = S/max(S(:));           # normalize magnitude so that max is 0 dB.
subplot(3,1,1)
imagesc (t, f(1:rows(S)), S);
set (gca, "ydir", "normal"); # put the 'y' direction in the correct direction

[x,Fs] = audioread("sound_files/yelp.wav");
step = fix(5*Fs/1000);     # Slice size is 5 ms
window = fix(40*Fs/1000);  # 40 ms data window
fftn = 2^nextpow2(window); # next highest power of 2
x1 = x(:,1)+x(:,2);
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:)); # magnitude in range 0<f<=4000 Hz.
S = S/max(S(:));           # normalize magnitude so that max is 0 dB.
subplot(3,1,2)
imagesc (t, f(1:rows(S)), S);
set (gca, "ydir", "normal"); # put the 'y' direction in the correct direction


[x,Fs] = audioread("sound_files/horn.wav");
x1 = x(:,1)+x(:,2);
t = 0:rows(x1):rows(x1)/Fs;
subplot(3,1,3)
imagesc (t, f(1:rows(S)), S);
set (gca, "ydir", "normal"); # put the 'y' direction in the correct direction


print -dpng plot.png