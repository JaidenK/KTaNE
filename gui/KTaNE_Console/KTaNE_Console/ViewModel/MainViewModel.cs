using KTaNE_Console.Core;
using KTaNE_Console.Model;
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

        public string TimeLimitString { get; set; } = "0:0:0";
        public string TimeLimitInput { get; set; } = "0";
        public RelayCommand SetTimeLimitCmd { get; set; }



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

            SetTimeLimitCmd = new RelayCommand((o) =>
            {
                byte nResponseBytes = 0;
                byte address = UartPacket.TIMER_ADDRESS;
                Int32 timeLimit_s = Int32.Parse(TimeLimitInput);
                byte[] command = new byte[5];
                command[0] = UartPacket.SET_TIME_LIMIT;
                Array.Copy(BitConverter.GetBytes(timeLimit_s), 0, command, 1, 4);

                serial.SendCommand(nResponseBytes, address, command);
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

            try
            {
                Application.Current.Dispatcher.Invoke(() =>
                {
                    var pkt = UartPacket.FromFullPacket(e.packet);
                    ReceivedPackets.Add(pkt);
                    RxPacketsText += pkt.ToString() + Environment.NewLine;

                    switch(pkt.address & 0b01111111)
                    {
                        case UartPacket.TIMER_ADDRESS:
                            switch(pkt.i2c_bytes[0])
                            {
                                case UartPacket.RESET:
                                    var bytes = serial.SendCommand(0, UartPacket.TIMER_ADDRESS,new byte[] { UartPacket.REQUEST_CONFIG, UartPacket.SET_TIME_LIMIT });
                                    TxPacketsText += UartPacket.FromFullPacket(bytes).ToString() + Environment.NewLine;
                                    break;
                                case UartPacket.SET_TIME_LIMIT:
                                    UInt32 newTimeLimit = BitConverter.ToUInt32(pkt.i2c_bytes, 1);
                                    TimeLimitString = String.Format("{0:00}:{1:00}", newTimeLimit / 60, newTimeLimit % 60);
                                    OnPropertyChanged("TimeLimitString");
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }

                });
            }
            catch(Exception ex)
            {
                ConsoleWrite(ex.Message);
            }
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
