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

# Install plugin .so file
install(TARGETS mesh_to_pointcloud
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
