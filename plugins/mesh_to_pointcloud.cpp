#include "mesh_to_pointcloud.hh"

using namespace gazebo;

MeshToPointCloud::MeshToPointCloud() {}

MeshToPointCloud::~MeshToPointCloud()
{
    running = false;
    if (pub_thread.joinable())
        pub_thread.join();
}

void MeshToPointCloud::Load(physics::ModelPtr _model, sdf::ElementPtr _sdf)
{
    model = _model;
    gzmsg << "MeshToPointCloud plugin loaded for model: "
          << model->GetName() << "\n";

    std::string topic_name = "mesh_vertices"; // default

    if (_sdf->HasElement("topic"))
    {
        topic_name = _sdf->Get<std::string>("topic");
    }

    ros_node = gazebo_ros::Node::Get(_sdf);
    pub = ros_node->create_publisher<sensor_msgs::msg::PointCloud2>(topic_name, 10);

    // ----------------------------- Load the mesh -----------------------------
    if (!_sdf->HasElement("mesh_uri"))
    {
        gzerr << "MeshToPointCloud plugin needs <mesh_uri>\n";
        return;
    }

    std::string meshUri = _sdf->Get<std::string>("mesh_uri");
    auto *mgr = gazebo::common::MeshManager::Instance();
    const gazebo::common::Mesh *mesh = mgr->Load(meshUri);

    if (!mesh)
    {
        gzerr << "Failed to load mesh: " << meshUri << "\n";
        return;
    }

    float *vertexArray = nullptr;
    int *indexArray = nullptr;
    mesh->FillArrays(&vertexArray, &indexArray);
    unsigned int numVertices = mesh->GetVertexCount();

    gzmsg << "Loaded mesh with " << numVertices << " vertices\n";

    // ----------------------------- Build cloud -----------------------------
    cloud_msg.header.frame_id = "world";
    cloud_msg.height = 1;
    cloud_msg.width = numVertices;
    cloud_msg.is_dense = true;

    cloud_msg.fields.resize(3);
    cloud_msg.fields[0].name = "x";
    cloud_msg.fields[1].name = "y";
    cloud_msg.fields[2].name = "z";
    cloud_msg.fields[0].offset = 0;
    cloud_msg.fields[1].offset = 4;
    cloud_msg.fields[2].offset = 8;
    cloud_msg.fields[0].datatype = sensor_msgs::msg::PointField::FLOAT32;
    cloud_msg.fields[1].datatype = sensor_msgs::msg::PointField::FLOAT32;
    cloud_msg.fields[2].datatype = sensor_msgs::msg::PointField::FLOAT32;

    cloud_msg.point_step = 12;
    cloud_msg.row_step = cloud_msg.point_step * numVertices;
    cloud_msg.data.resize(cloud_msg.row_step);

    sensor_msgs::PointCloud2Iterator<float> iterX(cloud_msg, "x");
    sensor_msgs::PointCloud2Iterator<float> iterY(cloud_msg, "y");
    sensor_msgs::PointCloud2Iterator<float> iterZ(cloud_msg, "z");

    for (unsigned int i = 0; i < numVertices; i++)
    {
        *iterX = vertexArray[i * 3 + 0];
        *iterY = vertexArray[i * 3 + 1];
        *iterZ = vertexArray[i * 3 + 2];

        ++iterX;
        ++iterY;
        ++iterZ;
    }

    // ----------------------------- Start thread -----------------------------
    running = true;
    pub_thread = std::thread(&MeshToPointCloud::PublishThread, this);

    gzmsg << "Publishing thread started.\n";
}

void MeshToPointCloud::PublishThread()
{
    rclcpp::Rate rate(10); // 10 Hz

    while (running)
    {
        cloud_msg.header.stamp = ros_node->now();

        pub->publish(cloud_msg);

        rate.sleep();
    }
}

GZ_REGISTER_MODEL_PLUGIN(MeshToPointCloud)