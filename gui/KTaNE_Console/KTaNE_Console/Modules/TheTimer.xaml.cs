using KTaNE_Console.Core;
using KTaNE_Console.Model;
using KTaNE_Console.ViewModel;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
using System.Windows.Threading;

namespace KTaNE_Console.Modules
{
    public class EEPROM
    {
        // This must match KTaNE_Constants.h
        public static readonly ushort MODULE_NAME = 0x00;
        public static readonly ushort MODULE_SERIAL = 0x10;
        public static readonly ushort MODULE_DATE = 0x20;

        public static readonly ushort SERIAL_NO = 0x50;
        public static readonly ushort TIME_LIMIT = 0x60;
        public static readonly ushort AA_BATTERIES = 0x62;
        public static readonly ushort D_BATTERIES = 0x63;
        public static readonly ushort PORTS = 0x64;
        public static readonly ushort INDICATORS = 0x70;

        public byte[] bytes = new byte[512];
        public void Update(int eeprom_address, byte[] bytes)
        {
            Array.Copy(bytes, 0, this.bytes, eeprom_address, bytes.Length);
        }
    }
    public class EEPROM_Timer : EEPROM
    {

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

        public string TimeLimitString => BitConverter.ToUInt16(Eeprom.bytes, EEPROM.TIME_LIMIT).ToString();
        public string TimeLimitInput { get; set; } = "0";
        //public RelayCommand SetTimeLimitCmd { get; set; }

        public string NumAABatteriesFeedback => Eeprom.bytes[EEPROM.AA_BATTERIES].ToString();
        public string NumAABatteriesInput { get; set; } = "0";
        public string NumDBatteriesFeedback => Eeprom.bytes[EEPROM.D_BATTERIES].ToString();
        public string NumDBatteriesInput { get; set; } = "0";

        public bool Port_DVID_Feedback => (Eeprom.bytes[EEPROM.PORTS] & (1 << 0)) != 0;
        public bool Port_Parallel_Feedback => (Eeprom.bytes[EEPROM.PORTS] & (1 << 1)) != 0;
        public bool Port_PS2_Feedback => (Eeprom.bytes[EEPROM.PORTS] & (1 << 2)) != 0;
        public bool Port_RJ45_Feedback => (Eeprom.bytes[EEPROM.PORTS] & (1 << 3)) != 0;
        public bool Port_Serial_Feedback => (Eeprom.bytes[EEPROM.PORTS] & (1 << 4)) != 0;
        public bool Port_StereoRCA_Feedback => (Eeprom.bytes[EEPROM.PORTS] & (1 << 5)) != 0;
        public bool Port_Spare1_Feedback => (Eeprom.bytes[EEPROM.PORTS] & (1 << 6)) != 0;
        public bool Port_Spare2_Feedback => (Eeprom.bytes[EEPROM.PORTS] & (1 << 7)) != 0;
        public bool Port_DVID_Input { get; set; } = false;
        public bool Port_Parallel_Input { get; set; } = false;
        public bool Port_PS2_Input { get; set; } = false;
        public bool Port_RJ45_Input { get; set; } = false;
        public bool Port_Serial_Input { get; set; } = false;
        public bool Port_StereoRCA_Input { get; set; } = false;
        public bool Port_Spare1_Input { get; set; } = false;
        public bool Port_Spare2_Input { get; set; } = false;
        public string SerialNoFeedback => System.Text.Encoding.ASCII.GetString(Eeprom.bytes, EEPROM.SERIAL_NO, 16).Trim();

        public ObservableCollection<string> StatusList { get; set; } = new ObservableCollection<string>();
        public string SerialNoInput { get; set; } = "0";
        //public RelayCommand SetSerialNoCmd { get; set; }
        public string IndicatorsFeedback
        {
            get
            {
                string s = "";
                for (int i = 0; ; i++)
                {
                    int addr = EEPROM.INDICATORS + 4 * i;
                    var str = System.Text.Encoding.ASCII.GetString(Eeprom.bytes, addr, 4);
                    if (str.Length == 4)
                    {
                        if (str[0] == '+' || str[0] == '-')
                        {
                            if (i > 0)
                                s += ", ";
                            s += str;
                            continue;
                        }
                    }
                    break;
                }
                return s;
            }
        }

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
        public string IndicatorsInput { get; set; }

        public RelayCommand ReadTimerEEPROMCmd { get; set; }
        public RelayCommand ApplyTimerEEPROMCmd { get; set; }
        private Queue<byte[]> TxQueue { get; set; } = new Queue<byte[]>();
        public byte Status { get; set; } = 0;

        public EEPROM_Timer Eeprom = new EEPROM_Timer();

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

            ApplyTimerEEPROMCmd = new RelayCommand((o) =>
            {
                try
                {
                    Task.Run(() =>
                    {
                        CW.ConsoleWrite("Configuring..." + Environment.NewLine);
                        var pkts = BuildSetEEPROMPackets();
                        pkts.ForEach(x =>
                        {
                            serial.Write(x);
                            Task.Delay(200);
                        });
                        CW.ConsoleWrite("Done." + Environment.NewLine);
                    });
                }
                catch (Exception ex)
                {
                    CW.ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });
        }

