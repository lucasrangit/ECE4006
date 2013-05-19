function [row col] = find_head(frame, mask)
% find_head finds the coordinates of greatest correlation btwn a frame and
% an input mask


% videoIN = aviread('c:/temp/teacher_walks_off-01.avi');
% image = filter_skin(videoIN(1).cdata);
% bgImage = imread('bg.bmp');
% frame = (image & bgImage).*255;
% mask = imread('headMask.bmp');

% Variables
topCrop = 120;
bottomCrop = 220;
headThold = 0.28;

maskSize = size(mask);
maskArea = maskSize(1)*maskSize(2);
frameSize = size(frame);

for i=topCrop:bottomCrop
    for j=1:(frameSize(2)-maskSize(2))
        subFrame = frame(i:(i+maskSize(1)-1),j:(j+maskSize(2)-1),:);
        tempCor = mask & subFrame;
        x = tempCor(:,:,1);
        cor = sum(x(:))/maskArea;
        %[i j cor]
        if cor > headThold
            row = i + floor(maskSize(1)/2);
            col = j + floor(maskSize(2)/2);
            return;
        end
    end
end
row = -1;
col = -1;
return;
