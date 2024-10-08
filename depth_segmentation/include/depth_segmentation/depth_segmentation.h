#ifndef DEPTH_SEGMENTATION_DEPTH_SEGMENTATION_H_
#define DEPTH_SEGMENTATION_DEPTH_SEGMENTATION_H_

#include <glog/logging.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/rgbd.hpp>

#include "depth_segmentation/DepthSegmenterConfig.h"
#include "depth_segmentation/common.h"

#include <pcl/pcl_base.h>
#include <pcl/point_types.h>
#include <cv_bridge/cv_bridge.h>

namespace depth_segmentation {

class Camera {
 public:
  Camera() : initialized_(false) {}
  Camera(const size_t height, const size_t width, const int type,
         const cv::Mat& camera_matrix)
      : initialized_(true),
        type_(type),
        height_(height),
        width_(width),
        camera_matrix_(camera_matrix) {}
  inline void initialize(const size_t height, const size_t width,
                         const int type, const cv::Mat& camera_matrix) {
    type_ = type;
    height_ = height;
    width_ = width;
    camera_matrix_ = camera_matrix;
    initialized_ = true;
  }
  inline void setCameraMatrix(const cv::Mat& camera_matrix) {
    CHECK(!camera_matrix.empty());
    camera_matrix_ = camera_matrix;
  }
  virtual void setImage(const cv::Mat& image) = 0;
  inline void setMask(const cv::Mat& mask) { mask_ = mask; }
  inline void setType(const int type) {
    CHECK(type == CV_8UC1 || type == CV_32FC1);
    type_ = type;
  }
  inline const bool initialized() const { return initialized_; }
  inline cv::Mat getCameraMatrix() const { return camera_matrix_; }
  inline cv::Mat getImage() const { return image_; }
  inline cv::Mat getMask() const { return mask_; }
  inline int getType() { return type_; }
  inline size_t getHeight() const { return height_; }
  inline size_t getWidth() const { return width_; }

 protected:
  cv::Mat image_;

 private:
  bool initialized_;
  int type_;
  size_t height_;
  size_t width_;
  cv::Mat camera_matrix_;
  cv::Mat mask_;
};

class DepthCamera : public Camera {
 public:
  DepthCamera() {}
  void setImage(const cv::Mat& image) {
    CHECK(!image.empty());
    CHECK(image.type() == CV_32FC1);
    image_ = image;
  }
};

class RgbCamera : public Camera {
 public:
  RgbCamera() {}
  void setImage(const cv::Mat& image) {
    CHECK(!image.empty());
    CHECK(image.type() == CV_8UC1);
    image_ = image;
  }
};

class CameraTracker {
 public:
  CameraTracker(const DepthCamera& depth_camera, const RgbCamera& rgb_camera);
  void initialize(const std::string odometry_type);

  bool computeTransform(const cv::Mat& src_rgb_image,
                        const cv::Mat& src_depth_image,
                        const cv::Mat& dst_rgb_image,
                        const cv::Mat& dst_depth_image,
                        const cv::Mat& src_depth_mask,
                        const cv::Mat& dst_depth_mask);
  bool computeTransform(const cv::Mat& dst_rgb_image,
                        const cv::Mat& dst_depth_image,
                        const cv::Mat& dst_depth_mask) {
    return computeTransform(getRgbImage(), getDepthImage(), dst_rgb_image,
                            dst_depth_image, getDepthMask(), dst_depth_mask);
  }
  void createMask(const cv::Mat& depth, cv::Mat* mask);
  void dilateFrame(cv::Mat& image, cv::Mat& depth);

  inline cv::Mat getTransform() const { return transform_; }
  inline cv::Mat getWorldTransform() const { return world_transform_; }
  inline cv::Mat getRgbImage() const { return rgb_camera_.getImage(); }
  inline cv::Mat getDepthImage() const { return depth_camera_.getImage(); }
  inline cv::Mat getDepthMask() const { return depth_camera_.getMask(); }
  inline DepthCamera getDepthCamera() const { return depth_camera_; }
  inline RgbCamera getRgbCamera() const { return rgb_camera_; }

