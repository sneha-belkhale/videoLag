/** @file videoLag.cpp
 
 @brief Main function to show a cool video editing effect
 @author Sneha Belkhale
 @date September 22nd, 2016
 
 When executing this file, it takes in a command line integer argument corresponding to different background detection methods
 args 1 : Adaptive gaussian background detection
 args 2 : Simple background subtraction, press 'space' when you are ready to set the background

 

 */

#include <iostream>

#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>


#define PI 3.14159265

/* main function */
int main(int argc, const char * argv[]) {
    
    /* initializations */
    std::string arg = argv[1];
    cv::namedWindow("webcam");
    cv::Mat frame; //place holder cv::Mat
    cv::Mat foregroundMask; //place holder cv::Mat
    cv::Mat background; //for static background method
    cv::Mat dstImage; //place holder cv::Mat
    int keyboard; //keyboard key to stop the program
    int count=0; //counter for debugging purposes
    int erosion_size = 1; // size of structuring element
    
    //initialize deques to hold the lagging frames and masks
    std::deque<cv::Mat> lagMasks;
    std::deque<cv::Mat> lagFrames;

    //create Background Subtractor objects
    cv::Ptr<cv::BackgroundSubtractor> pMOG2 = cv::createBackgroundSubtractorMOG2(500,4.0f*4.0f,false); //MOG2 approach
    
    //open a stream to campture webcam frames
    cv::VideoCapture stream(0);
    //make sure the webcam is opened correctly
    if(!stream.isOpened()){
        std::cerr << "Unable to open webcam"<< std::endl;
        exit(EXIT_FAILURE);
    }
    //create a structuring element for morphological operations on the foreground mask ...
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1), cv::Point(erosion_size, erosion_size) );
    
    //if using the simple background subtractor, ask user to set the background frame when ready
    if (arg =="2"){
        cv::Mat background_text;
        //get the background
        while((char)keyboard != 32){
            //make sure the webcam frame is opened correctly
            if(!stream.read(background)){
                std::cerr << "Unable to open webcam"<< std::endl;
                exit(EXIT_FAILURE);
            }
            background_text = background.clone();
            cv::putText(background_text, "press spacebar when ready ",cv::Point(200,200), CV_FONT_HERSHEY_SIMPLEX, 1 ,cv::Scalar(255,255,255),1,8,false);
            cv::imshow("webcam", background_text);
            keyboard=cv::waitKey(30);
        }
    }
    
    
    //Enter a continuous loop until the ESC key is pressed
    while((char)keyboard != 27){
    //while(count<30){ //for debugging
        //make sure the webcam frame is opened correctly
        if(!stream.read(frame)){
            std::cerr << "Unable to open webcam"<< std::endl;
            exit(EXIT_FAILURE);
        }
        //convert the frame to HSV for color manipulations
        //cv::cvtColor(frame,frame,CV_BGR2HSV);
        //instantiate a new matrix to hold the processed frame
        cv::Mat frameToProcess = cv::Mat(frame.rows, frame.cols, CV_8UC3, 0.0);
        //get a mask of the frame foreground region of interest
        if (arg == "1")
            pMOG2->apply(frame, foregroundMask);
        if (arg == "2") {
            cv::absdiff(background,frame,foregroundMask);
            cv::cvtColor(foregroundMask, foregroundMask, CV_BGR2GRAY);
            cv::threshold(foregroundMask, foregroundMask, 20, 255, cv::THRESH_BINARY);
        }
        //apply morphological operations to denoise the foreground mask
        //cv::morphologyEx(foregroundMask,foregroundMask,cv::MORPH_OPEN,element);
        //cv::erode(foregroundMask,foregroundMask,element);
        
        //insert a copy of the current foreground mask into our deque
        lagMasks.push_front(foregroundMask.clone());
        //insert a copy of the current frame into our deque for later reference
        lagFrames.push_front(frame.clone());
        /* 
        //for debugging, write the mask to an image file
        cv::String filename=std::to_string(count);
        filename+=".jpg";
        cv::imwrite(filename,foregroundMask);
        */
        
        
        //start by adding the most recent region of interest to our frame to process
        frame.copyTo(frameToProcess,foregroundMask);
            
        //all thats left is really to put some colors into this
        //can also try to put a gaussian blur or something ?
        //loop through the past frames and add them to the image wherever they do NOT overlap
        for(int i=1; i<lagMasks.size(); i++){
            cv::Mat temp;
            cv::Mat ROI = cv::Mat(frame.rows, frame.cols, CV_8UC1, 0.0);
            ROI=lagMasks[0].clone();
            for(int j=1; j<i;j++){
            cv::bitwise_or(lagMasks[j],ROI,ROI);
            }
            cv::bitwise_and(lagMasks[i],ROI,ROI);
            ROI=lagMasks[i]-ROI;
        
            
            // Modify the pixels of the RGB
            for (int ii=0; ii<lagFrames[i].rows; ii++)
            {
                for (int jj=0; jj<lagFrames[i].cols; jj++)
                {
                    lagFrames[i].at<cv::Vec3b>(ii,jj)[0] += 10;
                }
            }
            
            //copy the past frame to the region of interest
            lagFrames[i].copyTo(temp, ROI);
            //add it to our frame to process
            frameToProcess=temp+frameToProcess;
            
            /*
             //for debugging, write the mask to an image file
            cv::String filename=std::to_string(count*10+i);
            filename+=".jpg";
            cv::imwrite(filename,frameToProcess);
            */
            
        }
        
            //cv::cvtColor(frameToProcess,frameToProcess,CV_HSV2BGR);
            //fill the remaining background pixels with the current frame pixels
            cv::cvtColor(frameToProcess,dstImage,CV_BGR2GRAY);
            dstImage.setTo(255, dstImage > 0);
            cv::bitwise_not(dstImage,dstImage);
            frame.copyTo(frameToProcess,dstImage);
        
            //remove the oldest frame/ foreground mask from the deque
            if ( lagMasks.size()>5 ) {
                lagMasks.pop_back();
                lagFrames.pop_back();
            }
            //show the processed frame
            cv::imshow("webcam", frameToProcess);
            //set a lag time
            cv::waitKey(400);
            //update the counter for debugging purposes
            count++;
            //get user Key pressed
            keyboard=cv::waitKey(1);
            
}
    //release the webcam after exiting the while loop
    stream.release();
    return 0;
}


