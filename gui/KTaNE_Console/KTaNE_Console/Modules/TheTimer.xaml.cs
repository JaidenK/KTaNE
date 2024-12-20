using KTaNE_Console.Core;
using KTaNE_Console.Model;
using KTaNE_Console.ViewModel;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
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

    public class EEPROM_Timer
    {
        public byte[] bytes = new byte[512];

        internal void Update(int eeprom_address, byte[] bytes)
        {
            Array.Copy(bytes, 0, this.bytes, eeprom_address, bytes.Length);
        }
    }

    /// <summary>
    /// Interaction logic for TheTimer.xaml
    /// </summary>
    public partial class TheTimer : UserControl, IModule, INotifyPropertyChanged
    {
        private Serial serial;
        private IConsoleWriter CW;

        public event PropertyChangedEventHandler PropertyChanged;

        public byte Address => UartPacket.TIMER_ADDRESS;
        public string ModelID => throw new NotImplementedException();
        public SolvedState State => throw new NotImplementedException();

        public string TimeLimitString { get; set; } = "0:0:0";
        public string TimeLimitInput { get; set; } = "0";
        //public RelayCommand SetTimeLimitCmd { get; set; }

        public string NumAABatteriesFeedback => EEPROM.bytes[0x62].ToString();
        public string NumAABatteriesInput { get; set; } = "0";
        //public RelayCommand SetNumBatteriesCmd { get; set; }
        public string NumDBatteriesFeedback => EEPROM.bytes[0x63].ToString();

        public bool Port_DVID_Feedback => (EEPROM.bytes[0x64] & (1 << 0)) != 0;
        public bool Port_Parallel_Feedback => (EEPROM.bytes[0x64] & (1 << 1)) != 0;
        public bool Port_PS2_Feedback => (EEPROM.bytes[0x64] & (1 << 2)) != 0;
        public bool Port_RJ45_Feedback => (EEPROM.bytes[0x64] & (1 << 3)) != 0;
        public bool Port_Serial_Feedback => (EEPROM.bytes[0x64] & (1 << 4)) != 0;
        public bool Port_StereoRCA_Feedback => (EEPROM.bytes[0x64] & (1 << 5)) != 0;
        public bool Port_Spare1_Feedback => (EEPROM.bytes[0x64] & (1 << 6)) != 0;
        public bool Port_Spare2_Feedback => (EEPROM.bytes[0x64] & (1 << 7)) != 0;

        public string SerialNoFeedback => System.Text.Encoding.ASCII.GetString(EEPROM.bytes, 0x50, 16).Trim();
        public string SerialNoInput { get; set; } = "0";
        //public RelayCommand SetSerialNoCmd { get; set; }

        private string moduleName = "<NULL>";
        public string ModuleName
        {
            get
            {
                moduleName = System.Text.Encoding.ASCII.GetString(EEPROM.bytes, 0, 16).Trim();
                return moduleName;
            }
            set => moduleName = value;
        }
        private string moduleSerialNo = "<NULL>";
        public string ModuleSerialNo
        {
            get
            {
                moduleSerialNo = System.Text.Encoding.ASCII.GetString(EEPROM.bytes, 0x10, 16).Trim();
                return moduleSerialNo;
            }
            set => moduleSerialNo = value;
        }

        private string moduleVersion = "<NULL>";
        public string ModuleVersion
        {
            get
            {
                moduleVersion = System.Text.Encoding.ASCII.GetString(EEPROM.bytes, 0x20, 16).Trim();
                return moduleVersion;
            }
            set => moduleVersion = value;
        }

        public RelayCommand ReadTimerEEPROMCmd { get; set; }
        public RelayCommand ApplyTimerEEPROMCmd { get; set; }
        private Queue<byte[]> TxQueue { get; set; } = new Queue<byte[]>();

        public EEPROM_Timer EEPROM = new EEPROM_Timer();

        public List<byte[]> BuildGetEEPROMPackets(byte i2c_address, ushort eeprom_address, UInt16 length)
        {
            int remainingLength = length;
            int i = 0;
            List<byte[]> retval = new List<byte[]>();
            while (remainingLength > 0)
            {
                var current_address = (ushort)(eeprom_address + (0x10 * i));
                i++;
                var eeprom_addr_bytes = BitConverter.GetBytes(current_address);
                byte currentLength = (byte)(remainingLength > 16 ? 16 : remainingLength);
                remainingLength -= 16;

                byte[] bytes = new byte[13 + 4];
                bytes[0] = Serial.SYNC_BYTE;
                bytes[1] = Serial.SYNC_BYTE;
                bytes[2] = (byte)bytes.Length;
                bytes[3] = 1;
                bytes[4] = 2;
                bytes[9] = 20; // 16 bytes + header
                bytes[10] = (byte)i2c_address;
                bytes[11] = (byte)CommandID.GET_EEPROM;
                bytes[12] = eeprom_addr_bytes[0]; // EEPROM Addresss lower byte
                bytes[13] = eeprom_addr_bytes[1]; // EEPROM Addresss upper byte
                bytes[14] = currentLength; // number of bytes

                retval.Add(bytes);
            }

            return retval;
        }

        public TheTimer(Serial serial, IConsoleWriter ConsoleWriter)
        {
            InitializeComponent();
            DataContext = this;
            this.serial = serial;
            this.CW = ConsoleWriter;

            serial.PacketReceived += Serial_PacketReceived;

            //SetTimeLimitCmd = new RelayCommand((o) =>
            //{
            //    Int32 timeLimit_s = Int32.Parse(TimeLimitInput);
            //    serial.SendCommand(0, Address, (byte)CommandID.SET_TIME_LIMIT, (uint)timeLimit_s);
            //});

            //SetNumBatteriesCmd = new RelayCommand((o) =>
            //{
            //    Int32 nBatteries = Int32.Parse(NumBatteriesInput);
            //    serial.SendCommand(0, Address, (byte)CommandID.SET_N_BATTERIES, (uint)nBatteries);
            //});

            //SetSerialNoCmd = new RelayCommand((o) =>
            //{
            //    var stringBytes = Encoding.ASCII.GetBytes(SerialNoInput);
            //    var bytes = new byte[stringBytes.Length + 2];
            //    bytes[0] = (byte)CommandID.SET_SERIAL_NO;
            //    Array.Copy(stringBytes, 0, bytes, 1, stringBytes.Length);
            //    bytes[bytes.Length - 1] = 0; // Force null terminator
            //    serial.SendCommand(0, Address, bytes);
            //});

            ReadTimerEEPROMCmd = new RelayCommand((o) =>
            {
                try
                {
                    BuildGetEEPROMPackets(Address, 0, 512).ForEach(x =>
                    {
                        TxQueue.Enqueue(x);
                    });
                    var bytes = TxQueue.Dequeue();
                    serial.Write(bytes);
                }
                catch (Exception ex)
                {
                    CW.ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });
        }

        private void Serial_PacketReceived(object sender, SerialPacketReceivedEventArgs e)
        {
            if (TxQueue.Count > 0)
                serial.Write(TxQueue.Dequeue());

            var pkt = UartPacket.FromFullPacket(e.packet);

            if (((byte)(pkt.address & 0b01111111)) != Address)
                return;

            switch (pkt.Command)
            {
                case CommandID.RESET:
                    //SendConfigRequests();
                    // Clear modules list in main view model
                    break;
                //case CommandID.SET_TIME_LIMIT:
                //    UInt32 newTimeLimit = BitConverter.ToUInt32(pkt.i2c_bytes, 1);
                //    TimeLimitString = String.Format("{0:00}:{1:00}", newTimeLimit / 60, newTimeLimit % 60);
                //    break;
                //case CommandID.SET_N_BATTERIES:
                //    NumBatteriesFeedback = pkt.Argument.ToString();
                //    break;
                //case CommandID.SET_SERIAL_NO:
                //    int n = 0;
                //    for (; pkt.i2c_bytes[n + 1] != 0; n++)
                //        ;
                //    SerialNoFeedback = Encoding.ASCII.GetString(pkt.i2c_bytes, 1, n);
                //    break;
                case CommandID.GET_EEPROM:
                    int eeprom_address = BitConverter.ToInt16(pkt.i2c_bytes, 1);
                    int length = pkt.i2c_bytes[3];
                    var bytes = new byte[length];
                    Array.Copy(pkt.i2c_bytes, 4, bytes, 0, length);
                    EEPROM.Update(eeprom_address, bytes);
                    break;
                default:
                    break;
            }

            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(""));
        }

        public void SendConfigRequests()
        {
            //serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_TIME_LIMIT });
            //serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_N_BATTERIES });
            //serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_SERIAL_NO });
        }
    }
}