  void visualize(const cv::Mat old_depth_image,
                 const cv::Mat new_depth_image) const;

  enum CameraTrackerType {
    kRgbdICPOdometry = 0,
    kRgbdOdometry = 1,
    kICPOdometry = 2
  };
  static constexpr size_t kImageRange = 255;
  static constexpr double kMinDepth = 0.15;
  static constexpr double kMaxDepth = 10.0;
  const std::vector<std::string> kCameraTrackerNames = {
      "RgbdICPOdometry", "RgbdOdometry", "ICPOdometry"};

 private:
  const DepthCamera& depth_camera_;
  const RgbCamera& rgb_camera_;
  cv::Ptr<cv::rgbd::Odometry> odometry_;
  cv::Mat transform_;
  cv::Mat world_transform_;
};

struct SemanticInstanceSegmentation {
  std::vector<cv::Mat> masks;
  std::vector<int> labels;
  std::vector<uint32_t> instance_ids;
};

class DepthSegmenter {
 public:
  DepthSegmenter(const DepthCamera& depth_camera, Params& params)
      : depth_camera_(depth_camera), params_(params) {
    CHECK_EQ(params_.normals.window_size % 2u, 1u);
    CHECK_EQ(params_.max_distance.window_size % 2u, 1u);
  };
  void initialize();
  void dynamicReconfigureCallback(
      depth_segmentation::DepthSegmenterConfig& config, uint32_t level);
  void computeDepthMap(const cv::Mat& depth_image, cv::Mat* depth_map);
  void computeDepthDiscontinuityMap(const cv::Mat& depth_image,
                                    cv::Mat* depth_discontinuity_map);
  void computeMaxDistanceMap(const cv::Mat& image, cv::Mat* max_distance_map);
  void computeNormalMap(const cv::Mat& depth_map, cv::Mat* normal_map);
  void computeMinConvexityMap(const cv::Mat& depth_map,
                              const cv::Mat& normal_map,
                              cv::Mat* min_convexity_map);
  void computeFinalEdgeMap(const cv::Mat& convexity_map,
                           const cv::Mat& distance_map,
                           const cv::Mat& discontinuity_map, cv::Mat* edge_map);
  void edgeMap(const cv::Mat& image, cv::Mat* edge_map);
  void labelMap(const cv::Mat& rgb_image, const cv::Mat& depth_image,
                const cv::Mat& depth_map, const cv::Mat& edge_map,
                const cv::Mat& normal_map, cv::Mat* labeled_map,
                std::vector<cv::Mat>* segment_masks,
                std::vector<Segment>* segments,
                const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pcl_cloud = NULL);
  void labelMap(
      const cv::Mat& rgb_image, const cv::Mat& depth_image,
      const SemanticInstanceSegmentation& semantic_instance_segmentation,
      const cv::Mat& depth_map, const cv::Mat& edge_map,
      const cv::Mat& normal_map, cv::Mat* labeled_map,
      std::vector<cv::Mat>* segment_masks, std::vector<Segment>* segments,
      const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pcl_cloud = NULL);

  void labelMap(
      const cv::Mat& rgb_image, const cv::Mat& depth_image,
      const SemanticInstanceSegmentation& semantic_instance_segmentation,
      const cv::Mat& depth_map, const cv::Mat& edge_map,
      const cv::Mat& normal_map, cv::Mat* labeled_map,
      std::vector<cv::Mat>* segment_masks, std::vector<Segment>* segments, std::vector<Segment>& overlap_segments,
      const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pcl_cloud = NULL);

  void createSegmentFromOverlapMask(const cv::Mat& rgb_image, const cv::Mat& depth_image,
                                  const cv::Mat& depth_map, const cv::Mat& edge_map,
                                  const cv::Mat& normal_map, cv::Mat* labeled_map,
                                  const cv::Mat& original_depth_map,  
                                  const cv::Mat& overlap_mask, const Segment& depth_segment, Segment& overlap_segment, size_t label, const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pcl_cloud);    

