using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json.Linq;

namespace AGLStudio.Core.Helpers;

internal class JsonWrapper
{
    public object jsonGetObject(Dictionary<string, object> root, string name)
    {
        if (root.ContainsKey(name))
            return root[name];
        else
            return null;
    }

    /*
    public int jsonGetInt(JToken root, string name, int defval)
    {
        int res = defval;
        if (int.TryParse(root.ToString(), out res))
            return res;
        else
            return defval;
    }
    */

    public int jsonGetInt(Dictionary<string, object> root, string name, int defval)
    {
        int res = defval;
        object obj = jsonGetObject(root, name);
        if (obj != null)
        {
            if (int.TryParse(obj.ToString(), out res))
                return res;
            else
                return defval;
        }
        else
            return defval;
    }

    public Int64 jsonGetInt64(Dictionary<string, object> root, string name, Int64 defval)
    {
        Int64 res = defval;
        object obj = jsonGetObject(root, name);
        if (obj != null)
        {
            if (Int64.TryParse(obj.ToString(), out res))
                return res;
            else
                return defval;
        }
        else
            return defval;
    }

    public bool jsonGetBool(Dictionary<string, object> root, string name, bool defval)
    {
        bool res = defval;
        object obj = jsonGetObject(root, name);
        if (obj != null)
        {
            if (bool.TryParse(obj.ToString(), out res))
                return res;
            else
                return defval;
        }
        else
            return defval;
    }

    public float jsonGetFloat(Dictionary<string, object> root, string name, float defval)
    {
        float res = defval;
        object obj = jsonGetObject(root, name);
        if (obj != null)
        {
            if (float.TryParse(obj.ToString(), out res))
                return res;
            else
                return defval;
        }
        else
            return defval;
    }

    public string jsonGetString(Dictionary<string, object> root, string name, string defval)
    {
        string res = defval;
        object obj = jsonGetObject(root, name);
        if (obj != null)
        {
            return obj.ToString();
        }
        else
            return defval;
    }

    ///********************************************************************************************

    public object jsonGetObject(JObject root, string name)
    {
        if (root.ContainsKey(name))
        {
            if (root.GetValue(name).Type == JTokenType.Object || root.GetValue(name).Type == JTokenType.Array)
            {
                return (root.GetValue(name));
            }
        }

        return null;
    }

    public float jsonGetFloat(JObject root, string name, float defval)
    {
        if (root.ContainsKey(name))
        {
            if (root.GetValue(name).Type == JTokenType.Float || root.GetValue(name).Type == JTokenType.Integer)
            {
                return ((float)root.GetValue(name));
            }
        }

        return defval;
    }

    public bool jsonGetBool(JObject root, string name, bool defval)
    {
        if (root.ContainsKey(name))
        {
            if (root.GetValue(name).Type == JTokenType.Boolean)
            {
                return ((bool)root.GetValue(name));
            }
        }

        return defval;
    }

    public string jsonGetString(JObject root, string name, string defval)
    {
        if (root.ContainsKey(name))
            return root.GetValue(name).ToString();
        else
            return defval;
    }

    public int jsonGetInt(JObject root, string name, int defval)
    {
        if (root.ContainsKey(name))
        {
            if (root.GetValue(name).Type == JTokenType.Integer)
            {
                return ((int)root.GetValue(name));
            }
        }

        return defval;
    }
}
