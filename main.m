% Setup microphone
deviceReader = audioDeviceReader;
setup(deviceReader);

% Write audio output
audio_path = "audio.wav";
fileWriter = dsp.AudioFileWriter(audio_path, "FileFormat", "WAV");

% Record for 10 seconds
disp("Recording for 10 seconds...")

tic
while toc < 10
    acquiredAudio = deviceReader();
    fileWriter(acquiredAudio);
end

disp("Recording complete.")

% Release audio device and close output file
release(deviceReader)
release(fileWriter)

% Estimate pitch, show spectrogram
[f0, loc] = spectrogram(audio_path);

% Convert to keys
keys = arrayfun(@(freq) freq_to_key(freq), f0);