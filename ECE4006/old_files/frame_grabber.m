% The frameGrabber reads frames passed to it from the video stream and outputs 
% a smaller image region to filter_skin in the Main Method. On the first run 
% or if a head is not found, the entire cropped image is outputted.
% 2 variables open to optimization:
% 1) frameSize: specifies the length of the square region passed to filter_skin
% 2) cropThreshold: The point at which to crop out the bottom of the classroom. 
%    230 was found to best fit the video teacher_walks_off-01.avi.

function frameOut = frameGrabber(frame, x, y)
% frameGrabber reads in a video frame and outputs a focused frame for
% further processing
disp('Getting frame with frameGrabber');

% VARIABLES
frameSize = 50; % row by col size for output frame
cropThreshold = 230;
global oset;

%crop top
img_cropped = frame(1:cropThreshold,:,:);
[row col color] = size(img_cropped);

if ((x + y) == 0)
    frameOut = img_cropped;
    oset = [0 0];
else
    
    % Find upper left hand corner
    oset(1) =  oset(1) + x - round(frameSize/2);
    oset(2) =  oset(2) + y - round(frameSize/2);    
    row_end = oset(1)+frameSize;
    col_end = oset(2)+frameSize;
    % Safety Checks
    if oset(1) < 1
        oset(1) = 1; end
    if oset(2) < 1
        oset(2) = 1; end
    if row_end > row
        row_end = row; end
    if col_end > col
        col_end = col; end
    
    frameOut = img_cropped(oset(1):row_end, oset(2):col_end, :);

end %if
