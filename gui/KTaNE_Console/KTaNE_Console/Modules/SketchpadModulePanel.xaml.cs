using System;
using System.Collections.Generic;
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

namespace KTaNE_Console.Modules
{
    /// <summary>
    /// Interaction logic for SketchpadModulePanel.xaml
    /// </summary>
    public partial class SketchpadModulePanel : UserControl, IModulePanel
    {
        public SketchpadModulePanel()
        {
            InitializeComponent();
        }

        public List<byte[]> BuildSetEEPROMPackets()
        {
            return null;
        }
    }
}
