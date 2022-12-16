% Shortest note time (ms)
TEMPORAL_THRESHOLD = 10;

% Recording time (s)
RECORDING_TIME = 10;

% Setup microphone
deviceReader = audioDeviceReader;
setup(deviceReader);

% Write audio output
audio_path = "audio.wav";
fileWriter = dsp.AudioFileWriter(audio_path, "FileFormat", "WAV");

% Record for 10 seconds
disp("Recording for 10 seconds...")

tic
while toc < RECORDING_TIME
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

% Convert to duration
keys_start_stop_duration = reduce_runs(keys);

