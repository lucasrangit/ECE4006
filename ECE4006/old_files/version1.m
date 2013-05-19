function A = grad(I)
A=I;
tempx = [];
tempy = [];
[row col k0] = size(I);
%for k=1:3
    for i=1:row-2
        for j=1:col-2
          if i == 1
              y = yOuterTopDeriv(i,j,I);
          elseif i == 2
              y = yInnerTopDeriv(i,j,I);
          elseif i == row-1
              y = yInnerBottomDeriv(i,j,I);
          elseif i == row
              y = yOuterBottomDeriv(i,j,I);
          else
              y = yCenterDeriv(i,j,I);
          end

          if j == 1
              x = xOuterLeftDeriv(i,j,I);
          elseif j == 2
              x = xInnerLeftDeriv(i,j,I);
          elseif j == row-1
              x = xInnerRightDeriv(i,j,I);
          elseif j == row
              x= xOuterRightDeriv(i,j,I);
          else
              x = xCenterDeriv(i,j,I);
          end
          
          A(i,j) = I(i,j) + x + y;
        end
    end
%end
function A = whiteout(I)
[row col k0] = size(I);
A = zeros(row,col,k0);
for i = 1:row
    for j = 1:col
            if ( I(i,j,1) == 0 && I(i,j,2) == 0 && I(i,j,3) == 0 )
                for k=1:3
                    A(i,j,k) = 0;
                end
            else
                for k=1:3
                    A(i,j,k) = 255;
                end
            end
    end
end
addpath('Z:\ece4006');
close all;
% Read in variables

obg = rgb2gray(imread('noman.png')); bg = double(obg);
oimage = rgb2gray(imread('strawman.png')); image = double(oimage);
diff = abs(image - bg);

%bleached = whiteout(diff);

%contour = grad(bleached);
contour = grad(diff);
%Display
imshow(obg);
figure, imshow(oimage);
figure, imshow(diff);
%figure, imshow(bleached);
figure, imshow(contour);
