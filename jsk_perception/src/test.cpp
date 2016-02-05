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


#include <opencv2/opencv.hpp>
#include <opencv2/optflow.hpp>

void main()
{
  using namespace cv;

  // DeepFlow計算用のインスタンスを生成
  auto deepflow = optflow::createOptFlow_DeepFlow();
  // 動画読み込み
  auto capture = VideoCapture();
  
  // 前のフレーム保存
  // まだグレースケールしか対応してないようなので変換
  Mat prev, tmp;
  capture >> tmp;
  cvtColor(tmp, prev, COLOR_RGB2GRAY);

  while (cv::waitKey(1) == -1)
    {
      Mat curr;
      capture >> tmp;
      if (tmp.empty())
        break;
      cvtColor(tmp, curr, COLOR_RGB2GRAY);

      // オプティカルフローの計算
      Mat flow;
      deepflow->calc(prev, curr, flow);

      // 表示するようにX成分とY成分に分解
      Mat flowXY[2];
      split(flow, flowXY);

      // 極座標に変換
      Mat magnitude, angle;
      cartToPolar(flowXY[0], flowXY[1], magnitude, angle, true);

      //  色相（H）はオプティカルフローの角度
      //  彩度（S）は0～1に正規化したオプティカルフローの大きさ
      //  明度（V）は1
      Mat hsvPlanes[3];
      hsvPlanes[0] = angle;
      hsvPlanes[1] = magnitude;
      hsvPlanes[2] = Mat::ones(angle.size(), CV_32F);
      //  HSVを合成して一枚の画像にする
      Mat hsv;
      merge(hsvPlanes, 3, hsv);

      // 表示
      cv::imshow("DeepFlow", hsv);

      // 前のフレームを保存
      prev = curr;
    }
}