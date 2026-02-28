from setuptools import Command
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression
from launch_ros.substitutions import FindPackageShare
from launch_ros.actions import Node, SetParameter

import os

def generate_launch_description():
    pkg_name = 'proto_gazebo'

    use_sim_time = SetParameter(name='use_sim_time', value=True)

    world_name_arg = DeclareLaunchArgument(
        'world',
        default_value='karting_club',
        description='Name of the world file (without .world extension)'
    )

    world_path = PathJoinSubstitution([
        FindPackageShare(pkg_name),
        "worlds",
        PythonExpression(["'", LaunchConfiguration('world'), ".world'"])
    ])

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            FindPackageShare("gazebo_ros"), "/launch/gazebo.launch.py"
        ]),
        launch_arguments={
            'world': world_path,
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
            '-x', '67.968980',
            '-y', '-0.543021',
            '-z', '-2.130163',
            '-Y', '1.540349'
        ],
        output='screen'
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner.py",
        arguments=["joint_state_broadcaster", "-c", "/controller_manager"],
        output="screen",
    )
    
    rear_drive_spawner = Node(
        package="controller_manager",
        executable="spawner.py",
        arguments=["rear_wheel_velocity_controller", "-c", "/controller_manager"], 
        output="screen",
    )
    
    front_steering_spawner = Node(
        package="controller_manager",
        executable="spawner.py",
        arguments=["front_steering_controller", "-c", "/controller_manager"],
        output="screen",
    )

    return LaunchDescription([
        use_sim_time,
        world_name_arg,
        gazebo,
        description_launch,
        spawn_robot,
        joint_state_broadcaster_spawner,
        rear_drive_spawner,
        front_steering_spawner
    ])
