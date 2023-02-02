#ifndef DEPTH_SEGMENTATION_ROS_COMMON_H_
#define DEPTH_SEGMENTATION_ROS_COMMON_H_

#include <string>

#include <ros/ros.h>

namespace depth_segmentation {
const static std::string kRgbImageTopic = "/camera/color/image_raw";
const static std::string kRgbCameraInfoTopic = "/camera/color/camera_info";
const static std::string kDepthImageTopic =
    "/camera/aligned_depth_to_color/image_raw";
const static std::string kDepthCameraInfoTopic =
    "/camera/aligned_depth_to_color/camera_info";
const static std::string kSemanticInstanceSegmentationTopic =
    "/mask_rcnn/result";

const static std::string kPointCloud2Topic = "/camera/aligned_depth_to_color/points";  
const static std::string kJointStatesTopic = "/joint_states";

const static std::string kTfWorldFrame = "map";
const static std::string kTfDepthCameraFrame = "camera0";

}  // namespace depth_segmentation

#endif  // DEPTH_SEGMENTATION_ROS_COMMON_H_
