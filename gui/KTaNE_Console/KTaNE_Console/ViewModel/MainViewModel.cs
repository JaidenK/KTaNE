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
    public class MainViewModel : ObservableObject
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
        public string TestCommandAddressString { get; set; }
        public string TestCommandBytesString { get; set; }
        #endregion

        public List<IModule> ModuleList { get; set; } = new List<IModule>();

        public MainViewModel()
        {
            serial = new Serial();
            serial.TextReceived += Serial_TextReceived;
            serial.PacketReceived += Serial_PacketReceived;
            serial.PacketSent += Serial_PacketSent;

            ModuleList.Add(new TheTimer(serial));

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
            Console.WriteLine(e.packet[e.packet.Length-1].ToString());

            var pkt = UartPacket.FromFullPacket(e.packet);
            ReceivedPackets.Add(pkt);
            RxPacketsText += pkt.ToString() + Environment.NewLine;
            //OnPropertyChanged("PacketsList");
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
            foreach(var Module in ModuleList)
            {
                Module.SendConfigRequests();
            }
        }
    }
}
