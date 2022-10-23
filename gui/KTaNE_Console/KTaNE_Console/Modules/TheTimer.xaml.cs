using KTaNE_Console.Core;
using KTaNE_Console.Model;
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
    /// Interaction logic for TheTimer.xaml
    /// </summary>
    public partial class TheTimer : UserControl, IModule, INotifyPropertyChanged
    {
        private Serial serial;

        public event PropertyChangedEventHandler PropertyChanged;

        public byte Address => UartPacket.TIMER_ADDRESS;
        public string ModelID => throw new NotImplementedException();
        public SolvedState State => throw new NotImplementedException();

        public string TimeLimitString { get; set; } = "0:0:0";
        public string TimeLimitInput { get; set; } = "0";
        public RelayCommand SetTimeLimitCmd { get; set; }

        public string NumBatteriesFeedback { get; set; } = "0";
        public string NumBatteriesInput { get; set; } = "0";
        public RelayCommand SetNumBatteriesCmd { get; set; }

        public string SerialNoFeedback { get; set; } = "0";
        public string SerialNoInput { get; set; } = "0";
        public RelayCommand SetSerialNoCmd { get; set; }

        public TheTimer(Serial serial)
        {
            InitializeComponent();
            DataContext = this;
            this.serial = serial;

            serial.PacketReceived += Serial_PacketReceived;

            SetTimeLimitCmd = new RelayCommand((o) =>
            {
                Int32 timeLimit_s = Int32.Parse(TimeLimitInput);
                serial.SendCommand(0, Address, (byte)CommandID.SET_TIME_LIMIT, (uint)timeLimit_s);
            });

            SetNumBatteriesCmd = new RelayCommand((o) =>
            {
                Int32 nBatteries = Int32.Parse(NumBatteriesInput);
                serial.SendCommand(0, Address, (byte)CommandID.SET_N_BATTERIES, (uint)nBatteries);
            });

            SetSerialNoCmd = new RelayCommand((o) =>
            {
                var stringBytes = Encoding.ASCII.GetBytes(SerialNoInput);
                var bytes = new byte[stringBytes.Length + 2];
                bytes[0] = (byte)CommandID.SET_SERIAL_NO;
                Array.Copy(stringBytes, 0, bytes, 1, stringBytes.Length);
                bytes[bytes.Length - 1] = 0; // Force null terminator
                serial.SendCommand(0, Address, bytes);
            });
        }

        private void Serial_PacketReceived(object sender, SerialPacketReceivedEventArgs e)
        {
            var pkt = UartPacket.FromFullPacket(e.packet);

            if(((byte)(pkt.address & 0b01111111)) != Address)
                return;

            switch (pkt.Command)
            {
                case CommandID.RESET:
                    SendConfigRequests();
                    // Clear modules list in main view model
                    break;
                case CommandID.SET_TIME_LIMIT:
                    UInt32 newTimeLimit = BitConverter.ToUInt32(pkt.i2c_bytes, 1);
                    TimeLimitString = String.Format("{0:00}:{1:00}", newTimeLimit / 60, newTimeLimit % 60);
                    break;
                case CommandID.SET_N_BATTERIES:
                    NumBatteriesFeedback = pkt.Argument.ToString();
                    break;
                case CommandID.SET_SERIAL_NO:
                    int n = 0;
                    for (; pkt.i2c_bytes[n + 1] != 0; n++)
                        ;
                    SerialNoFeedback = Encoding.ASCII.GetString(pkt.i2c_bytes, 1, n);
                    break;
                default:
                    break;
            }

            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(""));
        }

        public void SendConfigRequests()
        {
            serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_TIME_LIMIT });
            serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_N_BATTERIES });
            serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_SERIAL_NO });
        }
    }
}
