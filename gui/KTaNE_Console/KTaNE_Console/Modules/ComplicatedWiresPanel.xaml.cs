using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
    public class ComplicatedWire
    {
        private bool isRed;

        public bool IsRed
        {
            get { return isRed; }
            set { isRed = value; }
        }
        private bool hasStar;

        public bool HasStar
        {
            get { return hasStar; }
            set { hasStar = value; }
        }
        private bool isBlue;

        public bool IsBlue
        {
            get { return isBlue; }
            set { isBlue = value; }
        }
        private bool led;

        public bool LED
        {
            get { return led; }
            set { led = value; }
        }


        public ComplicatedWire() { }
    }
    /// <summary>
    /// Interaction logic for ComplicatedWiresPanel.xaml
    /// </summary>
    public partial class ComplicatedWiresPanel : UserControl, INotifyPropertyChanged, IModulePanel
    {
        readonly ushort EEPROM_REDWIRES =  0x1E0;
        readonly ushort EEPROM_BLUEWIRES = 0x1E1;
        readonly ushort EEPROM_STARS =     0x1E2;
        readonly ushort EEPROM_LEDS =      0x1E3;
        public ObservableCollection<ComplicatedWire> Wires { get; set; } = new ObservableCollection<ComplicatedWire>();

        IModule module;
        public ComplicatedWiresPanel(IModule module)
        {
            this.module = module;
            Wires.Add(new ComplicatedWire());
            Wires.Add(new ComplicatedWire());
            Wires.Add(new ComplicatedWire());
            Wires.Add(new ComplicatedWire());
            Wires.Add(new ComplicatedWire());
            Wires.Add(new ComplicatedWire());
            this.DataContext = this;           
            InitializeComponent();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public List<byte[]> BuildSetEEPROMPackets()
        {
            if (module is null) return null;

            var list = new List<byte[]>();

            byte reds = 0;
            byte blues = 0;
            byte stars = 0;
            byte leds = 0;

            for(int i = 0; i < Wires.Count; i++)
            {
                var w = Wires[i];
                reds  |= (byte)(w.IsRed ? (1 << i) : 0);
                blues |= (byte)(w.IsBlue ? (1 << i) : 0);
                stars |= (byte)(w.HasStar ? (1 << i) : 0);
                leds  |= (byte)(w.LED ? (1 << i) : 0);
            }

            list.Add(TheTimer.buildSetEEPROMPacket(EEPROM_REDWIRES, reds));
            list.Add(TheTimer.buildSetEEPROMPacket(EEPROM_BLUEWIRES, blues));
            list.Add(TheTimer.buildSetEEPROMPacket(EEPROM_STARS, stars));
            list.Add(TheTimer.buildSetEEPROMPacket(EEPROM_LEDS, leds));

            return list;
        }
    }
}