  void inpaintImage(const cv::Mat& depth_image, const cv::Mat& edge_map,
                    const cv::Mat& label_map, cv::Mat* inpainted);
  void findBlobs(const cv::Mat& binary,
                 std::vector<std::vector<cv::Point2i>>* labels);
  inline DepthCamera getDepthCamera() const { return depth_camera_; }

  inline void addToVectors(cv::Mat* rescaled_depth, cv::Mat* dilated_rescaled_depth,
                  cv_bridge::CvImagePtr cv_rgb_image, cv_bridge::CvImagePtr cv_depth_image,
                  cv::Mat* bw_image, cv::Mat* mask) {
    // Add each element to its corresponding vector
    if (rescaled_depth)
      rescaled_depth_vec_.push_back((*rescaled_depth).clone());
    
    if (dilated_rescaled_depth)
      dilated_rescaled_depth_vec_.push_back((*dilated_rescaled_depth).clone());
    
    rgb_image_vec_.push_back(cv_rgb_image);
    depth_image_vec_.push_back(cv_depth_image);
    
    if (bw_image)
      bw_image_vec_.push_back((*bw_image).clone());
    
    if (mask)
      mask_vec_.push_back((*mask).clone());
  }

  inline bool retrieveFromVectors(int index, cv::Mat* rescaled_depth, cv::Mat* dilated_rescaled_depth,
                         cv_bridge::CvImagePtr& cv_rgb_image, cv_bridge::CvImagePtr& cv_depth_image,
                         cv::Mat* bw_image, cv::Mat* mask) {
    // Check if index is out of bounds
    if (index < 0 || index >= rescaled_depth_vec_.size()) {
        return false;  // Failed retrieval
    }

    // Assign the pointers directly from the vectors
    if (rescaled_depth) {
        *rescaled_depth = rescaled_depth_vec_[index];
    }
    if (dilated_rescaled_depth) {
        *dilated_rescaled_depth = dilated_rescaled_depth_vec_[index];
    }
    cv_rgb_image = rgb_image_vec_[index];
    cv_depth_image = depth_image_vec_[index];
    if (bw_image) {
        *bw_image = bw_image_vec_[index];
    }
    if (mask) {
        *mask = mask_vec_[index];
    }

    // Clear vectors after retrieval
    rescaled_depth_vec_.clear();
    dilated_rescaled_depth_vec_.clear();
    rgb_image_vec_.clear();
    depth_image_vec_.clear();
    bw_image_vec_.clear();
    mask_vec_.clear();

    return true;  // Successful retrieval
  }
  inline int getVectorSize()
  {
    return rescaled_depth_vec_.size();
  }
  int selectBestDepthImage();

 private:
  void generateRandomColorsAndLabels(size_t contours_size,
                                     std::vector<cv::Scalar>* colors,
                                     std::vector<int>* labels);
  const DepthCamera& depth_camera_;
  Params& params_;

  cv::rgbd::RgbdNormals rgbd_normals_;
  std::vector<cv::Scalar> colors_;
  std::vector<int> labels_;
  size_t running_index_; 

  std::vector<cv::Mat> rescaled_depth_vec_;
  std::vector<cv::Mat> dilated_rescaled_depth_vec_;
  std::vector<cv_bridge::CvImagePtr> rgb_image_vec_;
  std::vector<cv_bridge::CvImagePtr> depth_image_vec_;
  std::vector<cv::Mat> bw_image_vec_;
  std::vector<cv::Mat> mask_vec_;
};

// TODO(ntonci): Make a unit test.
void segmentSingleFrame(const cv::Mat& rgb_image, const cv::Mat& depth_image,
                        const cv::Mat& depth_intrinsics,
                        depth_segmentation::Params& params, cv::Mat* label_map,
                        cv::Mat* normal_map,
                        std::vector<cv::Mat>* segment_masks,
                        std::vector<Segment>* segments);

}  // namespace depth_segmentation

#endif  // DEPTH_SEGMENTATION_DEPTH_SEGMENTATION_H_
