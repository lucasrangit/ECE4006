function [x_coord y_coord] = centroid(R) 
% compute the centroid
[Rx Ry] = size(R);

weight_sum_x = zeros(1,Ry);
weight_sum_y = zeros(1,Rx);
    for y = 1:1:Ry
        row_sum = 0;
        for x = 1:1:Rx
            row_sum = row_sum + double(R(x,y));
        end
        weight_sum_x(y) = row_sum;
    end
    for x = 1:1:Rx
        col_sum = 0;
        for y = 1:1:Ry
            col_sum = col_sum + double(R(x,y));
        end
        weight_sum_y(x) = col_sum; 
    end
    num_x = sum( weight_sum_x .* [1:Ry] );
    num_y = sum( weight_sum_y .* [1:Rx] );
    dom = sum( weight_sum_x );
    x_coord = round(num_x/dom);
    y_coord = round(num_y/dom);
