using System;
using System.Collections.Generic;
using System.ComponentModel;
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
    /// Interaction logic for KeypadsModulePanel.xaml
    /// </summary>
    public partial class KeypadsModulePanel : UserControl, INotifyPropertyChanged, IModulePanel
    {
        readonly ushort EEPROM_KEY1 = 0x1E0;
        readonly ushort EEPROM_KEY2 = 0x1E1;
        readonly ushort EEPROM_KEY3 = 0x1E2;
        readonly ushort EEPROM_KEY4 = 0x1E3;

        IModule module;

        public int Pad1 { get; set; } = 0;
        public int Pad2 { get; set; } = 0;
        public int Pad3 { get; set; } = 0;
        public int Pad4 { get; set; } = 0;

        public KeypadsModulePanel(IModule module)
        {
            this.module = module;
            this.DataContext = this;

            byte val = (byte)(module.Eeprom.bytes[EEPROM_KEY1] - 1);
            Pad1 = val < 31 ? val : 22; // Default to Left C
            val = (byte)(module.Eeprom.bytes[EEPROM_KEY2] - 1);
            Pad2 = val < 31 ? val : 27; // Default to Balloon
            val = (byte)(module.Eeprom.bytes[EEPROM_KEY3] - 1);
            Pad3 = val < 31 ? val : 6; // Default to Squid Knife
            val = (byte)(module.Eeprom.bytes[EEPROM_KEY4] - 1);
            Pad4 = val < 31 ? val : 29; // Default to Upsidedown Y

            InitializeComponent();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public List<byte[]> BuildSetEEPROMPackets()
        {
            if (module is null) return null;

            var list = new List<byte[]>();

            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM_KEY1, (byte)(Pad1+1)));
            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM_KEY2, (byte)(Pad2+1)));
            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM_KEY3, (byte)(Pad3+1)));
            list.Add(TheTimer.buildSetEEPROMPacket(module.Address, EEPROM_KEY4, (byte)(Pad4+1)));

            return list;
        }
    }
}
