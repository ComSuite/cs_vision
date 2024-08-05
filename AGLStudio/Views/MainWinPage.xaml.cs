using AGLStudio.ViewModels;

using Microsoft.UI.Xaml.Controls;

namespace AGLStudio.Views;

public sealed partial class MainWinPage : Page
{
    public MainWinViewModel ViewModel
    {
        get;
    }

    public MainWinPage()
    {
        ViewModel = App.GetService<MainWinViewModel>();
        InitializeComponent();
    }
}
