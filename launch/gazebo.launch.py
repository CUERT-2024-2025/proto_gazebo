from setuptools import Command
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch_ros.actions import Node, SetParameter

import os

def generate_launch_description():
    pkg_name = 'proto_gazebo'

    use_sim_time = SetParameter(name='use_sim_time', value=True)

    world_path = PathJoinSubstitution(
        [FindPackageShare(pkg_name), "worlds", "karting_club_world.world"]
    )

    world_file_arg = DeclareLaunchArgument(
        'world',
        default_value=world_path,
        description='Full path to the world file to load'
    )

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            FindPackageShare("gazebo_ros"), "/launch/gazebo.launch.py"
        ]),
        launch_arguments={
            'world': LaunchConfiguration('world'),
            'extra_gazebo_args': '--verbose'  # This passes the flag correctly
        }.items()
    )

    description_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            FindPackageShare('proto_description'),'/launch/description.launch.py'
        ])
    )

    spawn_robot = Node(
        package='gazebo_ros',
        executable='spawn_entity.py',
        arguments=[
            '-topic', 'robot_description',
            '-entity', 'my_robot',
            '-x', '68',
            '-y', '-1.6',
            '-z', '-1.5',
            '-Y', '1.53'
        ],
        output='screen'
    )

    static_map_to_odom = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        arguments=['0', '0', '0', '0', '0', '0', 'world', 'map']
    )

    return LaunchDescription([
        use_sim_time,
        world_file_arg,
        gazebo,
        description_launch,
        spawn_robot,
        # static_map_to_odom
    ])
