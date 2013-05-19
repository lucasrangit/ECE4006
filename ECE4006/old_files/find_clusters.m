% I have not tested this to make sure it works yet, but I didn't change 
% anything from what Lucas had except I commented more.

function regions = find_clusters(frame, threshold, dir)
%INPUTS:
%    frame: a grayscale array of images F(row,col,t), where t is the array index 
%    threshold: how many white pixels must be found in a row to consider it a cluster
%    dir: either x or y, depending on whether you are sweeping the rows or columns for clusters
%OUTPUT:
%    regions: a grayscale array of images R(row,col,t)

[Mx My] = size(frame);

if (dir == 'x')
%for all frames but the first,
    %for t = 2:1:Mt
        %for every column,
        for y = 1:1:My
            %count the number of white pixels in a given column
            white_count = 0;
            %for each row in a given column
            for x = 1:1:Mx
                %if the pixel is white, count it
                if ( frame(x,y) == uint8(255) )
                    white_count = white_count + 1;
                else
                    %if enough white pixels in a row have not been found,
                    %then black out every one found and reset the counter.
                    if ( white_count <= threshold )
                        for j = x-white_count:x-1
                            frame(j,y) = uint8(0);
                        end
                        white_count = 0;
                    end
                end
            end
        %end
    end
elseif (dir == 'y')
    %for t = 2:1:Mt
        for x = 1:1:Mx
            white_count = 0;
            for y = 1:1:My
                if ( frame(x,y) == uint8(255) )
                    white_count = white_count + 1;
                else
                    if ( white_count <= threshold )
                        for j = y-white_count:y-1
                            frame(x,j) = uint8(0);
                        end
                        white_count = 0;
                    end
                end
            end
        end
   % end
end
regions = frame;
end

New find_head algorithm
function [row col] = find_head(frame)
[X Y] = size(frame);
half_x = ceil(X/2);
half_y = ceil(Y/2);
loopframe = frame./255;

X0 = 1;
Y0 = 1;
loopsum = 0;
while(half_x>X0 && half_x <X)
quad1sum = [0 1];
quad2sum = [0 2];
quad3sum = [0 3];
quad4sum = [0 4];
loopsum
    for i = X0:half_x
        for j = Y0:half_y
            quad1sum(1) = quad1sum(1) + loopframe(i,j);
        end
    end
    
    for i = X0:half_x
        for j = half_y:Y
            quad2sum(1) = quad2sum(1) + loopframe(i,j);
        end
    end
    
    for i = half_x:X
        for j = Y0:half_y
            quad3sum(1) = quad3sum(1) + loopframe(i,j);
        end
    end
    
    for i = half_x:X
        for j = half_y:Y
            quad4sum(1) = quad4sum(1) + loopframe(i,j);
        end
    end
    
    quad1sum(1)
    quad2sum(1)
    quad3sum(1)
    quad4sum(1)
    
    whichquad = maxquadrant(quad1sum, quad2sum, quad3sum, quad4sum)
    switch whichquad
        case 1
            X = half_x;
            Y = half_y;
            X0 = 1;
            Y0 = 1;
        case 2
            X = half_x;
            Y = Y;
            X0 = 1;
            Y0 = half_y;
        case 3
            X = X;
            Y = half_y;
            X0 = half_x;
            Y0 = 1;
        case 4
            X = X;
            Y = Y;
            X0 = half_x;
            Y0 = half_y;
    end
    half_x = ceil((X0+X)/2);
    half_y = ceil((Y0+Y)/2);
    X
    Y
    X0
    Y0
    loopsum = loopsum+1;
end
    row = half_x;
    col = half_y;
maxquadrant
function whichquad = maxquadrant(quad1sum, quad2sum, quad3sum, quad4sum)
whichquad = 0;
    if(quad1sum(1)>quad2sum(1))
        if(quad1sum(1)>quad3sum(1))
            if(quad1sum(1)>quad4sum(1))
                whichquad = quad1sum(2);
            else
                whichquad = quad4sum(2);
            end
        else
            if(quad3sum(1)>quad4sum(1))
                whichquad = quad3sum(2);
            else
                whichquad = quad4sum(2);
            end
        end
    else
        if(quad2sum(1)>quad3sum(1))
            if(quad2sum(1)>quad4sum(1))
                whichquad = quad2sum(2);
            else
                whichquad = quad4sum(2);
            end
        else
            if(quad3sum(1)>quad4sum(1))
                whichquad = quad3sum(2);
            else
                whichquad = quad4sum(2);
            end
        end
    end

Test code for find_head
frame = imread('C:\temp\image.bmp');
frame = frame(1:230,120:400,:);
[x y] = find_head(frame(:,:,1))
frame = box(x,y,10,10,2,frame,255,255,0);
imshow(frame);
