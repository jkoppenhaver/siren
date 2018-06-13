%First install the control package from https://octave.sourceforge.io/control/
%Then install the signal package from https://octave.sourceforge.io/signal/index.html
%pkg install 'pkg Name' -forge
pkg load signal

#################################################
##                Wail                         ##
#################################################
figure(1,"name", "Wail", "position",[10 10 950 1000]);
[x,Fs] = audioread("sound_files/Wail.wav");
x1 = (x(:,1)+x(:,2))/2;             # Convert the stereo audio into mono
f = (Fs/length(x1))*(1:length(x1)); # Generate the frequency values
y=fft(x1);                          # Run an FFT on the audio data
mask = f<5000;                      # Only plot frequencies less than 5000 Hz
y=y(mask);
f=f(mask);
subplot(311)                        # Plot the FFT and label the axis
plot(f,abs(y))
title("Whelen wail FFT");
xlabel("Frequency (Hz)");
ylabel("Amplitude");
step = fix(Fs*5/1000);              # 5 ms slice size
window = fix(Fs*40/1000);           # 40 ms window
fftn = 2^nextpow2(window);          # FFT length
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:));       # Limit the range to less than 2000Hz
S = S/max(S(:));                    # Normalize magnitude
subplot(312)                        # Plot the specgram
imagesc (t, f(1:rows(S)), S);
title("Whelen wail spectrogram");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
set (gca, "ydir", "normal"); # Fix the y axis direction
[max_value, max_index] = max(S);    # Identify the indecies of the maximum intensity
max_freqs = f(max_index);           #  values for each time and calculate the max frequency
subplot(313);
plot(t,max_freqs);                  # Plot the frequencies with the maximum intensities
title("Whelen wail frequency vs time");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
print -dpng "-S950,1000" wail.png;
pause(5);
close();

#################################################
##                Yelp                         ##
#################################################
figure(2,"name", "Yelp", "position",[10 10 950 1000]);
[x,Fs] = audioread("sound_files/Yelp.wav");
x1 = (x(:,1)+x(:,2))/2;             # Convert the stereo audio into mono
f = (Fs/length(x1))*(1:length(x1)); # Generate the frequency values
y=fft(x1);                          # Run an FFT on the audio data
mask = f<5000;                      # Only plot frequencies less than 5000 Hz
y=y(mask);
f=f(mask);
subplot(311)                        # Plot the FFT and label the axis
plot(f,abs(y))
title("Whelen yelp FFT");
xlabel("Frequency (Hz)");
ylabel("Amplitude");
step = fix(Fs*5/1000);              # 5 ms slice size
window = fix(Fs*40/1000);           # 40 ms window
fftn = 2^nextpow2(window);          # FFT length
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:));       # Limit the range to less than 2000Hz
S = S/max(S(:));                    # Normalize magnitude
subplot(312)                        # Plot the specgram
imagesc (t, f(1:rows(S)), S);
title("Whelen yelp spectrogram");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
set (gca, "ydir", "normal"); # Fix the y axis direction
[max_value, max_index] = max(S);    # Identify the indecies of the maximum intensity
max_freqs = f(max_index);           #  values for each time and calculate the max frequency
subplot(313);
plot(t,max_freqs);                  # Plot the frequencies with the maximum intensities
title("Whelen yelp frequency vs time");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
print -dpng "-S950,1000" yelp.png;
pause(5);
close();

#################################################
##                Phaser                       ##
#################################################
figure(3,"name", "Phaser", "position",[10 10 950 1000]);
[x,Fs] = audioread("sound_files/Phaser.wav");
x1 = (x(:,1)+x(:,2))/2;             # Convert the stereo audio into mono
f = (Fs/length(x1))*(1:length(x1)); # Generate the frequency values
y=fft(x1);                          # Run an FFT on the audio data
mask = f<5000;                      # Only plot frequencies less than 5000 Hz
y=y(mask);
f=f(mask);
subplot(311)                        # Plot the FFT and label the axis
plot(f,abs(y))
title("Whelen phaser FFT");
xlabel("Frequency (Hz)");
ylabel("Amplitude");
step = fix(Fs*5/1000);              # 5 ms slice size
window = fix(Fs*40/1000);           # 40 ms window
fftn = 2^nextpow2(window);          # FFT length
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*2000/Fs,:));       # Limit the range to less than 2000Hz
S = S/max(S(:));                    # Normalize magnitude
subplot(312)                        # Plot the specgram
imagesc (t, f(1:rows(S)), S);
title("Whelen phaser spectrogram");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
set (gca, "ydir", "normal"); # Fix the y axis direction
f=f(1:rows(S));                     # Use filtering to remove some of the noise
filter_low = 700;                   # Set the min and max frequencies you are trying to find
filter_high = 1700;
mask = f>filter_low & f<filter_high;# Mask only frequencies that fall in that range
f = f(mask);                        # Apply this mask to the data and the frequencies list
S = S(mask,:);
[max_value, max_index] = max(S);    # Identify the indecies of the maximum intensity
max_freqs = f(max_index);           #  values for each time and calculate the max frequency
subplot(313);
plot(t,max_freqs);                  # Plot the frequencies with the maximum intensities
title("Whelen phaser frequency vs time");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
print -dpng "-S950,1000" phaser.png;
pause(5);
close();

#################################################
##                 Horn                        ##
#################################################
figure(4,"name", "Horn", "position",[10 10 950 1000]);
[x,Fs] = audioread("sound_files/Horn_fixed.wav");
x1 = (x(:,1)+x(:,2))/2;             # Convert the stereo audio into mono
f = (Fs/length(x1))*(1:length(x1)); # Generate the frequency values
y=fft(x1);                          # Run an FFT on the audio data
mask = f<1000;                      # Only plot frequencies less than 1000 Hz
y=y(mask);
f=f(mask);
subplot(211)                        # Plot the FFT and label the axis
plot(f,abs(y))
title("Whelen horn FFT");
xlabel("Frequency (Hz)");
ylabel("Amplitude");
step = fix(Fs*5/1000);              # 5 ms slice size
window = fix(Fs*40/1000);           # 40 ms window
fftn = 2^nextpow2(window);          # FFT length
[S, f, t] = specgram(x1, fftn, Fs, window, window-step);
S = abs(S(1:fftn*1000/Fs,:));       # Limit the range to less than 1000Hz
S = S/max(S(:));                    # Normalize magnitude
subplot(212)                        # Plot the specgram
imagesc (t, f(1:rows(S)), S);
title("Whelen horn spectrogram");
xlabel("Time (s)");
ylabel("Frequency (Hz)");
set (gca, "ydir", "normal"); # Fix the y axis direction
print -dpng "-S950,1000" horn.png;
pause(5);
close();