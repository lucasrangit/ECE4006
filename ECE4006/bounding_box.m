function A = box(cent_x, cent_y, img)
%cent_x: the x-coordinate of the center of the rectangle
%cent_y: the y-coordinate of the center of the rectangle
%dim_x: the length in pixels of the x-dimension of the box
%dim_y: the length in pixels of the y-dimension of the box
%thickness: thickness of line defining box
%img: a 3-dimensional vector representing the pixels of an image
%r: the red color value of the box
%g: the green color value of the box
%b: the blue color value of the box

% User variables
r = 255;
g = 255;
b = 0;
thickness = 2;
dim_x = 30;
dim_y = 40;

% Begin Code
A = img;
[row col color] = size(img);
half_x = round(dim_x/2);
half_y = round(dim_y/2);
%draw horizontal sides
for i=(cent_x - half_x - thickness + 1):(cent_x + half_x + thickness - 1)
    if (i>0)&&(i< row)
        for t=0:(thickness-1)
        if ((cent_y - half_y - t) > 0) && ((cent_y - half_y - t) < col)
       
            A(i, (cent_y - half_y - t), 1) = r;
            A(i, (cent_y - half_y - t), 2) = g;
            A(i, (cent_y - half_y - t), 3) = b;
        end    
        if cent_y + half_y + t > 0 && cent_y + half_y + t < col
            
            A(i, (cent_y + half_y + t), 1) = r;
            A(i, (cent_y + half_y + t), 2) = g;
            A(i, (cent_y + half_y + t), 3) = b;
        end
        end
    end
end

%draw vertical sides
for i=cent_y - half_y - thickness + 1 :cent_y + half_y + thickness - 1
    if i >0 && i< col
        for t=0:(thickness - 1)
            if cent_x - half_x - t > 0 && cent_x - half_x - t < row
            A((cent_x - half_x - t), i, 1) = r;
            A((cent_x - half_x - t), i, 2) = g;
            A((cent_x - half_x - t), i, 3) = b;
            end
            if cent_x + half_x + t > 0 && cent_x + half_x + t < row
            A((cent_x + half_x + t), i, 1) = r;
            A((cent_x + half_x + t), i, 2) = g;
            A((cent_x + half_x + t), i, 3) = b;
        end
        end
    end
end
