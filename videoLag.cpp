/** @file videoLag.cpp
 
 @brief Main function to test different input points and triangulation methods in the TriSubdiv class.
 @author Sneha Belkhale
 @date September 22nd, 2016
 
 When executing this file, it takes in a command line integer argument from 1 - 10, corresponding to the different points and patterns.
 args 1 : 7 triangulate the image by generating 3 new triangles each time a point is added to the set
 args 7 : 10 simply create triangles by connecting the new point to its nearest 2 neighbors
 
 Displays the ~beautiful nature~ of the triangulation in an animation.

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
    
    cv::namedWindow("webcam");
    cv::Mat frame; //place holder cv::Mat
    cv::Mat foregroundMask; //place holder cv::Mat
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
        pMOG2->apply(frame, foregroundMask);
        //apply morphological operations to denoise the foreground mask
        //cv::morphologyEx(foregroundMask,foregroundMask,cv::MORPH_OPEN,element);
        //cv::erode(foregroundMask,foregroundMask,element);
        
        //insert a copy of the current foreground mask into our deque
        lagMasks.push_back(foregroundMask.clone());
        //insert a copy of the current frame into our deque for later reference
        lagFrames.push_back(frame.clone());
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
        
            /*
            // Modify the pixels of the HSV image
            for (int i=0; i<lagFrames[i].rows; i++)
            {
                for (int j=0; j<lagFrames[i].cols; j++)
                {
                    lagFrames[i].at<cv::Vec3b>(i,j)[0] += 10;
                }
            }*/
            
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

            cv::cvtColor(frameToProcess,dstImage,CV_BGR2GRAY);
            //add back in the past
            dstImage.setTo(255, dstImage > 0);
            cv::bitwise_not(dstImage,dstImage);
            
            frame.copyTo(frameToProcess,dstImage);
        
            if(lagMasks.size()>5){
            lagMasks.pop_front();
                lagFrames.pop_front();}

        cv::imshow("webcam", frameToProcess);
        cv::waitKey(400);
            count++;
        keyboard=cv::waitKey(30);
            
            
}
    stream.release();
        
    return 0;
}


