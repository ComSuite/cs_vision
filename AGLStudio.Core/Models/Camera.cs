using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AGLStudio.Core.Models;

internal class Camera
{
    public string device;
    public string id;
    public bool is_convert_to_gray;
    public bool is_display;
    public bool is_flip;
    public bool is_show_mask;
    public bool is_use_gpu;
    public string mqtt_broker_ip;
    public int mqtt_broker_port;
    public string mqtt_client_name;
    public int mqtt_connection_type;
    public string mqtt_detection_topic;
    public string mqtt_error_topic;
    public string mqtt_login;
    public string mqtt_password;
    public int mqtt_ping_interval;
    public string mqtt_ping_topic;
    public string mqtt_tls_cert_file;
    public bool mqtt_is_send_empty;
    public int connection_attempts_count;
    public string name;
    public int object_detector_kind;
    public int resize_x;
    public int resize_y;
    public int rotate_angle;
    public string video_stream_channel;
    public int video_stream_engine;
    public string video_stream_login;
    public int video_stream_mode;
    public string video_stream_password;
    public int video_stream_port;
    public bool is_use_super_resolution;
    public string super_resolution_name;
    public string super_resolution_model_path;
    public int super_resolution_factor;
    public string on_preprocess;
    public string on_postprocess;
    public bool execute_always;
    public int execute_mode;
    public int input_kind;
    public int output_kind;

    public ICollection<Detector> Detectors
    {
        get; set;
    }
}
