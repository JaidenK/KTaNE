using KTaNE_Console.ViewModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace KTaNE_Console
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        // Wrapping self in vm as placeholder for future refactoring
        private MainViewModel vm => DataContext as MainViewModel;

        private DispatcherTimer SerialTimer;

        public MainWindow()
        {
            InitializeComponent();

            SerialTimer = new DispatcherTimer();
            SerialTimer.Interval = TimeSpan.FromMilliseconds(10);
            SerialTimer.Tick += SerialTimer_Tick;
            SerialTimer.Start();
        }

        private void SerialTimer_Tick(object sender, EventArgs e)
        {
            //vm.PollSerial();
        }

        private void Window_ContentRendered(object sender, EventArgs e)
        {
        }

        private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            vm.Connect();
        }

        private void TextBox_ScrollToEnd(object sender, TextChangedEventArgs e)
        {
            (sender as TextBox).ScrollToEnd();
        }
    }
}
