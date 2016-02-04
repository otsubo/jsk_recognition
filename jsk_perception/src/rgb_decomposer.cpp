// -*- mode: c++ -*-
/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2015, JSK Lab
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/o2r other materials provided
 *     with the distribution.
 *   * Neither the name of the JSK Lab nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/


#include "jsk_perception/rgb_decomposer.h"
#include <boost/assign.hpp>
#include <jsk_topic_tools/log_utils.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

namespace jsk_perception
{
  void RGBDecomposer::onInit()
  {
    DiagnosticNodelet::onInit();
    pub_r_ = advertise<sensor_msgs::Image>(*pnh_, "output/red", 1);
    pub_g_ = advertise<sensor_msgs::Image>(*pnh_, "output/green", 1);
    pub_b_ = advertise<sensor_msgs::Image>(*pnh_, "output/blue", 1);
    pub_filtered_image_ = advertise<sensor_msgs::Image>(*pnh_, "output/filtered_image", 1);
    onInitPostProcess();
  }

  void RGBDecomposer::subscribe()
  {
    sub_ = pnh_->subscribe("input", 1, &RGBDecomposer::decompose, this);
    ros::V_string names = boost::assign::list_of("~input");
    jsk_topic_tools::warnNoRemap(names);
  }

  void RGBDecomposer::unsubscribe()
  {
    sub_.shutdown();
  }

  void RGBDecomposer::decompose(
    const sensor_msgs::Image::ConstPtr& image_msg)
  {
    if ((image_msg->width == 0) && (image_msg->height == 0)) {
        JSK_ROS_WARN("invalid image input");
        return;
    }
    cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(
      image_msg, image_msg->encoding);
    cv::Mat image = cv_ptr->image;
    if (image_msg->encoding == sensor_msgs::image_encodings::RGB8) {
      cv::cvtColor(image, image, CV_RGB2BGR);
    }

    std::vector<cv::Mat> bgr_planes;
    cv::split(image, bgr_planes);
    cv::Mat red = bgr_planes[2];
    cv::Mat blue = bgr_planes[0];
    cv::Mat green = bgr_planes[1];
    cv::Mat red_th;
    cv::Mat blue_th;
    cv::Mat green_th;
    cv::Mat red_tmp;
    cv::Mat blue_tmp;
    cv::Mat green_tmp;
    cv::Mat mask_image;
    cv::Mat filtered_image;
    //red max
    cv::threshold(red, red_tmp,150, 0, cv::THRESH_TOZERO_INV);
    //red minimum
    cv::threshold(red_tmp, red_th,10, 0, cv::THRESH_TOZERO);
    //blue max
    cv::threshold(blue, blue_tmp,120, 0, cv::THRESH_TOZERO_INV);
    //blue minimum
    cv::threshold(blue_tmp, blue_th,50, 0, cv::THRESH_TOZERO);
    //green max
    cv::threshold(green, green_tmp,240, 0, cv::THRESH_TOZERO_INV);
    //green minimum
    cv::threshold(green_tmp, green_th,70, 0, cv::THRESH_TOZERO);

    cv::bitwise_and(red_th, green_th, mask_image);
    cv::bitwise_and(mask_image, blue_th, mask_image);
    //cv::bitwise_and(image , image, filtered_image, mask_image);
    cv::threshold(mask_image, mask_image, 0, 255, cv::THRESH_BINARY);
    //image.copyTo(out_image);
    //image.copyTo(test_image);

    
    //cv::threshold(green,rest_image,200,250,cv::THRESH_TOZERO);
    //colorExtraction(&test_image, &out_image, CV_BGR2HSV, 150, 165, 100, 255, 70, 255);
    pub_r_.publish(cv_bridge::CvImage(
                     image_msg->header,
                     sensor_msgs::image_encodings::MONO8,
                     red).toImageMsg());
    pub_g_.publish(cv_bridge::CvImage(
                     image_msg->header,
                     sensor_msgs::image_encodings::MONO8,
                     green).toImageMsg());
    pub_b_.publish(cv_bridge::CvImage(
                     image_msg->header,
                     sensor_msgs::image_encodings::MONO8,
                     blue).toImageMsg());
    pub_filtered_image_.publish(cv_bridge::CvImage(
                                                   image_msg->header,
                                                   sensor_msgs::image_encodings::MONO8,
                                                   mask_image).toImageMsg());
  }
}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS (jsk_perception::RGBDecomposer, nodelet::Nodelet);
