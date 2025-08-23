using System.Globalization;
using System.Windows.Data;
using MaterialDesignThemes.Wpf;

namespace SoundDefaultUI;

public sealed class ThemeToColorZoneModeConverter : IValueConverter
{
    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        var isDark = value is true;
        return isDark ? ColorZoneMode.PrimaryDark : ColorZoneMode.PrimaryLight;
    }

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
        => Binding.DoNothing;
}
