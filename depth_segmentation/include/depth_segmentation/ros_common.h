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
const static std::string kTfDepthCameraFrame = "camera_color_optical_frame";

const bool kForwardLabeledSegmentsOnly = true; 
const bool kUSeOverlapBitsOnly = false;
const bool kPublishWhileMoving = false;
const bool kUseJointVelocities = true;
const bool kUseSelectiveJoints = false;
const std::vector<std::string> kSelectiveJointNames = {"head_joint1", "head_joint2", "head_joint3", "head_joint4", "head_joint5", "head_joint6"};
const bool kUSeStabilityScores = true;
const float kMaxJointDifference = 0.005f;
const float kMaxJointVelocity = 0.002f;
const float kWaitTimeAfterStationary = 0.2f;
const int kMinSegmentSize = 50;
const float kMinSegmentDepth = 0.07;
const float kMaxSegmentDepth = 0.30;

}  // namespace depth_segmentation

#endif  // DEPTH_SEGMENTATION_ROS_COMMON_H_
