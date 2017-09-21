sin_str = '';

for i = 0:255;
    int_val = int16(2047 * sin(2*pi*i/256));
    str_val = int2str(int_val);
    
    if i == 0
        sin_str = strcat(sin_str, '{ ');
    end
    sin_str = strcat(sin_str, str_val);
    if i ~= 255
        sin_str = strcat(sin_str, ', ');
    else
        sin_str = strcat(sin_str, ' };');
    end
end
