function key = freq_to_key(freq)
    % Returns a KEY (0 to 39) corresponding to input FREQ.
    % 
    % Frequency range: 110 (A2) to ~1047 (C6)
    % Keys are from 0 (A2) to 39 (C6)
    
    keySet = 0:39;
    freqSet = 110 * realpow(2^(1/12), keySet);
    freq_diffs = abs(freqSet - freq);
    [~, key] = min(freq_diffs);
end

