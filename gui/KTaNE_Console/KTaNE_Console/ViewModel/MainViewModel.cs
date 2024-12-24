using KTaNE_Console.Core;
using KTaNE_Console.Model;
using KTaNE_Console.Modules;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO.Ports;
using System.Linq;
using System.Text;
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
                OnPropertyChanged("ConsoleText");
            });

            TestCmd = new RelayCommand((o) =>
            {
                try
                {
                    int address = Convert.ToByte(TestCommandAddressString, 16);
                    var splitBytes = TestCommandBytesString.Split();



                    byte[] bytes = new byte[13 + splitBytes.Length];
                    bytes[0] = Serial.SYNC_BYTE;
                    bytes[1] = Serial.SYNC_BYTE;
                    bytes[2] = (byte)bytes.Length;
                    bytes[3] = 1;
                    bytes[4] = 2;
                    bytes[9] = 30; // N_MAX_MODULE_NAME_CHARS
                    bytes[10] = (byte)address;
                    for (int i = 0; i < splitBytes.Length; i++)
                    {
                        bytes[UartPacket.DATA_START + i] = Convert.ToByte(splitBytes[i], 16);
                    }

                    // Calculate CRC
                    // ... 

                    serial.Write(bytes);
                    TxPacketsText += UartPacket.FromFullPacket(bytes).ToString() + Environment.NewLine;
                }
                catch (Exception ex)
                {
                    ConsoleWrite(ex.Message + Environment.NewLine);
                }
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

                    //byte nEEPROMBytes = (byte)timeString.Length;
                    byte nEEPROMBytes = 1;

                    byte[] bytes = new byte[13 + 4 + nEEPROMBytes];
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
                    bytes[14] = nEEPROMBytes; // Number of bytes
                    //for(int i = 0; i < nEEPROMBytes; i++)
                    //{
                    //    bytes[15 + i] = (byte)timeString[i];
                    //}
                    bytes[15] = value;

                    // Calculate CRC
                    // ... 

                    serial.Write(bytes);
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
                    for (int i = 0; i < 512 / 16; i++)
                    {
                        //int i2c_address = 0x01; // Timer is always 0x01
                        byte i2c_address = Convert.ToByte(SetEEPROMI2CAddressString);

                        var eeprom_addr_bytes = BitConverter.GetBytes((short)(0x10 * i));

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
                        bytes[14] = 16; // number of bytes

                        // Calculate CRC
                        // ... 

                        //serial.Write(bytes);
                        TxQueue.Enqueue(bytes);
                    }
                    var bytes2 = TxQueue.Dequeue();
                    serial.Write(bytes2);
                    TxPacketsText += UartPacket.FromFullPacket(bytes2).ToString() + Environment.NewLine;
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
                    int i2c_address = 0x00; // Timer is always 0x00


                    byte[] bytes = new byte[13 + 2];
                    bytes[0] = Serial.SYNC_BYTE;
                    bytes[1] = Serial.SYNC_BYTE;
                    bytes[2] = (byte)bytes.Length;
                    bytes[3] = 1; // Seq Count
                    bytes[4] = 2; // Seq Count Echo
                    bytes[9] = 0; // Response length: 0
                    bytes[10] = (byte)i2c_address;
                    bytes[11] = (byte)0x11;// CommandID.REG_CTRL;
                    bytes[12] = 1; // CTRL_START

                    serial.Write(bytes);
                    TxPacketsText += UartPacket.FromFullPacket(bytes).ToString() + Environment.NewLine;
                }
                catch (Exception ex)
                {
                    ConsoleWrite(ex.Message + Environment.NewLine);
                }
            });

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

                        var mod = ModuleList.SingleOrDefault(x => x.Address == addr);
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

                if (TxQueue.Count > 0)
            {
                var bytes2 = TxQueue.Dequeue();
                serial.Write(bytes2);
                TxPacketsText += UartPacket.FromFullPacket(bytes2).ToString() + Environment.NewLine;
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
            serial.Connect(PortName, Baud);
            //foreach (var Module in ModuleList)
            //{
            //    Module.SendConfigRequests();
            //}
            ((TheTimer)(ModuleList[0])).ReadTimerEEPROMCmd.Execute(null);
        }
    }
}
