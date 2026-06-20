from ament_index_python import get_package_share_directory
from build.lane_detector.lane_detector import lane_segmentation_node
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable, TimerAction
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

    zed_interface_share = os.path.dirname(get_package_share_directory('zed_interfaces'))

    # Add it to Gazebo's search paths
    append_gazebo_path = SetEnvironmentVariable(
        name='GAZEBO_MODEL_PATH',
        value=[os.environ.get('GAZEBO_MODEL_PATH', ''), ':', zed_interface_share]
    )

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
        ]),
        launch_arguments={'sim_time': 'true'}.items()
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
            '-Y', '-1.601241'
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

    kinematics_node = Node(
        package="proto_controller",
        executable="kinematic_translator",
        output="screen",
    )

    # localization_launch = TimerAction(
    #     period=5.0,  # wait for Gazebo + controllers to be ready
    #     actions=[
    #         IncludeLaunchDescription(
    #             PythonLaunchDescriptionSource([
    #                 FindPackageShare('proto_localization'), '/launch/localization.launch.py'
    #             ]),
    #             launch_arguments={
    #                 'use_sim_time': 'true',
    #             }.items()
    #         )
    #     ]
    # )

    obstacle_detector_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            FindPackageShare('obstacle_detector'), '/launch/obstacle_detector.launch.py'
        ]),
        launch_arguments={
            'publish_obstacle_map': 'true',
            'marker_width': '0.30',
            'marker_height': '0.50',
        }.items()
    )

    stop_line_detector_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            FindPackageShare('stop_line_detector'), '/launch/stop_line_detector.launch.py'
        ]),
        launch_arguments={
            'publish_stop_line_markers': 'true',
        }.items()
    )

    lane_segmentation_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            FindPackageShare('lane_detector'), '/launch/lane_segmentation.launch.py'
        ]),
        launch_arguments={
            'publish_lane_markers': 'true',
        }.items()
    )

    return LaunchDescription([
        use_sim_time,
        world_name_arg,
        append_gazebo_path,
        gazebo,
        description_launch,
        spawn_robot,
        joint_state_broadcaster_spawner,
        rear_drive_spawner,
        front_steering_spawner,
        kinematics_node,
        obstacle_detector_launch,
        stop_line_detector_launch,
        # localization_launch,
        # obstacle_detector_launch,
        # stop_line_detector_launch,
    ])
