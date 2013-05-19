% Here is the code that I ran during our presentation on Tuesday. Except that
% I use the new bounding box function and have hardcoded the values of red 
% and green to 255 and blue to 0. I am working on implementing the feedback 
% of the bounding box.
%
% ECE 4006           Real-Time DSP with Dr. Barnwell             Fall 2005
%
% Section C Group 1
%  Lucas Rangit MAGASWERAN
%  Vincent Lacey
%  WaiLing Chan
%  Justin
%  Jaimin
%

% Initialization
clc;
close all; 
clear all;
%T = timer;
display('Initialization Complete.');

% Background Seperation %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
display('Begin Image Capture...');

% load image time t=1..19 into a matrix of images
figure;
for t = 1:1:19
    I(:,:,:,t) = imread(['teacher_with_student ' num2str(t,'%02.0f') '.tif']);
    subplot(4,5,t), imshow(I(:,:,:,t)), title(strcat('Image t=',num2str(t)));
end

% Crop Images %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
display('Applying Initial Bounding Box...');
bb_x = 110:220;
[Ix Iy Iz It] = size(I);
bb_y = Iy;
I = I(bb_x,:,:,:);

% Skin Tone Filter %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
display('Apply Skin Tone Filter...');

% compute ratios
display('Computing R/G Ratios...');
[Ix Iy Iz It] = size(I);
for t = 1:It
    for y = 1:Iy
        for x = 1:Ix
            I_RG(x,y,t) = double( I(x,y,1,t) ) / double( I(x,y,2,t) ) ;
        end
    end
end

% create skin pixel mask
display('Threshold Image...');
skin_min = 1.2;
skin_max = 1.4;
figure('Name',['Potential Skin Pixels ' num2str(skin_min) ' <= R/G <= ' num2str(skin_max)]);
[I_RGx I_RGy I_RGt] = size(I_RG);
for t = 1:I_RGt
    for y = 1:I_RGy
        for x = 1:I_RGx
            if ( skin_min <= I_RG(x,y,t) && I_RG(x,y,t) <= skin_max )
                I_RG_mask(x,y,t) = uint8(255);
            else
                I_RG_mask(x,y,t) = uint8(0);
            end
        end
    end
    subplot(4,5,t), imshow(I_RG_mask(:,:,t)), title(['Mask t=' num2str(t)]);
end

% Determine Foreground %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
display('Determine Foreground...');

% load background
figure;
B_0 = I_RG_mask(:,:,1);
subplot(4,5,1), subimage(B_0);
title('Background Image t=1');

% determine foreground
for t = 2:1:I_RGt
    F(:,:,t) = I_RG_mask(:,:,t) - B_0;
    subplot(4,5,t), imshow((F(:,:,t)));
    title(strcat('Foreground t=',num2str(t)));
end

% Motion and Optical Flow %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
display('Compute Optical Flow...');

% first order differencing
figure;
[Fx Fy Ft] = size(F);
for t = 2:1:Ft
    diff_1 = F(:,:,t) - F(:,:,t-1);
    M(:,:,t) = diff_1;
    subplot(4,5,t), imshow((M(:,:,t)));
    title(strcat('Motion at t=',num2str(t)));
end


% Spacial Clustering %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
display('Perform Region Merging / Spacial Clustering...');

% determine clusering in x direction
[Mx My Mt] = size(M);
xthreshold = 2;
regions_x = M;
for t = 2:1:Mt
    for y = 1:1:My
        white_count = 0;
        for x = 1:1:Mx
            if ( M(x,y,t) == uint8(255) )
                white_count = white_count + 1;
            else
                if ( white_count <= xthreshold )
                    for j = x-white_count:x-1
                        regions_x(j,y,t) = uint8(0);
                    end
                    white_count = 0;
                end
            end
        end
    end
end

% determine continuity in y direction
ythreshold = 2;
regions_y = M;
for t = 2:1:Mt
    for x = 1:1:Mx
        white_count = 0;
        for y = 1:1:My
            if ( M(x,y,t) == uint8(255) )
                white_count = white_count + 1;
            else
                if ( white_count <= ythreshold )
                    for j = y-white_count:y-1
                        regions_y(x,j,t) = uint8(0);
                    end
                    white_count = 0;
                end
            end
        end
    end
end

% merge/combine regions
figure('Name',['Potential Heads ' num2str(xthreshold) 'x' num2str(ythreshold)]);
for t = 2:1:Mt
    for y = 1:1:My
        for x = 1:1:Mx
            if ( regions_x(x,y,t) && regions_y(x,y,t) )
                R(x,y,t) = uint8(255);
            else
                R(x,y,t) = uint8(0);
            end
        end
    end
    subplot(4,5,t), subimage((R(:,:,t))), title(strcat('t=',num2str(t)));
end

% Draw Bounding Boxes %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
display('Determine Bounding Boxes...');

% compute the centroid
figure('Name','Tracked Heads');
[Rx Ry Rt] = size(R);
bb_x = 20;
bb_y = 20;
for t = 2:1:Rt
    weight_sum_x = zeros(1,Ry);
    weight_sum_y = zeros(1,Rx);
    for y = 1:1:Ry
        row_sum = 0;
        for x = 1:1:Rx
            row_sum = row_sum + double(R(x,y,t));
        end
        weight_sum_x(y) = row_sum;
    end
    for x = 1:1:Rx
        col_sum = 0;
        for y = 1:1:Ry
            col_sum = col_sum + double(R(x,y,t));
        end
        weight_sum_y(x) = col_sum; 
    end
    num_x = sum( weight_sum_x .* [1:Ry] );
    num_y = sum( weight_sum_y .* [1:Rx] );
    dom = sum( weight_sum_x );
    x_coord = num_x/dom;
    y_coord = num_y/dom;
    subplot(4,5,t), title(['t=' num2str(t)]);
    subimage(bounding_box(floor(y_coord),floor(x_coord),bb_x,bb_y,3,I(:,:,:,t)));
end
