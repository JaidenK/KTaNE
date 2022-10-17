using KTaNE_Console.Core;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

        public string PortString => serial.PortString;

        public RelayCommand ClearConsoleCmd { get; set; }

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
        }

        private void Serial_PacketReceived(object sender, SerialPacketReceivedEventArgs e)
        {
            // Require length 3 for sync, sync, length
            if (e.packet.Length < 3)
                return; 
            nPacketsReceived++;
            OnPropertyChanged("nPacketsReceived");
            Console.WriteLine(e.packet[e.packet.Length-1].ToString());
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
