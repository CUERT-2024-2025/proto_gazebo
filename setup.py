import os
from setuptools import setup
from glob import glob

package_name = 'proto_gazebo'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        (os.path.join('share', package_name), ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*.launch.py'))),
        (os.path.join('share', package_name, 'worlds'), glob(os.path.join('worlds', '*.world'))),
        (os.path.join('share', package_name, 'rviz'), glob(os.path.join('rviz', '*.rviz'))),
        *[(os.path.join('share', package_name, os.path.dirname(file_path)), [file_path]) 
          for file_path in glob(os.path.join('models', '**/*'), recursive=True) 
          if os.path.isfile(file_path)],        
    ],

    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='ziad',
    maintainer_email='ziadmontaser2005@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
        ],
    },
)
