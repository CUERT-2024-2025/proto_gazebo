cmake_minimum_required(VERSION 3.5)
project(proto_gazebo)

# --------------------------
# Build tools
# --------------------------
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

# --------------------------
# C++ dependencies
# --------------------------
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(gazebo REQUIRED)
find_package(gazebo_ros REQUIRED)

# -------------------------------------------------------------
# BUILD THE GAZEBO PLUGIN
# -------------------------------------------------------------
add_library(mesh_to_pointcloud SHARED
  plugins/mesh_to_pointcloud.cpp
)

target_include_directories(mesh_to_pointcloud PUBLIC
  ${GAZEBO_INCLUDE_DIRS}
  plugins
)

target_link_libraries(mesh_to_pointcloud
  ${GAZEBO_LIBRARIES}
)

ament_target_dependencies(mesh_to_pointcloud
  rclcpp
  sensor_msgs
  geometry_msgs
  gazebo_ros
)

# -------------------------------------------------------------
# BUILD THE MODEL POSE PUBLISHER PLUGIN
# -------------------------------------------------------------
add_library(model_pose_publisher SHARED
  plugins/model_pose_publisher.cpp
)

target_include_directories(model_pose_publisher PUBLIC
  ${GAZEBO_INCLUDE_DIRS}
  plugins
)

target_link_libraries(model_pose_publisher
  ${GAZEBO_LIBRARIES}
)

ament_target_dependencies(model_pose_publisher
  rclcpp
  sensor_msgs
  geometry_msgs
  gazebo_ros
)

# -------------------------------------------------------------
# BUILD THE FRUSTUM FILTER SENSOR PLUGIN
# -------------------------------------------------------------
add_library(pc_frustum_filter SHARED
  plugins/pc_frustum_filter.cpp
)

target_include_directories(pc_frustum_filter PUBLIC
  ${GAZEBO_INCLUDE_DIRS}
  plugins
)

target_link_libraries(pc_frustum_filter
  ${GAZEBO_LIBRARIES}
)

ament_target_dependencies(pc_frustum_filter
  rclcpp
  sensor_msgs
  geometry_msgs
  gazebo_ros
)

# Install plugin .so files
install(TARGETS mesh_to_pointcloud model_pose_publisher pc_frustum_filter
  LIBRARY DESTINATION lib
)

# -------------------------------------------------------------
# Python nodes
# -------------------------------------------------------------
ament_python_install_package(${PROJECT_NAME})

# install(PROGRAMS
#   scripts/py_node.py
#   DESTINATION lib/${PROJECT_NAME}
# )

# Install models, meshes, launch files, rviz, worlds
install(DIRECTORY models launch worlds
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
