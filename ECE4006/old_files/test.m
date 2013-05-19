% ECE 4006           Real-Time DSP with Dr. Barnwell             Fall 2005
%
% Section C Group 1
%  Lucas Rangit MAGASWERAN
%  Vincent Lacey
%  WaiLing Chan
%  Justin
%  Jaimen
%



% Initialization
clc;
close all; 
clear all;
%T = timer;
display('Initialization Complete.');

%Variables
fps = 2;

disp('Running frameGrabber');
mov = aviread('c:/temp/teacher_walks_off-01.avi');
movSize = size(mov);
%movie(mov,1);

% Background Seperation %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
display('Begin Image Capture...');

image = mov(1).cdata;
imshow(image);
figure;
A = filter_skin(image);
imshow(A);

Bx = find_clusters(A, 2, 'x');
By = find_clusters(A, 2, 'y');

% merge/combine regions
R = zeros(size(A));
[Mx My] = size(A);
%for t = 2:1:Mt
    for y = 1:1:My
        for x = 1:1:Mx
            if ( Bx(x,y) && By(x,y) )
                R(x,y) = uint8(255);
            else
                R(x,y) = uint8(0);
            end
        end
    %end
end

figure, imshow(R);

B = find_clusters(A, 2, 'x');
C = find_clusters(B, 2, 'y');
figure, imshow(C);

D = find_clusters(A, 2, 'y');
E = find_clusters(D, 2, 'x');
figure, imshow(E);
F = 255.*and(C, E);
figure, imshow(F);
