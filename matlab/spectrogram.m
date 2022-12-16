function [f0, loc] = spectrogram(audio_path)
    [audioIn, fs] = audioread(audio_path);
    [f0, loc] = pitch(audioIn, fs);
    pitch(audioIn, fs, ...
        "Range", [130 1050])
end
