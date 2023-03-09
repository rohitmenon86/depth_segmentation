#ifndef DEPTH_SEGMENTATION_ROS_COMMON_H_
#define DEPTH_SEGMENTATION_ROS_COMMON_H_

#include <string>

#include <ros/ros.h>

namespace depth_segmentation {
const static std::string kRgbImageTopic = "/camera/color/image_raw";
const static std::string kRgbCameraInfoTopic = "/camera/color/camera_info";
const static std::string kDepthImageTopic =
    "/camera/depth/image_raw";
const static std::string kDepthCameraInfoTopic =
    "/camera/depth/camera_info";
const static std::string kSemanticInstanceSegmentationTopic =
    "/mask_rcnn/result";

const static std::string kPointCloud2Topic = "/camera/depth/points";  
const static std::string kJointStatesTopic = "/joint_states";

const static std::string kTfWorldFrame = "world";
const static std::string kTfDepthCameraFrame = "camera_depth_frame";

const bool kForwardLabeledSegmentsOnly = true; 
const bool kUSeOverlapBitsOnly = true;

}  // namespace depth_segmentation

#endif  // DEPTH_SEGMENTATION_ROS_COMMON_H_
