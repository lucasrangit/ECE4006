function [skin_mask] = filter_skin(frame)
% Skin Tone Filter
% INPUT: a single RGB frame for skin tone processing
% OUTPUT: binary mask of same size representing skin tone
% for display purposes, temporarily store unsigned int values.

[framex framey framez] = size(frame);
if (framex == 0) || (framey == 0) || (framez == 0)
    disp('Error: Input frame size to filter_skin is zero!');
end
%THRESHOLD VARIABLES
skin_min = 1.2;
skin_max = 1.42;
for y = 1:framey
    for x = 1:framex
        % compute ratios
        RG_ratio = double( frame(x,y,1) ) / double( frame(x,y,2) ) ;
        % create skin pixel mask
        if ( skin_min <= RG_ratio && RG_ratio <= skin_max )
            skin_mask(x,y,1) = uint8(255); 
            skin_mask(x,y,2) = uint8(255); 
            skin_mask(x,y,3) = uint8(255); 
        else
            skin_mask(x,y,1) = uint8(0);
            skin_mask(x,y,2) = uint8(0);
            skin_mask(x,y,3) = uint8(0);
        end
    end
end

