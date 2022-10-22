using KTaNE_Console.Core;
using KTaNE_Console.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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

        public List<string> Ports => SerialPort.GetPortNames().ToList();

        public string ConsoleText { get; set; }
        public string PortName { get; set; } = "COM3";
        public int Baud { get; set; } = 9600;

        public int nPacketsReceived { get; set; } = 0;
        public string tbText { get; set; } = "23";

        public string PortString => serial.PortString;

        public RelayCommand ClearConsoleCmd { get; set; }
        public RelayCommand TestCmd { get; set; }

        public string TestCommandAddressString { get; set; }
        public string TestCommandBytesString { get; set; }

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



        public MainViewModel()
        {
            serial = new Serial();
            serial.TextReceived += Serial_TextReceived;
            serial.PacketReceived += Serial_PacketReceived;

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

        private void Serial_PacketReceived(object sender, SerialPacketReceivedEventArgs e)
        {
            // Require length 3 for sync, sync, length
            if (e.packet.Length < 3)
                return; 
            nPacketsReceived++;
            OnPropertyChanged("nPacketsReceived");
            Console.WriteLine(e.packet[e.packet.Length-1].ToString());
            Application.Current.Dispatcher.Invoke(() =>
            {
                var pkt = UartPacket.FromFullPacket(e.packet);
                ReceivedPackets.Add(pkt);
                RxPacketsText += pkt.ToString() + Environment.NewLine;
            });
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
        }

        //internal void PollSerial()
        //{
        //    if (port.BytesToRead == 0)
        //        return;

        //    StringBuilder sb = new StringBuilder(256);
        //    while(port.BytesToRead > 0)
        //    {
        //        sb.Append((char)port.ReadChar());
        //    }
        //    ConsoleWrite(sb.ToString());
        //}
    }
}
