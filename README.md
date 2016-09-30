# videoLag
OpenCV video editing effect - - produces colored motion lags to a foreground object in video detected from webcam 


When executing this file, it takes in a command line integer argument corresponding to different background detection methods

args = 1 : Adaptive gaussian mixture-based background segmentation algorithm (cv::BackgroundSubtractorMOG2)

args = 2 : Simple background subtraction using absolute differencing and thresholding, press 'space' when you are ready to set the background

 
The program uses morphological operations to denoise the obtained foreground mask, and uses a deque container to store and print 
past foreground masks (in different hues), for a cool motion lag effect. 
The video effect will work best when the webcam background is high contrast with the foreground, and VERY stable. 

