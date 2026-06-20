#include "pc_frustum_filter.hh"

using namespace gazebo;

GZ_REGISTER_SENSOR_PLUGIN(PcFrustumFilter)

PcFrustumFilter::PcFrustumFilter() : SensorPlugin()
{
}

PcFrustumFilter::~PcFrustumFilter()
{
}

void PcFrustumFilter::Load(sensors::SensorPtr _sensor, sdf::ElementPtr _sdf)
{
    sensor_ = _sensor;

    std::string input_topic = "mesh_vertices";
    std::string output_topic = "camera/filtered_points";
    fov_x_ = 1.047; // ~60 degrees default
    fov_y_ = 0.785; // ~45 degrees default
    clip_near_ = 0.1;
    clip_far_ = 100.0;

    if (_sdf->HasElement("input_topic"))
        input_topic = _sdf->Get<std::string>("input_topic");
    if (_sdf->HasElement("output_topic"))
        output_topic = _sdf->Get<std::string>("output_topic");
    
    if (_sdf->HasElement("fov_x"))
        fov_x_ = _sdf->Get<double>("fov_x");
    if (_sdf->HasElement("fov_y"))
        fov_y_ = _sdf->Get<double>("fov_y");
        
    if (_sdf->HasElement("clip_near"))
        clip_near_ = _sdf->Get<double>("clip_near");
    if (_sdf->HasElement("clip_far"))
        clip_far_ = _sdf->Get<double>("clip_far");

    // Retrieve the ROS 2 node from Gazebo ROS wrapper
    ros_node_ = gazebo_ros::Node::Get(_sdf);

    // Publishers and Subscribers
    pub_ = ros_node_->create_publisher<sensor_msgs::msg::PointCloud2>(output_topic, 10);
    
    auto callback = [this](const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
        this->PointCloudCallback(msg);
    };

    sub_ = ros_node_->create_subscription<sensor_msgs::msg::PointCloud2>(
        input_topic, 10, callback);

    RCLCPP_INFO(ros_node_->get_logger(), "PcFrustumFilter loaded for sensor: %s", sensor_->Name().c_str());
    RCLCPP_INFO(ros_node_->get_logger(), "Input: %s | Output: %s", input_topic.c_str(), output_topic.c_str());
    RCLCPP_INFO(ros_node_->get_logger(), "FOV(X: %.2f, Y: %.2f), Clip(%.2f - %.2f)", fov_x_, fov_y_, clip_near_, clip_far_);
}

void PcFrustumFilter::PointCloudCallback(const sensor_msgs::msg::PointCloud2::SharedPtr msg)
{
    // Find the current pose of the sensor in the world
    physics::WorldPtr world = physics::get_world(sensor_->WorldName());
    if (!world) return;

    physics::EntityPtr parent = world->EntityByName(sensor_->ParentName());
    if (!parent) return;

    // Calculate sensor world pose
    ignition::math::Pose3d parent_pose = parent->WorldPose();
    ignition::math::Pose3d sensor_world_pose = sensor_->Pose() + parent_pose;

    // Transformation backwards from world to sensor frame
    ignition::math::Pose3d world_to_sensor = sensor_world_pose.Inverse();

    sensor_msgs::msg::PointCloud2 out_msg;
    out_msg.header = msg->header;
    out_msg.height = 1;
    out_msg.is_dense = true;

    out_msg.fields = msg->fields;
    out_msg.point_step = msg->point_step;
    out_msg.is_bigendian = msg->is_bigendian;

    // Iterators for the input cloud
    sensor_msgs::PointCloud2ConstIterator<float> iter_x(*msg, "x");
    sensor_msgs::PointCloud2ConstIterator<float> iter_y(*msg, "y");
    sensor_msgs::PointCloud2ConstIterator<float> iter_z(*msg, "z");

    // Reserve max possible size
    out_msg.data.resize(msg->data.size()); 
    
    sensor_msgs::PointCloud2Iterator<float> out_x(out_msg, "x");
    sensor_msgs::PointCloud2Iterator<float> out_y(out_msg, "y");
    sensor_msgs::PointCloud2Iterator<float> out_z(out_msg, "z");

    uint32_t num_points = 0;
    
    // In Gazebo standard coordinate frame:
    // +X is forward
    // +Y is left
    // +Z is up
    double tan_fov_x = std::tan(fov_x_ / 2.0);
    double tan_fov_y = std::tan(fov_y_ / 2.0);

    for (; iter_x != iter_x.end(); ++iter_x, ++iter_y, ++iter_z)
    {
        ignition::math::Vector3d p_world(*iter_x, *iter_y, *iter_z);
        // Convert point to the sensor local frame
        ignition::math::Vector3d p_sensor = world_to_sensor * p_world;

        double x = p_sensor.X(); // Depth/forward Distance
        double y = p_sensor.Y(); // Horizontal Distance (left/right)
        double z = p_sensor.Z(); // Vertical Distance (up/down)

        // Check if the point falls within our view frustum limits defined by FOV / clipping
        if (x >= clip_near_ && x <= clip_far_)
        {
            if (std::abs(y) <= x * tan_fov_x && std::abs(z) <= x * tan_fov_y)
            {
                *out_x = *iter_x;
                *out_y = *iter_y;
                *out_z = *iter_z;

                ++out_x;
                ++out_y;
                ++out_z;
                
                num_points++;
            }
        }
    }

    out_msg.width = num_points;
    out_msg.row_step = num_points * out_msg.point_step;
    out_msg.data.resize(out_msg.row_step);

    pub_->publish(out_msg);
}