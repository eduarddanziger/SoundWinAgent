using System.Windows;
using System.Windows.Controls;

namespace SoundDefaultUI.Controls;

public partial class DefaultDeviceControl
{
    public static readonly DependencyProperty HeaderProperty = DependencyProperty.Register(
        nameof(Header), typeof(string), typeof(DefaultDeviceControl), new PropertyMetadata("Default Device"));

    public string Header
    {
        get => (string)GetValue(HeaderProperty);
        set => SetValue(HeaderProperty, value);
    }

    public DefaultDeviceControl()
    {
        InitializeComponent();
    }
}
