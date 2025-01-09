using KTaNE_Console.Core;
using KTaNE_Console.Model;
using KTaNE_Console.Modules;
using KTaNE_Console.Properties;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace KTaNE_Console.ViewModel
{
    public class MainViewModel : ObservableObject, IConsoleWriter
    {
        public Serial serial;

        #region Serial Bindings
        public List<string> ComPorts => SerialPort.GetPortNames().ToList();
        public string PortName { get; set; } = "COM3";
        public int Baud { get; set; } = 9600;
        public string PortString => serial.PortString;
        #endregion

        #region Consoles
        public string ConsoleText { get; set; }
        public int nPacketsReceived { get; set; } = 0;

        public RelayCommand ClearConsoleCmd { get; set; }

        public List<UartPacket> ReceivedPackets { get; set; } = new List<UartPacket>();
        public List<UartPacket> SentPackets { get; set; } = new List<UartPacket>();

        private string _rxPacketsText;
        public string RxPacketsText
        {
            get { return _rxPacketsText; }
            set { _rxPacketsText = value; OnPropertyChanged(); }
        }

        private string _txPacketsText;
        public string TxPacketsText
        {
            get { return _txPacketsText; }
            set { _txPacketsText = value; OnPropertyChanged(); }
        }
        #endregion

        #region Serial Commands
        public RelayCommand TestCmd { get; set; }
        public RelayCommand EEPROMDumpCmd { get; set; }
        public RelayCommand ConnectCmd { get; set; }
        public RelayCommand StartCmd { get; set; }
        public RelayCommand SetEEPROMCmd { get; set; }
        public string TestCommandAddressString { get; set; }
        public string TestCommandBytesString { get; set; }
        public string SetEEPROMAddressString { get; set; } = "0";
        public string SetEEPROMByteString { get; set; } = "0";
        public string SetEEPROMI2CAddressString { get; set; } = "1";


        public Queue<byte[]> TxQueue { get; set; } = new Queue<byte[]>();
        #endregion

        public ObservableCollection<IModule> ModuleList { get; set; } = new ObservableCollection<IModule>();

        public MainViewModel()
        {
            serial = new Serial();
            serial.TextReceived += Serial_TextReceived;
            serial.PacketReceived += Serial_PacketReceived;
            serial.PacketSent += Serial_PacketSent;

            ModuleList.Add(new TheTimer(serial,this));

            ClearConsoleCmd = new RelayCommand((o) =>
            {
                ConsoleText = "";
                TxPacketsText = "";
                RxPacketsText = "";
                OnPropertyChanged("ConsoleText");
                OnPropertyChanged("TxPacketsText");
                OnPropertyChanged("RxPacketsText");
            });

            TestCmd = new RelayCommand((o) =>
            {
                try
                {
                    int address = Convert.ToByte(TestCommandAddressString, 16);
                    var splitBytes = TestCommandBytesString.Split();

                    byte[] bytes = new byte[splitBytes.Length];
                    for (int i = 0; i < splitBytes.Length; i++)
                    {
                        bytes[i] = Convert.ToByte(splitBytes[i], 16);
                    }

                    serial.SendCommand(30, (byte)address, bytes);
                    TxPacketsText += UartPacket.FromFullPacket(bytes).ToString() + Environment.NewLine;
                }
                catch (Exception ex)
                {
                    ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });

            ConnectCmd = new RelayCommand((o) =>
            {
                Connect();
            });

            SetEEPROMCmd = new RelayCommand((o) =>
            {
                try
                {
                    var now = DateTime.Now;
                    var timeString = now.ToString("MM/dd/yyyy HH:mm");
                    Console.WriteLine(timeString);

                    byte i2c_address = Convert.ToByte(SetEEPROMI2CAddressString);
                    short eeprom_addr = Convert.ToInt16(SetEEPROMAddressString, 16);
                    var eeprom_addr_bytes = BitConverter.GetBytes(eeprom_addr);
                    byte value = Convert.ToByte(SetEEPROMByteString, 16);

                    byte[] bytes = new byte[5];
                    
                    bytes[0] = (byte)CommandID.SET_EEPROM; // GET_EEPROM command ID
                    bytes[1] = eeprom_addr_bytes[0]; // EEPROM Addresss lower byte
                    bytes[2] = eeprom_addr_bytes[1]; // EEPROM Addresss upper byte
                    bytes[3] = 1; // Number of bytes
                    bytes[4] = value;

                    serial.SendCommand(0, i2c_address, bytes);
                    TxPacketsText += UartPacket.FromFullPacket(bytes).ToString() + Environment.NewLine;
                }
                catch (Exception ex)
                {
                    ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });

            EEPROMDumpCmd = new RelayCommand((o) =>
            {
                try
                {
                    byte chunk_size = 16; // do not change carelessly
                    short eeprom_size = 512; // I don't think this is accurate
                    for (int i = 0; i < eeprom_size / chunk_size; i++)
                    {
                        byte i2c_address = Convert.ToByte(SetEEPROMI2CAddressString);

                        var eeprom_addr_bytes = BitConverter.GetBytes((short)(chunk_size * i));

                        byte[] bytes = new byte[4];

                        bytes[0] = (byte)CommandID.GET_EEPROM;
                        bytes[1] = eeprom_addr_bytes[0]; // EEPROM Addresss lower byte
                        bytes[2] = eeprom_addr_bytes[1]; // EEPROM Addresss upper byte
                        bytes[3] = chunk_size; // number of bytes

                        serial.SendCommand(20, i2c_address, bytes);

                        //TxPacketsText += UartPacket.FromFullPacket(bytes).ToString() + Environment.NewLine;
                    }
                }
                catch (Exception ex)
                {
                    ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });

            StartCmd = new RelayCommand((o) =>
            {
                try
                {
                    byte i2c_address = 0x00; // Timer is always 0x00
                    serial.SendCommand(0, i2c_address, 0x11, 1);// CommandID.REG_CTRL: CTRL_START
                    //TxPacketsText += UartPacket.FromFullPacket(bytes).ToString() + Environment.NewLine;
                }
                catch (Exception ex)
                {
                    ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });

            LoadSettings();

        }

        private void LoadSettings()
        {
            //if (Settings.Default.ConnectAutomatically)
            //{
            //    PortName = Settings.Default.ComPort;
            //    Connect();
            //}
        }

        internal void RefreshComList()
        {
            OnPropertyChanged("ComPorts");
        }

        private void Serial_PacketSent(object sender, SerialPacketSentEventArgs e)
        {
            TxPacketsText += UartPacket.FromFullPacket(e.packet).ToString() + Environment.NewLine;
        }

        private void Serial_PacketReceived(object sender, SerialPacketReceivedEventArgs e)
        {
            // Require length 3 for sync, sync, length
            if (e.packet.Length < 3)
                return;
            nPacketsReceived++;
            OnPropertyChanged("nPacketsReceived");
            //Console.WriteLine(e.packet[e.packet.Length - 1].ToString());

            var pkt = UartPacket.FromFullPacket(e.packet);
            ReceivedPackets.Add(pkt);
            RxPacketsText += pkt.ToString() + Environment.NewLine;
            //OnPropertyChanged("PacketsList");

            if (pkt.i2c_bytes[0] == (byte)CommandID.GET_EEPROM)
            {
                int i2c_address = pkt.address & 0x7F; // Mask off top bit that indicates this is a response
                int eeprom_address = BitConverter.ToInt16(pkt.i2c_bytes, 1);
                int length = pkt.i2c_bytes[3];
                string s = $"{i2c_address.ToString("D3")} {eeprom_address.ToString("X3")}: ";
                for (int i = 0; i < length; i++)
                {
                    byte b = pkt.i2c_bytes[4 + i];
                    s += b.ToString("X2") + " ";
                }
                for (int i = 0; i < length; i++)
                {
                    byte b = pkt.i2c_bytes[4 + i];
                    if (b >= 32 && b <= 126)
                    {
                        s += (char)b;
                    }
                    else
                    {
                        s += '.';
                    }
                }
                ConsoleWrite(s + Environment.NewLine);
            }
            else if (pkt.i2c_bytes[0] == (byte)CommandID.LIST_MODULES)
            {
                int nModules = (pkt.i2c_bytes.Length - 1) / 2;
                Application.Current?.Dispatcher.BeginInvoke((Action)(() =>
                {
                    if (ModuleList.Count != (nModules+1))
                    {
                        var timer = ModuleList.First();
                        ModuleList.Clear();
                        ModuleList.Add(timer);
                        ConsoleWrite("New modules." + Environment.NewLine);
                    }
                    for (int i = 0; i < nModules; i++)
                    {
                        var addr = pkt.i2c_bytes[1 + 2 * i];
                        var status = pkt.i2c_bytes[1 + 2 * i + 1];

                        var mod = ModuleList.SingleOrDefault(x => x.BusAddress == addr);
                        if (mod != null)
                        {
                            mod.Status = status;
                        }
                        else
                        {
                            ModuleList.Add(new PlaceholderModule(addr, serial, this));
                        }
                    }
                }), DispatcherPriority.DataBind);
            }
        }

        private void Serial_TextReceived(object sender, SerialTextReceivedEventArgs e)
        {
            ConsoleWrite(e.text);
        }

        public void ConsoleWrite(string s)
        {
            ConsoleText += s;
            OnPropertyChanged("ConsoleText");
        }

        public void Connect()
        {
            if(String.IsNullOrEmpty(PortName))
            {
                ConsoleWrite("Cannot connect: Port NULL");
                return; 
            }

            // User settings.
            // Temporarily removed because file corrupted in power outage
            //Settings.Default.ComPort = PortName;
            //Settings.Default.Save();

            ConsoleWrite("Connecting..." + Environment.NewLine);
            serial.Connect(PortName, Baud);

            Task.Run(() =>
            {
                Thread.Sleep(500);
                ConsoleWrite("Getting Timer config..." + Environment.NewLine);
                Thread.Sleep(500);
                ((TheTimer)(ModuleList[0])).ReadTimerEEPROMCmd.Execute(null);
            });
        }
    }
}
