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
        private SerialPort port { get; set; }

        public List<string> Ports => SerialPort.GetPortNames().ToList();
        public string PortName { get; set; } = "COM3";
        public int Baud { get; set; } = 9600;
        public string ConsoleText { get; set; }

        public string PortString => $"{port?.PortName ?? "NULL"}@{port?.BaudRate ?? 0}";

        public void ConsoleWrite(string s)
        {
            ConsoleText += s;
            OnPropertyChanged("ConsoleText");
        }

        public void Connect()
        {
            port?.Close();
            port = new SerialPort(PortName, Baud);

            ConsoleWrite($"Connecting to {PortString}... ");

            try
            {
                port.Open();
            }
            catch(Exception ex)
            {
                ConsoleWrite($"Failed to open {PortString}:\n");
                ConsoleWrite(ex.ToString() + "\n");
                return;
            }
            
            ConsoleWrite($"Connected!\n");
        }

        internal void PollSerial()
        {
            if (port.BytesToRead == 0)
                return;

            StringBuilder sb = new StringBuilder(256);
            while(port.BytesToRead > 0)
            {
                sb.Append((char)port.ReadChar());
            }
            ConsoleWrite(sb.ToString());
        }
    }
}
