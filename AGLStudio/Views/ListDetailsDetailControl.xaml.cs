using AGLStudio.Core.Models;

using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using System.Diagnostics;
using Emgu.CV;

namespace AGLStudio.Views;

using System.Drawing;
using AGLStudio.Core.Helpers;
using Microsoft.UI.Xaml.Media.Imaging;

public sealed partial class ListDetailsDetailControl : UserControl
{
    public static readonly MQTTClient _mqttClient = new MQTTClient();
    public readonly VideoCapture capture;

    public SampleOrder? ListDetailsMenuItem
    {
        get => GetValue(ListDetailsMenuItemProperty) as SampleOrder;
        set => SetValue(ListDetailsMenuItemProperty, value);
    }

    public static readonly DependencyProperty ListDetailsMenuItemProperty = DependencyProperty.Register("ListDetailsMenuItem", typeof(SampleOrder), typeof(ListDetailsDetailControl), new PropertyMetadata(null, OnListDetailsMenuItemPropertyChanged));

    public void onMiscReceived(string topic, string data)
    {
        if (this.DispatcherQueue != null)
        {
            this.DispatcherQueue.TryEnqueue(() => { tb_mqtt_misc.Text = data; });

            if (capture != null)
            {
                Mat frame = capture.QueryFrame();
                if (!frame.IsEmpty)
                {
                    //frame.Reshape(frame.NumberOfChannels, 480);
                    Bitmap bitmap = Emgu.CV.BitmapExtension.ToBitmap(frame);
                    if (this.DispatcherQueue != null)
                    {
                        this.DispatcherQueue.TryEnqueue(() =>
                        {
                            BitmapImage image = new BitmapImage();
                            using (MemoryStream stream = new MemoryStream())
                            {
                                bitmap.Save(stream, System.Drawing.Imaging.ImageFormat.Bmp);
                                stream.Position = 0;
                                image.SetSource(stream.AsRandomAccessStream());
                            }
                            camera_frame.Source = image;
                        });
                    }
                }
            }
        }
        else
        {
            _mqttClient.unsubscribe_all();
            _mqttClient.disconnect();
        }
    }

    public ListDetailsDetailControl()
    {
        InitializeComponent();

        _mqttClient.connect("192.168.0.128", 1883, true);
        _mqttClient.miscCallback += onMiscReceived;

        capture = new VideoCapture("http://192.168.0.140:8088/camera1");
    }

    private static void OnListDetailsMenuItemPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is ListDetailsDetailControl control)
        {
            control.ForegroundElement.ChangeView(0, 0, 1);
        }
    }
}
