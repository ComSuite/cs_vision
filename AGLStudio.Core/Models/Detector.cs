using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AGLStudio.Core.Models;

internal class Predecessor
{
    public int _class;
    public int id;
}

internal class Detector
{
    public int chnls;
    public int height;
    public int id;
    public string input_tensor_name;
    public bool is_draw_detections;
    public bool is_send_results;
    public bool is_use_gpu;
    public int kind;
    public string labels_path;
    public string model_path;
    public string name;
    public int neural_network_id;
    public string output_tensor_name;
    public Predecessor predecessor;
    public int results_mapping_rule;
    public string rules_path;
    public int width;
    public string color;
    public string on_detect;
    public bool execute_always;
    public int execute_mode;
}
