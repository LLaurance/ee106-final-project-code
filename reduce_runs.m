function value_start_stop_duration = reduce_runs(vector)
    % Convert an input vector with repeating elements into an n by 4 matrix

    value_start_stop_duration = [0 0 0];    % Value-start-stop
    for index = 1:length(vector)
        current_value = vector(index);
        last_value = value_start_stop_duration(end, 1);

        if current_value == last_value
            value_start_stop_duration(end, 3) = value_start_stop_duration(end, 3) + 1;
        else
            value_start_stop_duration(end + 1, :) = [current_value index index];
        end
    end

    value_start_stop_duration(:, 4) = value_start_stop_duration(:, 3) - value_start_stop_duration(:, 2) + 1;
end