        private byte[] buildSetEEPROMPacket(ushort eeprom_addr, byte b)
        {
            return buildSetEEPROMPacket(eeprom_addr, new byte[] { b });
        }
        private byte[] buildSetEEPROMPacket(ushort eeprom_addr, byte[] bytes_to_write)
        {

            byte i2c_address = Address;
            var eeprom_addr_bytes = BitConverter.GetBytes(eeprom_addr);

            byte[] bytes = new byte[13 + 4 + bytes_to_write.Length];
            bytes[0] = Serial.SYNC_BYTE;
            bytes[1] = Serial.SYNC_BYTE;
            bytes[2] = (byte)bytes.Length;
            bytes[3] = 1;
            bytes[4] = 2;
            bytes[9] = 0; // No response
            bytes[10] = i2c_address;
            bytes[11] = (byte)CommandID.SET_EEPROM; // GET_EEPROM command ID
            bytes[12] = eeprom_addr_bytes[0]; // EEPROM Addresss lower byte
            bytes[13] = eeprom_addr_bytes[1]; // EEPROM Addresss upper byte
            bytes[14] = (byte)bytes_to_write.Length; // Number of bytes
            for (int i = 0; i < bytes_to_write.Length; i++)
            {
                bytes[15 + i] = bytes_to_write[i];
            }

            return bytes;
        }

        private List<byte[]> BuildSetEEPROMPackets()
        {
            var list = new List<byte[]>();

            string serial = SerialNoInput;
            if (serial.Length > 16)
            {
                serial = serial.Substring(0, 16);
            }
            while (serial.Length < 16)
            {
                serial += ' ';
            }

            list.Add(buildSetEEPROMPacket(EEPROM.SERIAL_NO, Encoding.ASCII.GetBytes(serial)));
            list.Add(buildSetEEPROMPacket(EEPROM.TIME_LIMIT, BitConverter.GetBytes(ushort.Parse(TimeLimitInput))));
            list.Add(buildSetEEPROMPacket(EEPROM.AA_BATTERIES, byte.Parse(NumAABatteriesInput)));
            list.Add(buildSetEEPROMPacket(EEPROM.D_BATTERIES, byte.Parse(NumDBatteriesInput)));

            // Ports
            byte ports = 0;

            ports |= (byte)(Port_DVID_Input ? (1 << 0) : 0);
            ports |= (byte)(Port_Parallel_Input ? (1 << 1) : 0);
            ports |= (byte)(Port_PS2_Input ? (1 << 2) : 0);
            ports |= (byte)(Port_RJ45_Input ? (1 << 3) : 0);
            ports |= (byte)(Port_Serial_Input ? (1 << 4) : 0);
            ports |= (byte)(Port_StereoRCA_Input ? (1 << 5) : 0);
            ports |= (byte)(Port_Spare1_Input ? (1 << 6) : 0);
            ports |= (byte)(Port_Spare2_Input ? (1 << 7) : 0);

            list.Add(buildSetEEPROMPacket(EEPROM.PORTS, ports));

            // Indicators
            string indicators = IndicatorsInput ?? "";
            int i = 0;
            while(indicators.Length > 0)
            {
                string sub = indicators;
                if(indicators.Length > 16)
                {
                    sub = indicators.Substring(0, 16);
                    indicators = indicators.Substring(16, indicators.Length - 16);
                }
                else
                {
                    indicators = "";
                }
                list.Add(buildSetEEPROMPacket((ushort)(EEPROM.INDICATORS + (i * 0x10)), Encoding.ASCII.GetBytes(sub)));
                i++;
            }

            return list;
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
                case CommandID.GET_EEPROM:
                    int eeprom_address = BitConverter.ToInt16(pkt.i2c_bytes, 1);
                    int length = pkt.i2c_bytes[3];
                    var bytes = new byte[length];
                    Array.Copy(pkt.i2c_bytes, 4, bytes, 0, length);
                    Eeprom.Update(eeprom_address, bytes);
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(""));
                    break;
                case CommandID.LIST_MODULES:
                    int nModules = (pkt.i2c_bytes.Length - 1) / 2;                    
                    Dispatcher.BeginInvoke((Action)(() =>
                    {
                        if (StatusList.Count != nModules)
                        {
                            StatusList.Clear();
                        }
                        for (int i = 0; i < nModules; i++)
                        {
                            if (StatusList.Count <= i)
                            {
                                StatusList.Add("");
                            }
                            var addr = pkt.i2c_bytes[1 + 2 * i];
                            var status = pkt.i2c_bytes[1 + 2 * i + 1];
                            StatusList[i] = $"{addr.ToString("X2")}: {status.ToString("X2")}";
                        }
                    }), DispatcherPriority.DataBind);
                    break;
                default:
                    break;
            }

            //PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(""));
        }

        public void SendConfigRequests()
        {
            //serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_TIME_LIMIT });
            //serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_N_BATTERIES });
            //serial.SendCommand(0, Address, new byte[] { (byte)CommandID.REQUEST_CONFIG, (byte)CommandID.SET_SERIAL_NO });
        }
    }
}
