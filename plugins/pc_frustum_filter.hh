#ifndef PC_FRUSTUM_FILTER_PLUGIN_HH
#define PC_FRUSTUM_FILTER_PLUGIN_HH

#include <gazebo/gazebo.hh>
#include <gazebo/sensors/sensors.hh>
#include <gazebo/plugins/SensorPlugin.hh>
#include <gazebo_ros/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>
#include <gazebo/physics/physics.hh>
#include <ignition/math/Pose3.hh>

#include <string>
#include <mutex>
#include <cmath>

namespace gazebo
{
  class PcFrustumFilter : public SensorPlugin
  {
  public:
    PcFrustumFilter();
    virtual ~PcFrustumFilter();
    virtual void Load(sensors::SensorPtr _sensor, sdf::ElementPtr _sdf);

  private:
    void PointCloudCallback(const sensor_msgs::msg::PointCloud2::SharedPtr msg);

    sensors::SensorPtr sensor_;
    gazebo_ros::Node::SharedPtr ros_node_;
    
    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_;

    double fov_x_;
    double fov_y_;
    double clip_near_;
    double clip_far_;
  };
}
#endif // PC_FRUSTUM_FILTER_PLUGIN_HH