Installtion guide for NVIDIA Jetson Nano based devices (AAEON Boxer 8221AI & etc.) 
                                   **********
								     v 1.2
Author: Alexander Epstine (a@epstine.com)								   
Date: 18 May 2024
								   
JetPack installation
******************************************
For flash Jetson Nano board you needed a Linux 16 or 18 x86-64 computer.

It is possible to install native NVIDIA BSP:
https://docs.nvidia.com/sdk-manager/download-run-sdkm/index.html

For install vendor`s BSP, for example:
https://data-us.aaeon.com/DOWNLOAD/MANUAL/BOXER-8221AI%20Manual%202nd%20Ed.pdf

Also you can probably install JetPack from WSL2:
https://docs.nvidia.com/sdk-manager/wsl-systems/index.html

After the board was flashed you should delete unused software nad then install additional packages and libraries.

Install additional software
******************************************
sudo apt install nano 
sudo apt-get -y install python3-pip
sudo -H pip3 install -U jetson-stats
sudo reboot

Delete GUI
******************************************
Just use the jetson_config utility from jetson-stats package:

sudo jetson_config

Delete unused software
******************************************
sudo docker volume ls -q -f driver=nvidia-docker | xargs -r -I{} -n1 docker ps -q -a -f volume={} | xargs -r docker rm -f
sudo apt-get remove --purge -y nvidia-docker
sudo apt-get remove --purge -y nvidia-docker2

sudo apt update
sudo apt autoremove -y
sudo apt clean
sudo apt remove thunderbird libreoffice-* -y
sudo apt-get remove --purge libreoffice* -y
sudo dpkg -P libreoffice*
sudo apt-get remove --purge gnome-shell ubuntu-wallpapers-bionic light-themes chromium-browser* libvisionworks libvisionworks-sfm-dev 
sudo apt-get remove --purge aisleriot gnome-sudoku mahjongg ace-of-penguins gnomine gbrainy gnome-mines
sudo apt-get remove --purge gnome-mahjongg gnomine gnome-sudoku aisleriot
sudo apt-get remove --purge aisleriot gnome-sudoku gnome-mines

sudo apt-get purge ubuntu-desktop

sudo apt-get autoremove -y
sudo apt clean -y

Delete preinstalled OpenCV
******************************************
sudo apt remove --purge libopencv
sudo ldconfig

Install OpenCV with cuda & etc. support
******************************************
copy all files from Distrib\Lib to host /usr/local/lib
Then execute:

sudo ldconfig

Install dependences
******************************************
sudo apt-get install -y libmosquitto-dev
sudo pip3 install -U jetson-stats
sudo apt-get install libatlas-base-dev

Performance
******************************************

To maximize performance:
sudo nvpmodel -m 0
sudo jetson_clocks

Overclocking:
https://forums.developer.nvidia.com/t/overclocking-jetson-nanos-cpu-and-gpu/83501
https://developer.ridgerun.com/wiki/index.php/Xavier/JetPack_5.0.2/Performance_Tuning/Maximizing_Performance
https://qengineering.eu/overclocking-the-jetson-nano.html

Monitoring
******************************************
You can use jtop from jetson-stats package or tegrastats.

Build TensorRT engine from ONNX files
******************************************

cd ~/bin
./build_engine --model <onnx file path> --output <engine file path>

For help: ./build_engine --help


How to run and config the SensFusion`s application:
******************************************
You should copy bin, settings, samples and models folders from distro to the target board and:

cd ~/bin
chmod u+x ./larch_vision_jetson_nano.out

For check installation:
cd ~/bin
./larch_vision_jetson_nano.out ../settings/larch_jn_settings_trt.json


Config MQTT section for device and cameras
******************************************

Whole device specific:
**************************
"mqtt_broker_ip": "192.168.0.128", Ip address of mqtt broker. Can be local or internet
"mqtt_broker_port": 1883, // port, 1883 by default
"mqtt_client_name": "device1",
"mqtt_command_topic": "larch/device1", 
"mqtt_settings_get_topic": "larch/settings_get",
"mqtt_settings_set_topic": "larch/settings_set",
"mqtt_login": "",
"mqtt_password": "",
"mqtt_tls_cert_file": "",
"mqtt_connection_type": Possible values: 0 - without any security, 1 - with login/password , 2 - TLS

Camera specific:
**************************
"mqtt_detection_topic": "larch/detections",


