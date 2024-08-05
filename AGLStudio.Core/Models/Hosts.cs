using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AGLStudio.Core.Models;

internal class Host
{
    public int config_version_high;
    public int config_version_low;
    public int device_kind;
    public string id;
    public string mqtt_broker_ip;
    public int mqtt_broker_port;
    public string mqtt_client_name;
    public string mqtt_command_topic;
    public int mqtt_connection_type;
    public string mqtt_error_topic;
    public string mqtt_login;
    public string mqtt_password;
    public int mqtt_ping_interval;
    public string mqtt_ping_topic;
    public string mqtt_response_topic;
    public string mqtt_settings_get_topic;
    public string mqtt_settings_set_topic;
    public string mqtt_tls_cert_file;
    public string name;
    public bool is_use_readonly_checker;
    public string readonly_checker_dictionary;
    public string settings_backup_path;
    public string secrets_dictionary;
	public bool is_create_backup;

    public ICollection<Camera> Cameras
    {
        get; set;
    }
}
