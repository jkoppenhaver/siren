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


S = abs(S(2:fftn*4000/Fs,:)); # magnitude in range 0<f<=4000 Hz.
S = S/max(S(:));           # normalize magnitude so that max is 0 dB.
S = max(S, 10^(-40/10));   # clip below -40 dB.
S = min(S, 10^(-3/10));    # clip above -3 dB.
imagesc (t, f, S);         # display
set (gca, "ydir", "normal"); # put the 'y' direction in the correct directio