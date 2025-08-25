using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace SoundDefaultUI;

public class ThemeToDisabledColorConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        bool isDarkTheme = (bool)value;
        return isDarkTheme ? Color.FromRgb(119, 119, 119) : Colors.LightGray;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}