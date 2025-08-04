using System.Configuration;
using System.Data;
using System.Windows;

namespace SoundDefaultUI
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public SoundDeviceService? SoundDeviceService { private get; set; }

        public App()
        {
            InitializeComponent();
        }

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);

            SoundDeviceService?.Dispose();
        }
    }


}
