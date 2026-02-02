#ifndef MESH_TO_PC_PLUGIN_HH
#define MESH_TO_PC_PLUGIN_HH

#include <gazebo/gazebo.hh>
#include <gazebo/common/Mesh.hh>
#include <gazebo/common/MeshManager.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo_ros/node.hpp>

#include <gazebo/rendering/Visual.hh>
#include <gazebo/rendering/Scene.hh>
#include <gazebo/common/Plugin.hh>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <std_msgs/msg/int32.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>

namespace gazebo
{
  class MeshToPointCloud : public VisualPlugin
  {
  public:
    MeshToPointCloud();
    ~MeshToPointCloud();
    void Load(rendering::VisualPtr _visual, sdf::ElementPtr _sdf) override;

  private:
    void PublishThread();
    
    rendering::VisualPtr visual;
    gazebo_ros::Node::SharedPtr ros_node;

    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub;

    sensor_msgs::msg::PointCloud2 cloud_msg;

    std::thread pub_thread;
    bool running = false;
  };
}
#endif
