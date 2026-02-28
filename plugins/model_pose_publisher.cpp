#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/common/common.hh>
#include <ignition/math/Pose3.hh>

// ROS 2 and Gazebo ROS includes
#include <gazebo_ros/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose.hpp>

// C++ Threading
#include <thread>
#include <atomic>
#include <chrono>

namespace gazebo
{
  class ModelPosePublisher : public WorldPlugin
  {
    public: ModelPosePublisher() : stop_thread_(false) {}

    public: ~ModelPosePublisher()
    {
      this->stop_thread_ = true;
      if (this->ros_thread_.joinable())
      {
        this->ros_thread_.join();
      }
    }

    public: void Load(physics::WorldPtr _world, sdf::ElementPtr _sdf) override
    {
      this->world_ = _world;
      this->ros_node_ = gazebo_ros::Node::Get(_sdf);

      std::string topic_name = "/model_poses";
      if (_sdf->HasElement("topicName"))
        topic_name = _sdf->Get<std::string>("topicName");

      this->model_keyword_ = "target_model";
      if (_sdf->HasElement("modelKeyword"))
        this->model_keyword_ = _sdf->Get<std::string>("modelKeyword");

      this->pub_ = this->ros_node_->create_publisher<geometry_msgs::msg::PoseArray>(topic_name, 10);

      this->ros_thread_ = std::thread(std::bind(&ModelPosePublisher::PublishLoop, this));

      RCLCPP_INFO(this->ros_node_->get_logger(), 
        "Threaded Model Pose Publisher loaded. Topic: %s, Keyword: %s", 
        topic_name.c_str(), this->model_keyword_.c_str());
    }

    private: void PublishLoop()
    {
      while (this->world_->SimTime().Double() == 0.0 && !this->stop_thread_ && rclcpp::ok())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      if (this->stop_thread_ || !rclcpp::ok()) return;

      geometry_msgs::msg::PoseArray cached_pose_array;
      cached_pose_array.header.frame_id = "world";

      physics::Model_V models = this->world_->Models();
      for (auto const& model : models)
      {
        std::string name = model->GetName();
        
        if (name.find(this->model_keyword_) != std::string::npos)
        {
          ignition::math::Pose3d pose = model->WorldPose();
          
          geometry_msgs::msg::Pose p;
          p.position.x = pose.Pos().X();
          p.position.y = pose.Pos().Y();
          p.position.z = pose.Pos().Z();
          p.orientation.x = 0;
          p.orientation.y = 0;
          p.orientation.z = 0;
          p.orientation.w = 1;
          
          cached_pose_array.poses.push_back(p);
        }
      }

      RCLCPP_INFO(this->ros_node_->get_logger(), 
        "Cached %zu static models. Beginning 50Hz publishing thread.", 
        cached_pose_array.poses.size());

      rclcpp::Rate rate(10.0);
      while (rclcpp::ok() && !this->stop_thread_)
      {
        cached_pose_array.header.stamp = this->ros_node_->now();
        
        this->pub_->publish(cached_pose_array);
        
        rate.sleep();
      }
    }

    private: physics::WorldPtr world_;
    
    private: gazebo_ros::Node::SharedPtr ros_node_;
    private: rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr pub_;
    private: std::string model_keyword_;

    private: std::thread ros_thread_;
    private: std::atomic<bool> stop_thread_;
  };

  GZ_REGISTER_WORLD_PLUGIN(ModelPosePublisher)
}