using KTaNE_Console.Core;
using KTaNE_Console.Model;
using KTaNE_Console.ViewModel;
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
    /// Interaction logic for PlaceholderModule.xaml
    /// </summary>
    public partial class PlaceholderModule : UserControl, IModule, INotifyPropertyChanged
    {

        public event PropertyChangedEventHandler PropertyChanged;
        public byte Address { get; set; }

        public string Name { get; set; }

        public string ModelID { get; set; }

        public SolvedState State { get; set; }

        public byte Status { get; set; }

        private string moduleName = "<NULL>";
        public string ModuleName
        {
            get
            {
                moduleName = System.Text.Encoding.ASCII.GetString(Eeprom.bytes, EEPROM.MODULE_NAME, 16).Trim();
                return moduleName;
            }
            set => moduleName = value;
        }
        private string moduleSerialNo = "<NULL>";
        public string ModuleSerialNo
        {
            get
            {
                moduleSerialNo = System.Text.Encoding.ASCII.GetString(Eeprom.bytes, EEPROM.MODULE_SERIAL, 16).Trim();
                return moduleSerialNo;
            }
            set => moduleSerialNo = value;
        }

        private string moduleVersion = "<NULL>";
        public string ModuleVersion
        {
            get
            {
                moduleVersion = System.Text.Encoding.ASCII.GetString(Eeprom.bytes, EEPROM.MODULE_DATE, 16).Trim();
                return moduleVersion;
            }
            set => moduleVersion = value;
        }

        public EEPROM Eeprom = new EEPROM();


        private Serial serial;
        private IConsoleWriter CW;
        public PlaceholderModule(byte addr, Serial serial, IConsoleWriter ConsoleWriter)
        {
            InitializeComponent();
            this.DataContext = this;
            this.serial = serial;
            this.CW = ConsoleWriter;
            this.Address = addr;
            serial.PacketReceived += Serial_PacketReceived;
        }
        private void Serial_PacketReceived(object sender, SerialPacketReceivedEventArgs e)
        {
            var pkt = UartPacket.FromFullPacket(e.packet);

            if (((byte)(pkt.address & 0b01111111)) != Address)
                return;

            switch (pkt.Command)
            {
                case CommandID.GET_EEPROM:
                    int eeprom_address = BitConverter.ToInt16(pkt.i2c_bytes, 1);
                    int length = pkt.i2c_bytes[3];
                    var bytes = new byte[length];
                    Array.Copy(pkt.i2c_bytes, 4, bytes, 0, length);
                    Eeprom.Update(eeprom_address, bytes);
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(""));
                    break;
                default:
                    break;
            }
        }
    }
}
