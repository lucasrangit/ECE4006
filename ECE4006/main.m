% Program Main
clear;
clc;
close all;

% USER VARIABLES
fps = 2; % frames processed per second
vel_thold = 1; % velocity threshold = (change in pixels) / fps
movie1 = 'c:/temp/teacher_walks_off-01.avi';
movie2 = 'c:/temp/teacher_desk_walks_off-04.avi';
bg1 = 'teacher_walks_off-01-bg.bmp';
bg2 = 'teacher_desk_walks_off-04-bg.bmp';
mask = 'headMask.bmp'

% Initialize Variables;
x_old = 0;
y_old = 0;

% Initiate video sequence
disp('Initiating input video sequence');
videoIN = aviread(movie2);
movSize = size(videoIN);
bgImage = imread(bg2);
headMask = imread(mask);

for t = 1:fps:movSize(2); %%%%%%%%%%%
    
    disp(['Processing frame ' num2str(t)]);
    
    % Read and mask image
    imageOut = videoIN(t).cdata;
    image = filter_skin(imageOut);
    fgImage = (image & bgImage).*255;
    
    % Find head
    [x y] = find_head(fgImage, headMask);
    if ( (x == -1) || (y == -1) )
        x = x_old;
        y = y_old;
    end
    x = floor((x + x_old)/2);
    y = floor((y + y_old)/2);
    
    % Compute velocity
    dx = (x - x_old)/fps;
    dy = (y - y_old)/fps;
    v = sqrt(dx^2 + dy^2);
    
    % Update history
    if (v > vel_thold)
        x_out = x;
        y_out = y;
    end
    x_old = x_out;
    y_old = y_out;
    
    % Write frame to movie output file
    imageOut = box(x_out,y_out, imageOut);
    fillImage = im2frame(imageOut);
    for k=t:t+(fps-1)
        videoOUT(k) = fillImage;
    end
    
end %%%%%%%%%%%

% Play tracked image
disp('Image tracking complete.  Playing tracked image...');
movie(videoOUT);
